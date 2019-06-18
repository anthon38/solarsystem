#ifndef GALAXY_H
#define GALAXY_H

#include "renderable.h"

#include <QOpenGLBuffer>

class Galaxy : public Renderable
{
public:
    Galaxy(QObject *parent = 0);
    ~Galaxy();
    void createVAO();
    void render(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection);
    void setPointSizeCoeff(float coeff) {m_pointSizeCoeff = coeff;}

private:
    QVector3D spectrumToRgb(const QString &spectrum);
    QVector3D colorIndexToRgb(qreal colorIndex);

    float m_pointSizeCoeff;

    QVector<QVector3D> m_vertices;
    QVector<QVector4D> m_colors;

    QOpenGLBuffer m_vertexBuffer;
    QOpenGLBuffer m_colorBuffer;
};

#endif // GALAXY_H
