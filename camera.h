#ifndef CAMERA_H
#define CAMERA_H

#include <QSequentialAnimationGroup>
#include <QPropertyAnimation>

#include "Eigen/Core"
#include "Eigen/Geometry"

Q_DECLARE_METATYPE(Eigen::Vector3d)
Q_DECLARE_METATYPE(Eigen::Quaterniond)

class Camera : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Eigen::Vector3d position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(Eigen::Vector3d upVector READ upVector WRITE setUpVector)
    Q_PROPERTY(Eigen::Quaterniond orientation READ orientation WRITE setOrientation)


public:
    Camera(QObject *parent = 0);
    ~Camera();
    Eigen::Vector3d position() const {return m_position;}
    void setPosition(const Eigen::Vector3d &position);
    Eigen::Vector3d upVector() const {return m_upVector;}
    void setUpVector(const Eigen::Vector3d &upVector);
    Eigen::Quaterniond orientation() const {return m_orientation;}
    void setOrientation(const Eigen::Quaterniond &orientation);

    Eigen::Vector3d center() const {return m_sceneCenter;}
    void setCenter(const Eigen::Vector3d &center);
    void rotateAroundCenter(const Eigen::Quaterniond &q);
    void rotate(const Eigen::Quaterniond &q);

    void lookAt(const Eigen::Vector3d &target);

    Eigen::Affine3d modelView() const {return m_modelView;}
    Eigen::Affine3d projection() const {return m_projection;}

    float fov() {return m_fov;}
    void setFov(float fov);
    void setAspectRatio(float aspectRatio);
    void setZNear(double zNear);
    void setZFar(double zFar);
    void setSceneRadius(double sceneRadius);

    void goToCenter();
    void moveTo(const Eigen::Vector3d &position);

signals:
    void positionChanged();

private:
    Eigen::Quaterniond lookAtQuaternion(const Eigen::Vector3d &target);
    void updateModelView();
    void updateProjection();

    Eigen::Affine3d m_modelView;
    Eigen::Affine3d m_projection;

    Eigen::Vector3d m_position;
    Eigen::Vector3d m_sceneCenter;
    Eigen::Vector3d m_upVector;
    Eigen::Quaterniond m_orientation;
    float m_fov;
    float m_aspectRatio;
    double m_zNear;
    double m_zFar;
    double m_sceneRadius;
    double m_zNearCoefficient;
    double m_zClippingCoefficient;

    QSequentialAnimationGroup *m_goToAnimation;
    QPropertyAnimation *m_orientationAnimation;
    QPropertyAnimation *m_positionAnimation;
    QPropertyAnimation *m_moveToAnimation;
};

#endif // CAMERA_H
