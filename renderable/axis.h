#ifndef AXIS_H
#define AXIS_H

#include "renderable.h"

#include <QOpenGLBuffer>

class Axis : public Renderable
{
public:
    Axis(float length = 1.0, QObject *parent = 0);
    ~Axis();
    void createVAO();
    void render(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection);

private:
    QVector<QVector3D> m_vertices;
    QVector<QVector3D> m_colors;

    QOpenGLBuffer m_vertexBuffer;
    QOpenGLBuffer m_colorBuffer;
};

#endif // AXIS_H
