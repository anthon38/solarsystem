#ifndef FLARE_H
#define FLARE_H

#include "renderable.h"

#include <QOpenGLBuffer>
#include <QOpenGLTexture>

class Flare : public Renderable
{
public:
    Flare(QObject *parent = 0);
    ~Flare();
    void createVAO();
    void render(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection);

protected slots:
    void cleanup();

private:
    QVector<QVector3D> m_vertices;
    QVector<QVector2D> m_texCoords;

    QOpenGLBuffer m_vertexBuffer;
    QOpenGLBuffer m_texcoordBuffer;

    QOpenGLTexture *m_texture;
};

#endif // FLARE_H
