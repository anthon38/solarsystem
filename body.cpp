#include "body.h"
#include "path.h"

#include <cmath>
#include <QSettings>
#include <QStringList>
#include <QImage>
#include <QtMath>

int Body::ObjectID(0);
bool Body::ShowAxis(false);
bool Body::ShowOrbit(true);
float Body::PointSizeThreshold(10.0);

Body::Body(const QString &name, QObject *parent)
    : QObject(parent)
    , m_name(name)
    , m_objectId(++ObjectID)
    , m_radius(0.0)
    , m_boundingRadius(0.0)
    , m_sphere(0)
    , m_ring(0)
    , m_orbit(0)
    , m_axis(0)
    , m_pointObject(0)
    , m_onScreenRadius(0)
    , m_onScreenDistanceToParent(-1)
    , m_text(0)
    , m_flare(0)
    , m_root(0)
    , m_texture(0)
    , m_nightTexture(0)
    , m_ringTexture(0)
{
    float unitcoeff = 1.0;
    initializeOpenGLFunctions();

    m_root = dynamic_cast<Body*>(parent);

    QSettings data(resPath()+"data/data.txt", QSettings::IniFormat);
    data.beginGroup(name);

    m_texture = loadTexture(resPath()+data.value("texture").toString());
    if (data.contains("nightTexture"))
        m_nightTexture = loadTexture(resPath()+data.value("nightTexture").toString());

    float radius = data.value("radius").toFloat()*unitcoeff;
    m_isLightSource = data.contains("lightsource") ? data.value("lightsource").toBool() : false;
    if (m_isLightSource) {
        m_flare = new Flare(this);
    }
    float flattening = data.contains("flattening") ? data.value("flattening").toFloat() : 0.0;
    m_sphere = new Sphere(radius, flattening, this);
    if (data.contains("shader")) {
        m_sphere->setCustomShader(data.value("shader").toString());
    }
    m_sphere->setColor(m_isLightSource ? QVector3D(1.0, 1.0, 1.0) : QVector3D(0.0, 0.0, 0.0));
    m_radius = radius;
    m_boundingRadius = m_radius;

    if (data.contains("ringTexture")) {
        m_ringTexture = loadTexture(resPath()+data.value("ringTexture").toString());
        float innerRadius = data.value("innerRadius").toFloat()*unitcoeff;
        float outerRadius = data.value("outerRadius").toFloat()*unitcoeff;
        m_ring = new Ring(innerRadius, outerRadius, this);
        m_boundingRadius = outerRadius;
    }

    QVector3D color = QVector3D(0.5, 0.5, 0.5);
    if (data.contains("color")) {
        QList<QVariant> vec = data.value("color").toList();
        if (vec.size()!=3)
            qWarning()<<vec<<"color size is not 3";
        color = QVector3D(vec.at(0).toFloat(), vec.at(1).toFloat(), vec.at(2).toFloat());
    }

    m_pointObject = new PointObject(color, this);
    m_pointObject->setColor(m_objectId);

    if (m_root) {
        OrbitalElements elements;
        elements.eccentricity = data.value("eccentricity").toDouble();
        elements.semiMajorAxis = data.value("semimajoraxis").toDouble()*unitcoeff;
        elements.inclination = qDegreesToRadians(data.value("inclination").toDouble());
        elements.longitudeOfAscendingNode = qDegreesToRadians(data.value("ascendingnode").toDouble());
        elements.argumentOfPeriapsis = qDegreesToRadians(data.value("argumentperiapsis").toDouble());
        elements.meanAnomalyAtEpoch = qDegreesToRadians(data.value("meanAnomaly").toDouble());
        elements.revolutionPeriod = data.value("siderealrev").toDouble()*86400.0;
        m_orbit = new Orbit(elements, color, this);
    }

    m_rotation.period = data.value("siderealrot").toDouble()*86400.0;
    m_rotation.axialTilt = qDegreesToRadians(data.value("axialtilt").toDouble());

    m_text = new TextBillboard(m_name, color, this);
    m_text->setColor(m_objectId);

    m_axis = new Axis(2.0*radius, this);

    QStringList satellites = data.value("satellites").toString().split(":");
    data.endGroup();
    foreach (const QString &satellite, satellites) {
        if (!satellite.isEmpty()) {
            m_satellites.append(new Body(satellite, this));
        }
    }
}

Body::~Body()
{
    glDeleteTextures(1, &m_texture);
    glDeleteTextures(1, &m_ringTexture);
    glDeleteTextures(1, &m_nightTexture);
}

void Body::setTime(double time)
{
    double longitudeOfPeriapsis = 0;
    double rot0 = 0;
    if (m_orbit) {
        // Orbital orientation
        m_referenceFrame = m_referenceFrame*m_orbit->orientation();
        m_orbitFrame = m_referenceFrame;
        // Orbital position
        Eigen::Vector2d position = m_orbit->position(time);
        m_orbit->setBodyPosition(position, time);
        m_referenceFrame.translate(Eigen::Vector3d(position.x(), position.y(), 0.0));
        longitudeOfPeriapsis = m_orbit->elements().argumentOfPeriapsis
                                    + m_orbit->elements().longitudeOfAscendingNode;
        rot0 = m_orbit->elements().meanAnomalyAtEpoch;// FIXME: only works for moon and earth by definition
    }
    m_referenceFrame.rotate(Eigen::AngleAxisd(M_PI/2.0-longitudeOfPeriapsis, Eigen::Vector3d::UnitZ()));
    m_referenceFrame.rotate(Eigen::AngleAxisd(m_rotation.axialTilt, Eigen::Vector3d::UnitY()));
    // Draw all satellites in the equatorial plane for the sake of simplicity.
    // TODO: In reality they should be in the Laplace plane, which can be closer to the body's orbital plane.
    m_laplaceFrame = m_referenceFrame;
    m_referenceFrame.rotate(Eigen::AngleAxisd(-M_PI/2.0+longitudeOfPeriapsis, Eigen::Vector3d::UnitZ()));
    m_referenceFrame.rotate(Eigen::AngleAxisd(rot0 + fmod(2.0*M_PI/m_rotation.period*time, 2.0*M_PI), Eigen::Vector3d::UnitZ()));

    foreach (Body* satellite, m_satellites) {
        satellite->setReferenceFrame(m_laplaceFrame);
        satellite->setTime(time);
    }
}

