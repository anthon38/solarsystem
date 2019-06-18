#ifndef RENDERABLE_H
#define RENDERABLE_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

#include "Eigen/Geometry"

class Renderable: public QObject, protected QOpenGLFunctions
{
    Q_OBJECT
public:

    enum { PROGRAM_VERTEX_ATTRIBUTE,
           PROGRAM_NORMAL_ATTRIBUTE,
           PROGRAM_COLOR_ATTRIBUTE,
           PROGRAM_TEXTURE_ATTRIBUTE,
           PROGRAM_VERTEX_HIGH_ATTRIBUTE,
           PROGRAM_VERTEX_LOW_ATTRIBUTE };

    Renderable(QObject *parent = 0);
    virtual ~Renderable();
    void setAlpha(float alpha) {m_alpha = alpha;}
    void setCustomShader(const QString& name);
    virtual void createVAO() = 0;
//    virtual void beginRendering() = 0;
//    virtual void endRendering() = 0;
    virtual void render(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection) = 0;

protected slots:
    void cleanup();

protected:
    void setShaders(QOpenGLShaderProgram &program, const QString &shaderName);
    void setShader(QOpenGLShaderProgram &program, QOpenGLShader::ShaderType type, const QString &fileName);
    void checkProgram(const QOpenGLShaderProgram &program) const;

    void setUniformMatrix(int location, const Eigen::Affine3d &transformation);
    void setUniformMatrix(int location, const Eigen::Matrix4d &matrix);
    void setUniformMatrix(int location, const Eigen::Matrix3d &matrix);
    void setUniformVector(int location, const Eigen::Vector4d &vector);
    void setUniformVector(int location, const Eigen::Vector3d &vector);

    float m_logZbufferC;
    float m_C;
    float m_alpha;
    int m_dataSize;
    QOpenGLShaderProgram m_program;
    bool m_useVao;
    QOpenGLVertexArrayObject m_vao;
};

#endif // RENDERABLE_H
