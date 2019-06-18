#include "viewitem.h"


#include <QtQuick/QQuickWindow>
#include <QOpenGLFramebufferObject>
#include <QSGSimpleTextureNode>
#include <QtMath>

class TextureNode : public QObject, public QSGSimpleTextureNode
{
    Q_OBJECT

public:
    TextureNode(QQuickWindow *window, ViewItem *renderer)
        : m_fbo(0)
        , m_texture(0)
        , m_window(window)
        , m_renderer(renderer)
        , m_picked(false)
    {
        setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);
        connect(m_window, SIGNAL(beforeRendering()), this, SLOT(renderFBO()), Qt::DirectConnection);
    }

    ~TextureNode()
    {
        delete m_texture;
        delete m_fbo;
    }

public slots:
    void renderFBO()
    {
        m_window->resetOpenGLState();

        if (!m_fbo) {
            QSize size = rect().size().toSize();
            QOpenGLFramebufferObjectFormat format;
            format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
            m_fbo = new QOpenGLFramebufferObject(size, format);
            m_texture = m_window->createTextureFromId(m_fbo->texture(), size);
            setTexture(m_texture);
        }
        m_renderer->renderTo(m_fbo);
        m_window->update();

        if (m_picked) {
            m_renderer->pickObject(m_pickedPos.x(), m_pickedPos.y());
            m_picked = false;
        }
    }

public:
    void schedulePick(const QPoint &pos) {m_pickedPos = pos; m_picked = true;}

private:
    QOpenGLFramebufferObject *m_fbo;
    QSGTexture *m_texture;
    QQuickWindow *m_window;
    ViewItem *m_renderer;
    bool m_picked;
    QPoint m_pickedPos;
};

ViewItem::ViewItem(QQuickItem * parent)
    : QQuickItem(parent)
    , m_multiSampleFbo(0)
    , m_superSampleFbo(0)
    , m_simpleFbo(0)
    , m_simpleFbo2(0)
    , m_postProcessFbo1(0)
    , m_postProcessFbo2(0)
    , m_nBlurPass(4)
    , m_antiAliasingType(NOAA)
    , m_node(0)
    , m_camera(0)
    , m_selectedBody(0)
    , m_galaxy(0)
    , m_sun(0)
{
    setFlag(ItemHasContents, true);
    setAcceptedMouseButtons(Qt::AllButtons);

    m_camera = new Camera(this);
}

ViewItem::~ViewItem()
{
    m_screenQuad->deleteLater();
    m_galaxy->deleteLater();
    m_sun->deleteLater();
    delete m_multiSampleFbo;
    delete m_superSampleFbo;
    delete m_simpleFbo;
    delete m_simpleFbo2;
    delete m_postProcessFbo1;
    delete m_postProcessFbo2;
}

QSGNode *ViewItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    // Don't bother with resize and such, just recreate the node from scratch
    // when geometry changes.
    if (oldNode) {
        delete oldNode;
    } else {
        init();
    }

    resizeGL(width(), height());
    TextureNode *node = new TextureNode(window(), this);
    node->setRect(boundingRect());
    m_node = node;

    return node;
}

QString ViewItem::selection() const
{
    if (!m_selectedBody) {
        return QString();
    }
    return m_selectedBody->name();
}

qreal ViewItem::distanceToGround() const
{
    if ((!m_selectedBody)||(!m_camera)) {
        return qreal();
    }
    return ((m_selectedBody->center()-m_camera->position()).norm()-m_selectedBody->radius())*1000.0;
}

void ViewItem::setNBlurPass(int nPass)
{
    m_mutex.lock();
    m_nBlurPass = nPass;
    m_mutex.unlock();
    emit nBlurChanged();
}
void ViewItem::setAntiAliasingType(int type)
{
    m_mutex.lock();
    m_antiAliasingType = type;
    float coeff = (m_antiAliasingType==SSAA) ? 2.0 : 1.0;
    Body::setPointSizeThreshold(coeff*10.0);
    if (m_galaxy) m_galaxy->setPointSizeCoeff(coeff);
    m_mutex.unlock();
    emit antialiasingTypeChanged();
}

