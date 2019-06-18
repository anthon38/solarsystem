#include "axis.h"

Axis::Axis(float length, QObject *parent)
    : Renderable(parent)
    , m_vertexBuffer(QOpenGLBuffer::VertexBuffer)
    , m_colorBuffer(QOpenGLBuffer::VertexBuffer)
{
    setShaders(m_program, "axis");
    checkProgram(m_program);

    m_vertices.append(QVector3D(0.0, 0.0, 0.0));
    m_colors.append(QVector3D(1.0, 0.0, 0.0));
    m_vertices.append(length*QVector3D(1.0, 0.0, 0.0));
    m_colors.append(QVector3D(1.0, 0.0, 0.0));

    m_vertices.append(QVector3D(0.0, 0.0, 0.0));
    m_colors.append(QVector3D(0.0, 1.0, 0.0));
    m_vertices.append(length*QVector3D(0.0, 1.0, 0.0));
    m_colors.append(QVector3D(0.0, 1.0, 0.0));

    m_vertices.append(QVector3D(0.0, 0.0, 0.0));
    m_colors.append(QVector3D(0.0, 0.0, 1.0));
    m_vertices.append(length*QVector3D(0.0, 0.0, 1.0));
    m_colors.append(QVector3D(0.0, 0.0, 1.0));

    m_program.bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
    m_program.bindAttributeLocation("color", PROGRAM_COLOR_ATTRIBUTE);

    m_dataSize = m_vertices.size();
    // Axis vertex buffer init
    m_vertexBuffer.create();
    m_vertexBuffer.bind();
    m_vertexBuffer.allocate(m_vertices.constData(), m_vertices.size() * sizeof(QVector3D));
    m_vertexBuffer.release();
    m_vertices.clear();
    // Axis color buffer init
    m_colorBuffer.create();
    m_colorBuffer.bind();
    m_colorBuffer.allocate(m_colors.constData(), m_colors.size() * sizeof(QVector3D));
    m_colorBuffer.release();
    m_colors.clear();

    createVAO();
}

Axis::~Axis()
{
    m_vertexBuffer.destroy();
    m_colorBuffer.destroy();
}

void Axis::createVAO()
{
    m_vao.create();
    m_vao.bind();

    m_program.enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_program.enableAttributeArray(PROGRAM_COLOR_ATTRIBUTE);

    m_vertexBuffer.bind();
    m_program.setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3);
    m_vertexBuffer.release();

    m_colorBuffer.bind();
    m_program.setAttributeBuffer(PROGRAM_COLOR_ATTRIBUTE, GL_FLOAT, 0, 3);
    m_colorBuffer.release();

    m_vao.release();

    m_program.disableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_program.disableAttributeArray(PROGRAM_COLOR_ATTRIBUTE);
}

void Axis::render(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection)
{
    m_program.bind();

    setUniformMatrix(m_program.uniformLocation("modelViewMatrix"), view*model);
    setUniformMatrix(m_program.uniformLocation("projectionMatrix"), projection);
    m_program.setUniformValue("logZbufferC", m_logZbufferC);
    m_program.setUniformValue("C", m_C);

    m_vao.bind();
    glDrawArrays(GL_LINES, 0, 2);
    glDrawArrays(GL_LINES, 2, 2);
    glDrawArrays(GL_LINES, 4, 2);
    m_vao.release();

    m_program.release();
}
