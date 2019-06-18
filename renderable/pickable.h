#ifndef PICKABLE_H
#define PICKABLE_H

#include "renderable.h"

class Pickable : public Renderable
{
public:
    Pickable(QObject *parent = 0);
    void setColor(int id);
    void setColor(const QVector3D &color);
    virtual void renderSolidColor(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection) = 0;

protected:
    QOpenGLShaderProgram m_programColor;

    QVector3D m_color;

};

#endif // PICKABLE_H
