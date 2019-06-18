#include "renderable.h"
#include "path.h"

#include <QOpenGLContext>
#include <QFile>

Renderable::Renderable(QObject *parent)
    : QObject(parent)
    , m_alpha(1.0)
    , m_useVao(false)
{
    initializeOpenGLFunctions();
    if (QOpenGLContext::currentContext()->format().renderableType() == QSurfaceFormat::OpenGL) {
        if (QOpenGLContext::currentContext()->format().majorVersion() >= 3) {
            m_useVao = true;
        } else if (QOpenGLContext::currentContext()->hasExtension("GL_ARB_vertex_array_object ")) {
            m_useVao = true;
        }
    } else if (QOpenGLContext::currentContext()->format().renderableType() == QSurfaceFormat::OpenGLES) {
        if (QOpenGLContext::currentContext()->hasExtension("GL_OES_vertex_array_object ")) {
            m_useVao = true;
        }
    }
    connect(QOpenGLContext::currentContext(), SIGNAL(aboutToBeDestroyed()), this, SLOT(cleanup()), Qt::DirectConnection);

    m_C = 1.0;
    const float farplane = 1000.0*5874000.000;
    m_logZbufferC = 2.0 / log(farplane*m_C + 1.0);

//    // http://outerra.blogspot.fr/2013/07/logarithmic-depth-buffer-optimizations.html
//    m_logZbufferC = 2.0 / log(farplane + 1.0);

}

Renderable::~Renderable()
{
}

void Renderable::setCustomShader(const QString &name)
{
    m_program.removeAllShaders();
    setShaders(m_program, name);
    checkProgram(m_program);

    m_program.bind();
    // Textures
    m_program.setUniformValue("texture0", 0);
    m_program.setUniformValue("texture1", 1);

    // Light settings
    m_program.setUniformValue("light.Ld", QVector4D(1.0, 1.0, 1.0, 1.0));
    m_program.release();
}

void Renderable::cleanup()
{
    m_vao.destroy();
    qDebug()<<"cleanup Renderable"<<m_useVao;
}

void Renderable::setShaders(QOpenGLShaderProgram &program, const QString &shaderName)
{
    setShader(program, QOpenGLShader::Vertex, shaderName+".vert");
    setShader(program, QOpenGLShader::Fragment, shaderName+".frag");
    program.link();
}

void Renderable::setShader(QOpenGLShaderProgram &program, QOpenGLShader::ShaderType type, const QString &fileName)
{
    QString fullPath = resPath()+shadersDir()+fileName;
    QFile file(fullPath);
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "Renderable: Unable to open file" << fullPath;
        return;
    }
    QByteArray contents = file.readAll();
#ifndef Q_OS_ANDROID
    contents.prepend("#version 120\n");
#else
    contents.prepend("#version 100\n");
#endif
    program.addShaderFromSourceCode(type, contents);
}

void Renderable::checkProgram(const QOpenGLShaderProgram &program) const
{
//    if (!program.isLinked()) {
        QString error = program.log();
        if (!error.isEmpty())
            qWarning()<<"Shaders log: "<<error;
//    }
}

void Renderable::setUniformMatrix(int location, const Eigen::Affine3d &transformation)
{
//    Eigen::Affine3f transfo((transformation).cast<float>());
//    glUniformMatrix4fv(location, 1, GL_FALSE, transfo.data());
    glUniformMatrix4fv(location, 1, GL_FALSE, transformation.template cast<float>().data());
}

void Renderable::setUniformMatrix(int location, const Eigen::Matrix4d &matrix)
{
    Eigen::Matrix4f mat = matrix.cast<float>();
    glUniformMatrix4fv(location, 1, GL_FALSE, mat.data());
}

void Renderable::setUniformMatrix(int location, const Eigen::Matrix3d &matrix)
{
    Eigen::Matrix3f mat = matrix.cast<float>();
    glUniformMatrix3fv(location, 1, GL_FALSE, mat.data());
}

void Renderable::setUniformVector(int location, const Eigen::Vector4d &vector)
{
    Eigen::Vector4f vect = vector.cast<float>();
    glUniform4fv(location, 1, vect.data());
}

void Renderable::setUniformVector(int location, const Eigen::Vector3d &vector)
{
    Eigen::Vector3f vect = vector.cast<float>();
    glUniform3fv(location, 1, vect.data());
}

