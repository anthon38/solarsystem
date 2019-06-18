#ifndef VIEWITEM_H
#define VIEWITEM_H

#include "camera.h"
#include "body.h"
#include "timeline.h"
#include "renderable/galaxy.h"
#include "renderable/screenquad.h"

#include <QQuickItem>
#include <QOpenGLFramebufferObject>
#include <QMutex>
#include <QStringList>

class TextureNode;

class ViewItem : public QQuickItem, protected QOpenGLFunctions
{
    Q_OBJECT
    Q_ENUMS(AntiAliasing)
    Q_PROPERTY(QString date READ date NOTIFY dateUpdated)
    Q_PROPERTY(QString selection READ selection NOTIFY selectionChanged)
    Q_PROPERTY(qreal distanceToGround READ distanceToGround NOTIFY distanceToGroundChanged)
    Q_PROPERTY(int blurPass READ nBlurPass WRITE setNBlurPass NOTIFY nBlurChanged)
    Q_PROPERTY(int antiAliasingType READ antiAliasingType WRITE setAntiAliasingType NOTIFY antialiasingTypeChanged)
    Q_PROPERTY(QList<int> aaTypes READ aaTypes NOTIFY aaTypesChanged)
    Q_PROPERTY(QStringList bodies READ bodies NOTIFY bodyAdded)
    Q_PROPERTY(bool showAxis READ showAxis WRITE setShowAxis NOTIFY showAxisChanged)
    Q_PROPERTY(bool showOrbits READ showOrbits WRITE setShowOrbits NOTIFY showOrbitsChanged)
    Q_PROPERTY(qint64 timeLineRate READ timeLineRate NOTIFY timeLineRateChanged)

public:

    enum AntiAliasing {
        NOAA = 0,
        MSAA = 1,
        SSAA = 2,
        FXAA = 3
    };

    explicit ViewItem(QQuickItem * parent = 0);
    ~ViewItem();
    QString date() const {return m_timeline.dateTime().toString();}
    QString selection() const;
    qreal distanceToGround() const;
    int nBlurPass() const {return m_nBlurPass;}
    void setNBlurPass(int nPass);
    int antiAliasingType() const {return m_antiAliasingType;}
    void setAntiAliasingType(int type);
    QList<int> aaTypes() const {return m_aaTypes;}
    QStringList bodies() const {return m_bodiesNames;}
    Q_INVOKABLE void goToObject(const QString& name);
    bool showAxis() {return Body::showAxis();}
    void setShowAxis(bool showAxis);
    bool showOrbits() {return Body::showOrbit();}
    void setShowOrbits(bool showOrbits);

    void renderTo(QOpenGLFramebufferObject *fbo);
    void pickObject(int x, int y);

    // Time line
    qint64 timeLineRate() {return m_timeline.rate();}
    Q_INVOKABLE void realTime() {m_timeline.realTime();}
    Q_INVOKABLE void realRate() {m_timeline.realRate();}
    Q_INVOKABLE void speedUp() {m_timeline.speedUp();}
    Q_INVOKABLE void speedDown() {m_timeline.speedDown();}
    Q_INVOKABLE void pause() {m_timeline.pause();}

signals:
    void dateUpdated();
    void selectionChanged();
    void distanceToGroundChanged();
    void bodyAdded();
    void showAxisChanged();
    void showOrbitsChanged();
    void timeLineRateChanged();
    void nBlurChanged();
    void antialiasingTypeChanged();
    void aaTypesChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);

    void keyPressEvent(QKeyEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void touchEvent(QTouchEvent *event);

private slots:
    void animate();

private:
    void init();
    void renderScene(int width, int height);
    void resizeGL(int width, int height);
    void addBody(Body *body);
    void selectBody(Body* body);
    void zoom(qreal delta);

    bool closerToCamera(const Body* body1, const Body* body2);

    QMutex m_mutex;

    QOpenGLFramebufferObject *m_multiSampleFbo;
    QOpenGLFramebufferObject *m_superSampleFbo;
    QOpenGLFramebufferObject *m_simpleFbo;
    QOpenGLFramebufferObject *m_simpleFbo2;
    QOpenGLFramebufferObject *m_postProcessFbo1;
    QOpenGLFramebufferObject *m_postProcessFbo2;
    ScreenQuad *m_screenQuad;
    int m_nBlurPass;
    int m_antiAliasingType;
    QList<int> m_aaTypes;

    TextureNode *m_node;
    Timeline m_timeline;
    Camera *m_camera;
    Body *m_selectedBody;
    QVector2D m_mouseLastPosition;
    QSize m_size;

    Galaxy *m_galaxy;
    Body *m_sun;
    QList<Body*> m_bodies;
    QStringList m_bodiesNames;
    Eigen::Affine3d EME2000;

    class Sort
    {
    public:
        Sort(ViewItem* p) : m(p) {}
        bool operator() ( const Body* body1, const Body* body2 )
        {
            return m->closerToCamera(body1, body2);
        }
    private:
        ViewItem* m;
    };

};

#endif // VIEWITEM_H
