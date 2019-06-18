#ifndef SCREENQUAD_H
#define SCREENQUAD_H

#include "renderable.h"

#include <QOpenGLBuffer>

class ScreenQuad : public Renderable
{
public:
    enum Blur { NoBlur,
                HorizontalBlur,
                VerticalBlur };

    ScreenQuad(QObject *parent = 0);
    ~ScreenQuad();
    void createVAO();
    void render(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection);
    void setResolution(int width, int height);
    void setBlurResolution(int width, int height);
    void renderBlurred(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection, Blur blurType);
    void renderCombinedTextures(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection);
    void renderFXAA(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection);

private:
    QOpenGLShaderProgram m_programBlurred;
    QOpenGLShaderProgram m_programCombined;
    QOpenGLShaderProgram m_programFXAA;

    QVector<QVector3D> m_vertices;
    QVector<QVector2D> m_texCoords;

    QOpenGLBuffer m_vertexBuffer;
    QOpenGLBuffer m_texcoordBuffer;

    int m_width;
    int m_height;
};

#endif // SCREENQUAD_H
