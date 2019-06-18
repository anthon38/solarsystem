#ifndef SPHERE_H
#define SPHERE_H

#include "pickable.h"

#include <QOpenGLBuffer>

class Sphere : public Pickable
{
public:
    Sphere(float radius, float flattening = 0.0, QObject *parent = 0);
    ~Sphere();
    void createVAO();
    void render(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection);
    void renderSolidColor(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection);

private:
    QVector<int> m_indices;
    QVector<QVector3D> m_vertices;
    QVector<QVector3D> m_normals;
    QVector<QVector2D> m_texCoords;

    QOpenGLBuffer m_indexBuffer;
    QOpenGLBuffer m_vertexBuffer;
    QOpenGLBuffer m_normalBuffer;
    QOpenGLBuffer m_texcoordBuffer;
};

#endif // SPHERE_H
