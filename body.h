#ifndef BODY_H
#define BODY_H

#include "renderable/sphere.h"
#include "renderable/ring.h"
#include "renderable/orbit.h"
#include "renderable/axis.h"
#include "renderable/pointobject.h"
#include "renderable/textbillboard.h"
#include "renderable/flare.h"

#include <QString>
#include <QList>

namespace RenderMode {
    enum Mode { Opaque, Translucent, Picking, LightSource };
}

struct Rotation {
    double period;
    double axialTilt;
//    double meridianAngle;
};

class Body: public QObject, protected QOpenGLFunctions
{

public:
    Body(const QString &name, QObject *parent = 0);
    ~Body();
    void setTime(double time/*seconds past epoch*/);
    Eigen::Affine3d referenceFrame() const {return m_referenceFrame;}
    void setReferenceFrame(const Eigen::Affine3d &frame) {m_referenceFrame = frame;}

    void render(const Eigen::Affine3d &view, const Eigen::Affine3d &projection, RenderMode::Mode mode);

    int objectId() const {return m_objectId;}
    QString name() const {return m_name;}
    float radius() const {return m_radius;}
    float boundingRadius() const {return m_boundingRadius;}
    Body* root() const {return m_root;}
    QList<Body*> satellites() const {return m_satellites;}
    Eigen::Vector3d center() const {return m_referenceFrame*Eigen::Vector3d::Zero();}

    void setOnScreenRadius(int radius) {m_onScreenRadius = radius;}
    void setOnScreenDistanceToParent(int distance) {m_onScreenDistanceToParent = distance;}
    static bool showAxis() {return ShowAxis;}
    static void setShowAxis(bool showAxis) {ShowAxis = showAxis;}
    static bool showOrbit() {return ShowOrbit;}
    static void setShowOrbit(bool showOrbit) {ShowOrbit = showOrbit;}
    static void setPointSizeThreshold(float pointSizeThreshold) {
        PointSizeThreshold = pointSizeThreshold;
        PointObject::setPointSize(PointSizeThreshold);
    }

protected:
    GLuint loadTexture(const QString &image);

    static int ObjectID;
    static bool ShowAxis;
    static bool ShowOrbit;
    static float PointSizeThreshold;

    QString m_name;
    int m_objectId;
    float m_radius;
    float m_boundingRadius;
    bool m_isLightSource;
    Sphere *m_sphere;
    Ring *m_ring;
    Orbit *m_orbit;
    Axis *m_axis;
    PointObject *m_pointObject;
    int m_onScreenRadius;
    int m_onScreenDistanceToParent;
    TextBillboard *m_text;
    Flare *m_flare;

    Rotation m_rotation;

    Body *m_root;
    QList<Body*> m_satellites;
    GLuint m_texture;
    GLuint m_nightTexture;
    GLuint m_ringTexture;

    Eigen::Affine3d m_referenceFrame;
    Eigen::Affine3d m_orbitFrame;
    Eigen::Affine3d m_laplaceFrame;
};

#endif // BODY_H
