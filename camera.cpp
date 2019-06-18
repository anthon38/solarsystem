#include "camera.h"

#include <QtMath>

QVariant Vector3dInterpolator(const Eigen::Vector3d &start, const Eigen::Vector3d &end, qreal progress)
{
    Eigen::Vector3d value(start*(1.0d-(double)progress)+end*(double)progress);
    return QVariant::fromValue(value);
}

QVariant QuaterniondInterpolator(const Eigen::Quaterniond &start, const Eigen::Quaterniond &end, qreal progress)
{
    return QVariant::fromValue(start.slerp((double)progress, end));
}

Camera::Camera(QObject *parent)
    : QObject(parent)
    , m_position(0.0, 1.0, 0.0)
    , m_sceneCenter(0.0, 0.0, 0.0)
    , m_upVector(0.0, 0.0, 1.0)
    , m_fov(45.0)
    , m_aspectRatio(1.0)
    , m_zNear(1.0e-3)
    , m_zFar(1.0e5)
    , m_zNearCoefficient(1.0e-8)
    , m_zClippingCoefficient(1.0e6)
{
    qRegisterAnimationInterpolator<Eigen::Vector3d>(Vector3dInterpolator);
    qRegisterAnimationInterpolator<Eigen::Quaterniond>(QuaterniondInterpolator);

    m_goToAnimation = new QSequentialAnimationGroup(this);

    m_orientationAnimation = new QPropertyAnimation(this, "orientation");
    m_orientationAnimation->setEasingCurve(QEasingCurve::OutCirc);
    m_orientationAnimation->setDuration(1000);
    m_goToAnimation->addAnimation(m_orientationAnimation);

    m_positionAnimation = new QPropertyAnimation(this, "position");
    m_positionAnimation->setEasingCurve(QEasingCurve::OutCirc);
    m_positionAnimation->setDuration(2000);
    m_goToAnimation->addAnimation(m_positionAnimation);

    m_moveToAnimation = new QPropertyAnimation(this, "position");
    m_moveToAnimation->setEasingCurve(QEasingCurve::OutCirc);
    m_moveToAnimation->setDuration(500);

    m_orientation.setIdentity();
    updateModelView();
    updateProjection();
}

Camera::~Camera()
{
}

void Camera::setPosition(const Eigen::Vector3d &position)
{
    m_position = position;
    updateModelView();
    emit positionChanged();
}

void Camera::setUpVector(const Eigen::Vector3d &upVector)
{
    m_upVector = upVector;
    updateModelView();
}

void Camera::setOrientation(const Eigen::Quaterniond &orientation)
{
    m_orientation = orientation;
    updateModelView();
}

void Camera::setCenter(const Eigen::Vector3d &center)
{
    m_sceneCenter = center;
    updateModelView();
}

void Camera::rotateAroundCenter(const Eigen::Quaterniond &q)
{

    m_position = m_sceneCenter+q*(m_position-m_sceneCenter);
    m_upVector = q*m_upVector;
    m_orientation = m_orientation*q.inverse();

    updateModelView();
    emit positionChanged();
}

void Camera::rotate(const Eigen::Quaterniond &q)
{
    m_orientation = m_orientation*q;
    m_upVector = q.inverse()*m_upVector;
    updateModelView();
}

void Camera::lookAt(const Eigen::Vector3d &target)
{
    m_orientation = lookAtQuaternion(target);
    updateModelView();
}

Eigen::Quaterniond Camera::lookAtQuaternion(const Eigen::Vector3d &target)
{
    Eigen::Vector3d w = (m_position - target).normalized();
    Eigen::Vector3d u = m_upVector.cross(w).normalized();
    Eigen::Vector3d v = w.cross(u);
    Eigen::Matrix4d rot;
    rot <<  u.x(), u.y(), u.z(), 0.0d,
            v.x(), v.y(), v.z(), 0.0d,
            w.x(), w.y(), w.z(), 0.0d,
            0.0d,  0.0d,  0.0d,  1.0d;
    Eigen::Affine3d aff(rot);
    return Eigen::Quaterniond(aff.rotation()).normalized();
}

void Camera::setFov(float fov)
{
    m_fov = fov;
    updateProjection();
}

void Camera::setAspectRatio(float aspectRatio)
{
    m_aspectRatio = aspectRatio;
    updateProjection();
}

void Camera::setZNear(double zNear)
{
    m_zNear = zNear;
    updateProjection();
}

void Camera::setZFar(double zFar)
{
    m_zFar = zFar;
    updateProjection();
}

void Camera::setSceneRadius(double sceneRadius)
{
    m_sceneRadius = sceneRadius;
    m_zNear = m_zNearCoefficient*m_zClippingCoefficient*m_sceneRadius;
    m_zFar = (m_position-m_sceneCenter).norm()+m_zClippingCoefficient*m_sceneRadius;
    updateProjection();
}

void Camera::updateModelView()
{
    m_orientation.normalize();
    m_modelView = Eigen::Affine3d(m_orientation);
    m_modelView.translate(-m_position);
}

void Camera::updateProjection()
{
    double f = 1.0d / tan(qDegreesToRadians(m_fov)/2.0d);
    Eigen::Matrix4d mat;
    mat <<  f / m_aspectRatio, 0.0, 0.0,                               0.0,
            0.0,               f,   0.0,                               0.0,
            0.0,               0.0, (m_zFar+m_zNear)/(m_zNear-m_zFar), (2.0*m_zFar*m_zNear)/(m_zNear-m_zFar),
            0.0,               0.0, -1.0,                              0.0;
    m_projection = Eigen::Affine3d(mat);
}

void Camera::goToCenter()
{
    m_orientationAnimation->setStartValue(QVariant::fromValue(m_orientation));
    Eigen::Quaterniond target = lookAtQuaternion(m_sceneCenter);
    m_orientationAnimation->setEndValue(QVariant::fromValue(target));

    const float vfov = qDegreesToRadians(m_fov);
    const float hFov = 2.0 * atan( tan(vfov/2.0) * m_aspectRatio );
    const double yview = m_sceneRadius / sin(vfov/2.0);
    const double xview = m_sceneRadius / sin(hFov/2.0);
    const double distance = std::max(xview, yview);
    Eigen::Vector3d positionToCenter = m_position-m_sceneCenter - distance*((m_position-m_sceneCenter).normalized());

    m_positionAnimation->setStartValue(QVariant::fromValue(m_position));
    Eigen::Vector3d endvalue = m_position - positionToCenter;
    m_positionAnimation->setEndValue(QVariant::fromValue(endvalue));

    m_goToAnimation->start();
}

void Camera::moveTo(const Eigen::Vector3d &position)
{
//    if (m_moveToAnimation->state() == QPropertyAnimation::Running) {
////        // jump to end of animation, without any further knowledge of it:
////        viewAnimation->targetObject()->setProperty(viewAnimation->propertyName(),
////                                                   viewAnimation->endValue());
//        Eigen::Vector3d currentValue = m_moveToAnimation->currentValue().value<Eigen::Vector3d>();
//        m_moveToAnimation->setStartValue(QVariant::fromValue(currentValue));
//        m_moveToAnimation->setEndValue(QVariant::fromValue(position));
//    } else {
        m_moveToAnimation->setStartValue(QVariant::fromValue(m_position));
        m_moveToAnimation->setEndValue(QVariant::fromValue(position));

        m_moveToAnimation->start();
//    }
}