void Body::render(const Eigen::Affine3d &view, const Eigen::Affine3d &projection, RenderMode::Mode mode)
{
    if ( (m_onScreenDistanceToParent >= 0)
       &&(m_onScreenDistanceToParent < PointSizeThreshold*2) ) {
        return;
    }
    if ( mode == RenderMode::Opaque ) {
        if ( ShowAxis ) {
            m_axis->render(m_laplaceFrame, view, projection);
        }
        if (m_onScreenRadius > PointSizeThreshold) {
            if (m_nightTexture) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, m_nightTexture);
                glActiveTexture(GL_TEXTURE0);
            }
            glBindTexture(GL_TEXTURE_2D, m_texture);
            m_sphere->render(m_referenceFrame, view, projection);
        }
        return;
    }

    if ( mode == RenderMode::Translucent ) {
        if (m_onScreenRadius > PointSizeThreshold) {
            if ( ShowOrbit && m_orbit ) {
                m_orbit->render(m_orbitFrame, view, projection);
            }
            if (m_ring) {
                    glBindTexture(GL_TEXTURE_2D, m_ringTexture);
                    m_ring->render(m_referenceFrame, view, projection);
            }
        } else {
            float alpha = ((float)m_onScreenDistanceToParent / (PointSizeThreshold*2.0)) - 1.0;
            if ( (alpha > 1.0) || (m_onScreenDistanceToParent < 0) ) alpha = 1.0;
            m_pointObject->setAlpha(alpha);
            m_pointObject->render(m_referenceFrame, view, projection);
            if ( ShowOrbit && m_orbit ) {
                m_orbit->setAlpha(alpha);
                m_orbit->render(m_orbitFrame, view, projection);
            }
            m_text->setAlpha(alpha);
            m_text->render(m_referenceFrame, view, projection);
        }
        return;
    }

    if ( mode == (RenderMode::Picking) ) {
        if (m_onScreenRadius > PointSizeThreshold) {
            m_sphere->setColor(m_objectId);
            m_sphere->renderSolidColor(m_referenceFrame, view, projection);
            m_sphere->setColor(m_isLightSource ? QVector3D(1.0, 1.0, 1.0) : QVector3D(0.0, 0.0, 0.0));
        } else {
            m_pointObject->renderSolidColor(m_referenceFrame, view, projection);
            m_text->renderSolidColor(m_referenceFrame, view, projection);
        }
        return;
    }

    if ( mode == (RenderMode::LightSource) ) {
        if (m_isLightSource) {
            glBindTexture(GL_TEXTURE_2D, m_texture);
            m_sphere->render(m_referenceFrame, view, projection);
        } else {
            if (m_onScreenRadius > 0) {
                m_sphere->renderSolidColor(m_referenceFrame, view, projection);
            }
        }
        if (m_ring)
            m_ring->renderSolidColor(m_referenceFrame, view, projection);
        return;
    }
}

GLuint Body::loadTexture(const QString &image)
{
    QImage img(image);
    if (img.isNull()) {
        qWarning()<<image<<"Unable to load file, unsupported file format";
        return GLuint();
    }
    QImage glImg = img.mirrored().convertToFormat(QImage::Format_RGBA8888);

    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

//    // OpenGL 4.2
//    int num_mipmaps = ceil(log2(qMax(glImg.width(),glImg.height())))+1;
//    glTexStorage2D(GL_TEXTURE_2D, num_mipmaps-1, GL_RGBA8, glImg.width(), glImg.height());
//    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, glImg.width(), glImg.height(),
//                    GL_RGBA, GL_UNSIGNED_BYTE, glImg.bits());

    GLint internalFormat = GL_RGBA;
#ifndef Q_OS_ANDROID
    if (QOpenGLContext::currentContext()->hasExtension("GL_EXT_texture_compression_s3tc")) {
        internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    }
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, glImg.width(), glImg.height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, glImg.bits());
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    return textureId;
}

//GLuint Body::loadTexture(const QString &image, QOpenGLTexture *texture)
//{
//    QImage img(image);
//    if (img.isNull()) {
//        qWarning("Unable to load file, unsupported file format");
//        return GLuint();
//    }

//    texture = new QOpenGLTexture(img.mirrored());
//    texture->setWrapMode(QOpenGLTexture::Repeat);
//    texture->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::Linear);

////    texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
////    texture->setFormat(QOpenGLTexture::RGBA_DXT5);
////    texture->setSize(img.width(), img.height());
////    texture->setMipLevels(texture->maximumMipLevels());
////    texture->allocateStorage();

////    QImage glImage = img.mirrored().convertToFormat(QImage::Format_RGBA8888);
////    texture->setCompressedData(glImage.byteCount(), glImage.bits());
//////    texture->setData(0, QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, glImage.bits());
////    texture->setWrapMode(QOpenGLTexture::Repeat);
////    texture->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::Linear);

//    return texture->textureId();
//}
