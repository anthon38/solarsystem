#ifndef ORBIT_H
#define ORBIT_H

#include "renderable.h"

#include <QOpenGLBuffer>

struct OrbitalElements {
    double eccentricity;
    double semiMajorAxis; // 10e3m
    double inclination; // rad
    double longitudeOfAscendingNode; // rad
    double argumentOfPeriapsis; // rad
    double meanAnomalyAtEpoch; // rad
    double revolutionPeriod; // s
};

class Orbit : public Renderable
{
public:
    Orbit(const OrbitalElements &elements, const QVector3D &color = QVector3D(0.5, 0.5, 0.5), QObject *parent = 0);
    ~Orbit();
    OrbitalElements elements() const {return m_elements;}

    Eigen::Affine3d orientation() const {return m_orientation;}
    Eigen::Vector2d position(double time) const;

    void createVAO();
    void render(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection);
    void setBodyPosition(Eigen::Vector2d position, double time);

private:
    double eccentricAnomaly(double ecc, double M, double epsilon) const;
    QVector2D doubleToTwoFloats(double value);
    void updateBuffers();

    OrbitalElements m_elements;
    Eigen::Affine3d m_orientation;

    QVector<QVector3D> m_verticesHigh;
    QVector<QVector3D> m_verticesLow;
    QVector<QVector4D> m_colors;

    QOpenGLBuffer m_vertexHighBuffer;
    QOpenGLBuffer m_vertexLowBuffer;
    QOpenGLBuffer m_colorBuffer;

    QVector4D m_color;
    QList<double> m_timeTable;
    int m_lastIndex;
};

#endif // ORBIT_H