void ViewItem::init()
{
    initializeOpenGLFunctions();

    m_aaTypes << NOAA;
    if (QOpenGLFramebufferObject::hasOpenGLFramebufferBlit())
        m_aaTypes << MSAA;
    m_aaTypes << SSAA;
    m_aaTypes << FXAA;
    emit aaTypesChanged();

    glEnable( GL_DEPTH_TEST );
#ifndef Q_OS_ANDROID
    glEnable( GL_PROGRAM_POINT_SIZE );
    glEnable( GL_POINT_SPRITE );
//    glEnable(GL_DEPTH_CLAMP);
//    glEnable(GL_LINE_SMOOTH);
//    glLineWidth(1.2);
    glLineWidth(2.0);
#else
    glLineWidth(2.0);
#endif

    m_screenQuad = new ScreenQuad();

    m_galaxy = new Galaxy();

    m_sun = new Body("sun");
    addBody(m_sun);

    // We need the earth frame at J2000 for the galaxy
    m_sun->setReferenceFrame(Eigen::Affine3d::Identity());
    m_sun->setTime(0.0);
    foreach (const Body* body, m_bodies) {
        if (body->name() == "earth") {
            EME2000 = body->referenceFrame();
            break;
        }
    }
    // From the HYG catalog description:
    // X,Y,Z: The Cartesian coordinates of the star, in a system based on the equatorial coordinates as seen from Earth.
    // +X is in the direction of the vernal equinox (at epoch 2000), +Z towards the north celestial pole,
    // and +Y in the direction of R.A. 6 hours, declination 0 degrees.
    // tl;dr: WGS84
    EME2000.rotate(Eigen::AngleAxisd(-M_PI/2.0, Eigen::Vector3d::UnitZ()));

    // Use current time
    double time = m_timeline.currentTime();
    m_sun->setTime(time);
    selectBody(m_sun);

    connect(m_camera, SIGNAL(positionChanged()), this, SIGNAL(distanceToGroundChanged()));
    connect(&m_timeline, SIGNAL(tick()), this, SIGNAL(dateUpdated()));
    connect(&m_timeline, SIGNAL(rateChanged()), this, SIGNAL(timeLineRateChanged()));
    connect(window(), SIGNAL(beforeRendering()), this, SLOT(animate()), Qt::DirectConnection);

    m_camera->setPosition(Eigen::Vector3d(0.0, 1.0, 1.0)*3000.0*900.0);
    m_camera->setUpVector(Eigen::Vector3d(0.0, -1.0, 1.0));
    m_camera->lookAt(m_sun->center());
}

void ViewItem::renderScene(int width, int height)
{
    Q_UNUSED(width)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const Eigen::Affine3d &mv = m_camera->modelView();
    const Eigen::Affine3d &p = m_camera->projection();

    foreach (Body* body, m_bodies) {
        double distance = ((body->center()-m_camera->position()).norm()-body->radius());
        float vfov = qDegreesToRadians(m_camera->fov());
        int onScreenRadius = (body->boundingRadius()/(tan(vfov/2.0)*distance))*height;
        body->setOnScreenRadius(onScreenRadius);

        Body* root = body->root();
        if (root) {
            double radius = (root->center()-body->center()).norm();
            distance = (root->center()-m_camera->position()).norm()-radius;
            int onScreenDistanceToParent = (radius/(tan(vfov/2.0)*distance))*height;
            body->setOnScreenDistanceToParent(onScreenDistanceToParent);
        }
    }

    // Depth test is disabled when rendering the stars so we have to draw them first
    glDisable( GL_DEPTH_TEST );
    m_galaxy->render(EME2000, mv, p);
    glEnable( GL_DEPTH_TEST );
    // Sort the bodies by distance to camera, near to far
    qSort(m_bodies.begin(), m_bodies.end(), Sort(this));
    // First pass, render opaque objects near to far
    for (int i = 0; i < m_bodies.size(); ++i) {
        m_bodies.at(i)->render(mv, p, RenderMode::Opaque);
    }
    // Second pass, render transluscent objects far to near
    for (int i = m_bodies.size()-1; i >= 0; --i) {
        m_bodies.at(i)->render(mv, p, RenderMode::Translucent);
    }
}

