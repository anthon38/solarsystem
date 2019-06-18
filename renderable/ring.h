#ifndef RING_H
#define RING_H

#include "pickable.h"

#include <QOpenGLBuffer>

class Ring : public Pickable
{
public:
    Ring(float innerRadius, float outerRadius, QObject *parent = 0);
    ~Ring();
    void createVAO();
    void render(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection);
    void renderSolidColor(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection);

private:
    QVector<QVector3D> m_vertices;
    QVector<float> m_texCoords;

    QOpenGLBuffer m_vertexBuffer;
    QOpenGLBuffer m_normalBuffer;
    QOpenGLBuffer m_texcoordBuffer;
};

#endif // RING_H
