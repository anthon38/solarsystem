#ifndef TEXTBILLBOARD_H
#define TEXTBILLBOARD_H

#include "pickable.h"

#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>

class TextBillboard : public Pickable
{
public:
    TextBillboard(const QString &text, const QVector3D &color = QVector3D(0.5, 0.5, 0.5), QObject *parent = 0);
    ~TextBillboard();
    void createVAO();
    void render(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection);
    void renderSolidColor(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection);
    static void setResolution(const QSizeF &resolution) {Resolution = resolution;}

private:
    unsigned int nearestPowerOfTwo(unsigned int n) const;

    QVector<QVector3D> m_vertices;
    QVector<QVector2D> m_texCoords;

    QOpenGLBuffer m_vertexBuffer;
    QOpenGLBuffer m_texcoordBuffer;

    QOpenGLFramebufferObject *m_fbo;
    static QSizeF Resolution;
};

#endif // TEXTBILLBOARD_H
