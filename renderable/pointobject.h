#ifndef POINTOBJECT_H
#define POINTOBJECT_H

#include "pickable.h"

#include <QOpenGLBuffer>

class PointObject : public Pickable
{
public:
    PointObject(const QVector3D &color = QVector3D(0.5, 0.5, 0.5), QObject *parent = 0);
    ~PointObject();
    void createVAO();
    void render(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection);
    void renderSolidColor(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection);
    static float pointSize() {return PointSize;}
    static void setPointSize(float pointSize) {PointSize = pointSize;}

private:
    static float PointSize;

    QVector<QVector3D> m_vertices;
    QVector<QVector3D> m_colors;

    QOpenGLBuffer m_vertexBuffer;
    QOpenGLBuffer m_colorBuffer;

    float m_pointSize;
};

#endif // POINTOBJECT_H