void ViewItem::renderTo(QOpenGLFramebufferObject *fbo)
{
    m_mutex.lock();
    const Eigen::Affine3d &mv = m_camera->modelView();
    const Eigen::Affine3d &p = m_camera->projection();

    if (m_nBlurPass > 0) {
        // Generate light map, use a smaller fbo for efficiency
        glViewport(0, 0, m_postProcessFbo1->size().width(),  m_postProcessFbo1->size().height());
        m_postProcessFbo1->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable( GL_DEPTH_TEST );
        foreach (Body* body, m_bodies) {
            body->render(mv, p, RenderMode::LightSource);
        }
        m_postProcessFbo1->release();
    }

    for (int i = 0; i < m_nBlurPass; ++i) {
        // Apply horizontal blur
        m_postProcessFbo2->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_2D, m_postProcessFbo1->texture());
        m_screenQuad->renderBlurred(Eigen::Affine3d::Identity(), Eigen::Affine3d::Identity(), Eigen::Affine3d::Identity(), ScreenQuad::HorizontalBlur);
        m_postProcessFbo2->release();

        // Apply vertical blur
        m_postProcessFbo1->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_2D, m_postProcessFbo2->texture());
        m_screenQuad->renderBlurred(Eigen::Affine3d::Identity(), Eigen::Affine3d::Identity(), Eigen::Affine3d::Identity(), ScreenQuad::VerticalBlur);
        m_postProcessFbo1->release();
    }

    // Render the scene
    if (m_antiAliasingType == MSAA) {
        // Multisampled antialiasing
        glViewport(0, 0, m_multiSampleFbo->size().width(),  m_multiSampleFbo->size().height());
        m_multiSampleFbo->bind();
        renderScene(m_multiSampleFbo->size().width(),  m_multiSampleFbo->size().height());
        m_multiSampleFbo->release();
        QOpenGLFramebufferObject::blitFramebuffer(m_simpleFbo, m_multiSampleFbo);
    } else if (m_antiAliasingType == SSAA) {
        // Supersampled antialiasing
        glViewport(0, 0, m_superSampleFbo->size().width(),  m_superSampleFbo->size().height());
        m_superSampleFbo->bind();
        GLfloat lineWidth;
        glGetFloatv(GL_LINE_WIDTH, &lineWidth);
        glLineWidth(lineWidth*2.0);
        renderScene(m_superSampleFbo->size().width(),  m_superSampleFbo->size().height());
        glLineWidth(lineWidth);
        m_superSampleFbo->release();
    } else {
        // No antialiasing
        glViewport(0, 0, m_simpleFbo->size().width(),  m_simpleFbo->size().height());
        m_simpleFbo->bind();
        renderScene(m_simpleFbo->size().width(),  m_simpleFbo->size().height());
        m_simpleFbo->release();

        if (m_antiAliasingType == FXAA) {
            m_simpleFbo2->bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, m_simpleFbo2->size().width(),  m_simpleFbo2->size().height());
            glBindTexture(GL_TEXTURE_2D, m_simpleFbo->texture());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            m_screenQuad->setResolution(m_simpleFbo2->width(), m_simpleFbo2->height());
            m_screenQuad->renderFXAA(Eigen::Affine3d::Identity(),Eigen::Affine3d::Identity(), Eigen::Affine3d::Identity());
            m_simpleFbo2->release();
        }
    }

    QOpenGLFramebufferObject *sceneFbo = (m_antiAliasingType == SSAA) ? m_superSampleFbo : m_simpleFbo;
    if (m_antiAliasingType == FXAA) sceneFbo = m_simpleFbo2;
    glViewport(0, 0, fbo->size().width(),  fbo->size().height());
    fbo->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (m_nBlurPass > 0) {
        // Add the blurred texture to the scene
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_postProcessFbo1->texture());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sceneFbo->texture());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        m_screenQuad->renderCombinedTextures(Eigen::Affine3d::Identity(),Eigen::Affine3d::Identity(), Eigen::Affine3d::Identity());
    } else {
         // No blur, just the scene texture
        glBindTexture(GL_TEXTURE_2D, sceneFbo->texture());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        m_screenQuad->render(Eigen::Affine3d::Identity(),Eigen::Affine3d::Identity(), Eigen::Affine3d::Identity());
    }
    fbo->release();

    m_mutex.unlock();
}

