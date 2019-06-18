#include "pointobject.h"

float PointObject::PointSize(10.0);

PointObject::PointObject(const QVector3D &color, QObject *parent)
    : Pickable(parent)
    , m_vertexBuffer(QOpenGLBuffer::VertexBuffer)
    , m_colorBuffer(QOpenGLBuffer::VertexBuffer)
    , m_pointSize(10.0)
{
    setShaders(m_program, "coloredPoint");
    checkProgram(m_program);

    setShaders(m_programColor, "pointSolidColor");
    checkProgram(m_programColor);

    m_vertices.append(QVector3D());
    m_colors.append(color);

    m_program.bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
    m_program.bindAttributeLocation("color", PROGRAM_COLOR_ATTRIBUTE);
    m_programColor.bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);

    m_dataSize = m_vertices.size();
    // PointObject vertex buffer init
    m_vertexBuffer.create();
    m_vertexBuffer.bind();
    m_vertexBuffer.allocate(m_vertices.constData(), m_vertices.size() * sizeof(QVector3D));
    m_vertexBuffer.release();
    m_vertices.clear();
    // PointObject color buffer init
    m_colorBuffer.create();
    m_colorBuffer.bind();
    m_colorBuffer.allocate(m_colors.constData(), m_colors.size() * sizeof(QVector3D));
    m_colorBuffer.release();
    m_colors.clear();

    createVAO();
}

PointObject::~PointObject()
{
    m_vertexBuffer.destroy();
    m_colorBuffer.destroy();
}

void PointObject::createVAO()
{
    m_vao.create();
    m_vao.bind();

    m_program.enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_program.enableAttributeArray(PROGRAM_COLOR_ATTRIBUTE);
    m_programColor.enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);

    m_vertexBuffer.bind();
    m_program.setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3);
    m_programColor.setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3);
    m_vertexBuffer.release();

    m_colorBuffer.bind();
    m_program.setAttributeBuffer(PROGRAM_COLOR_ATTRIBUTE, GL_FLOAT, 0, 3);
    m_colorBuffer.release();

    m_vao.release();

    m_program.disableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_program.disableAttributeArray(PROGRAM_COLOR_ATTRIBUTE);
    m_programColor.disableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
}

void PointObject::render(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection)
{
    m_program.bind();

    setUniformMatrix(m_program.uniformLocation("modelViewMatrix"), view*model);
    setUniformMatrix(m_program.uniformLocation("projectionMatrix"), projection);
    m_program.setUniformValue("pointSize", PointSize);
    m_program.setUniformValue("logZbufferC", m_logZbufferC);
    m_program.setUniformValue("C", m_C);
    m_program.setUniformValue("alpha", m_alpha);

    m_vao.bind();
    glEnable(GL_BLEND);
    glDrawArrays(GL_POINTS, 0, m_dataSize);
    glDisable(GL_BLEND);
    m_vao.release();

    m_program.release();
}

void PointObject::renderSolidColor(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection)
{
    m_programColor.bind();

    m_programColor.setUniformValue("color", m_color);
    setUniformMatrix(m_programColor.uniformLocation("modelViewMatrix"), view*model);
    setUniformMatrix(m_programColor.uniformLocation("projectionMatrix"), projection);
    m_programColor.setUniformValue("pointSize", PointSize);
    m_programColor.setUniformValue("logZbufferC", m_logZbufferC);
    m_programColor.setUniformValue("C", m_C);

    m_vao.bind();
    glDrawArrays(GL_POINTS, 0, m_dataSize);
    m_vao.release();

    m_programColor.release();
}