void ViewItem::resizeGL(int width, int height)
{
    m_mutex.lock();
    m_camera->setAspectRatio((float)width/(float)height);

    QOpenGLFramebufferObjectFormat fboFormat;
    fboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    fboFormat.setSamples(4);

    int smallWidth = width/4;
    int smallHeight = height/4;
    if (m_multiSampleFbo) delete m_multiSampleFbo;
    m_multiSampleFbo = new QOpenGLFramebufferObject(width, height, fboFormat);

    fboFormat.setSamples(0);
    if (m_superSampleFbo) delete m_superSampleFbo;
    m_superSampleFbo = new QOpenGLFramebufferObject(width*2, height*2, fboFormat);
    if (m_simpleFbo) delete m_simpleFbo;
    m_simpleFbo = new QOpenGLFramebufferObject(width, height, fboFormat);
    if (m_simpleFbo2) delete m_simpleFbo2;
    m_simpleFbo2 = new QOpenGLFramebufferObject(width, height, fboFormat);
    if (m_postProcessFbo2) delete m_postProcessFbo2;
    m_postProcessFbo2 = new QOpenGLFramebufferObject(smallWidth, smallHeight, fboFormat);
    if (m_postProcessFbo1) delete m_postProcessFbo1;
    m_postProcessFbo1 = new QOpenGLFramebufferObject(smallWidth, smallHeight, fboFormat);

    m_screenQuad->setBlurResolution(smallWidth, smallHeight);
    TextBillboard::setResolution(QSizeF(width, height));
    m_mutex.unlock();
}

void ViewItem::addBody(Body *body)
{
    m_bodies.append(body);
    m_bodiesNames.append(body->name());
    emit bodyAdded();
    foreach (Body* sat, body->satellites()) {
        if (!m_bodies.contains(sat))
            addBody(sat);
    }
}

void ViewItem::selectBody(Body *body)
{
    m_mutex.lock();
    m_selectedBody = body;

    m_camera->setCenter(body->center());
    m_camera->setSceneRadius(body->radius()*1.3);
    m_mutex.unlock();

    emit selectionChanged();
    emit distanceToGroundChanged();
}

void ViewItem::animate()
{
    m_mutex.lock();
    Eigen::Vector3d oldBodyCenterd = m_selectedBody->center();
    m_sun->setReferenceFrame(Eigen::Affine3d::Identity());

    double time = m_timeline.currentTime();
    m_sun->setTime(time);

    Eigen::Vector3d bodyCenter = m_selectedBody->center();
    Eigen::Vector3d diff = bodyCenter - oldBodyCenterd;

    m_camera->setPosition(m_camera->position() + diff);
    m_camera->setCenter(bodyCenter);
    m_mutex.unlock();
}

void ViewItem::pickObject(int x, int y)
{
    glViewport(0, 0, m_simpleFbo->size().width(),  m_simpleFbo->size().height());
    m_simpleFbo->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable( GL_DEPTH_TEST );
    foreach (Body* body, m_bodies) {
        body->render(m_camera->modelView(), m_camera->projection(), RenderMode::Picking);
    }
    m_simpleFbo->release();

    QRgb color = m_simpleFbo->toImage().pixel(x, y);
    int ob = qRed(color)*256*256+qGreen(color)*256+qBlue(color);

    foreach (Body* body, m_bodies) {
        if (ob == body->objectId()) {
            selectBody(body);
            m_camera->goToCenter();
            return;
        }
    }
}

void ViewItem::goToObject(const QString& name)
{
    foreach (Body* body, m_bodies) {
        if (body->name() == name) {
            selectBody(body);
            m_camera->goToCenter();
            return;
        }
    }
}

void ViewItem::setShowAxis(bool showAxis)
{
    Body::setShowAxis(showAxis);
    emit showAxisChanged();
}

void ViewItem::setShowOrbits(bool showOrbits)
{
    Body::setShowOrbit(showOrbits);
    emit showOrbitsChanged();
}

void ViewItem::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_M :
        if (m_antiAliasingType == NOAA) m_antiAliasingType = MSAA;
        else if (m_antiAliasingType == MSAA) m_antiAliasingType = SSAA;
        else m_antiAliasingType = NOAA;
        break;
    case Qt::Key_Plus :
        m_timeline.speedUp();
        break;
    case Qt::Key_Minus :
        m_timeline.speedDown();
        break;
    case Qt::Key_R :
        m_timeline.realTime();
        break;
    case Qt::Key_A :
        Body::setShowAxis(!Body::showAxis());
        emit showAxisChanged();
        break;
    case Qt::Key_O :
        Body::setShowOrbit(!Body::showOrbit());
        emit showOrbitsChanged();
        break;
    case Qt::Key_F :
        if (window()->visibility() == QWindow::FullScreen)
            window()->showMaximized();
        else
            window()->showFullScreen();
        break;
    case Qt::Key_Escape :
        window()->close();
        break;
    default :
        QQuickItem::keyPressEvent(event);
        break;
    }
}

void ViewItem::mouseDoubleClickEvent(QMouseEvent* event)
{
    if ((event->button() == Qt::LeftButton) && (event->modifiers() == Qt::NoButton)) {
        if (m_node) {
            m_node->schedulePick(event->pos());
        }
    } else if ((event->button() == Qt::MiddleButton) && (event->modifiers() == Qt::NoButton)) {
        m_camera->goToCenter();
    }
}

void ViewItem::mousePressEvent(QMouseEvent *event)
{
    m_mouseLastPosition = QVector2D(event->pos());
}

void ViewItem::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {

        QVector2D diff = QVector2D(event->pos()) - m_mouseLastPosition;
        if (diff.length() == 0.0) {
            // Happens sometimes on touch screens.
            return;
        }

        m_mutex.lock();
        Eigen::Vector3d rotationTangent = Eigen::Vector3d(diff.x(), -diff.y(), 0.0);
        Eigen::Vector3d directionOfCamera = Eigen::Vector3d(0.0, 0.0, 1.0);
        Eigen::Vector3d rotationAxis = rotationTangent.cross(directionOfCamera);
        rotationAxis = m_camera->modelView().inverse().rotation()*rotationAxis;
        rotationAxis.normalize();
        Eigen::Quaterniond q(Eigen::AngleAxisd(qDegreesToRadians(diff.length()/10.0), rotationAxis));
        m_camera->rotateAroundCenter(q);
        m_mutex.unlock();
    } else if (event->buttons() == Qt::RightButton) {

        QVector2D diff = QVector2D(event->pos()) - m_mouseLastPosition;
        if (diff.length() == 0.0) {
            // Happens sometimes on touch screens.
            return;
        }

        m_mutex.lock();
        Eigen::Vector3d rotationTangent = Eigen::Vector3d(diff.x(), -diff.y(), 0.0);
        Eigen::Vector3d directionOfCamera = Eigen::Vector3d(0.0, 0.0, 1.0);
        Eigen::Vector3d rotationAxis = rotationTangent.cross(directionOfCamera);
        rotationAxis = m_camera->modelView().inverse().rotation()*rotationAxis;
        rotationAxis.normalize();
        Eigen::Quaterniond q(Eigen::AngleAxisd(qDegreesToRadians(diff.length()/10.0), rotationAxis));
        m_camera->rotate(q);
        m_mutex.unlock();
    }
    m_mouseLastPosition = QVector2D(event->pos());
}

void ViewItem::wheelEvent(QWheelEvent *event)
{
    zoom((qreal)event->delta()); //-120 ou 120
}

void ViewItem::touchEvent(QTouchEvent *event)
{
    if (event->touchPoints().count() == 2) {
//        if (event->touchPointStates() != Qt::TouchPointMoved)
//            return;
        QTouchEvent::TouchPoint p1 = event->touchPoints().at(0);
        QTouchEvent::TouchPoint p2 = event->touchPoints().at(1);
        QPointF lastpos = p1.lastScreenPos()-p2.lastScreenPos();
        QPointF newpos = p1.screenPos()-p2.screenPos();
        qreal delta = lastpos.manhattanLength() - newpos.manhattanLength(); //entre -30 et 30 en gros
        zoom(delta);
    } else {
        QQuickItem::touchEvent(event);
    }
}

void ViewItem::zoom(qreal delta)
{
    m_mutex.lock();
    Eigen::Vector3d cameraToCenter = m_camera->center() - m_camera->position();
    Eigen::Vector3d translation = cameraToCenter - cameraToCenter.normalized()*m_selectedBody->radius();
    if (delta > 0.0) {
        translation /= 20.0;
    } else {
        translation /= -20.0;
    }
    Eigen::Vector3d newPos = m_camera->position()-translation;
    if ((m_camera->center() - newPos).norm() < m_selectedBody->radius())
        return;

    m_camera->setPosition(newPos);
//    m_camera->moveTo(newPos);
    m_mutex.unlock();
}

bool ViewItem::closerToCamera(const Body* body1, const Body* body2)
{
    Eigen::Vector3d camPos = m_camera->position();

    double d1 = (body1->center()-camPos).norm();
    double d2 = (body2->center()-camPos).norm();
    return d1 < d2;
}

#include "viewitem.moc"
