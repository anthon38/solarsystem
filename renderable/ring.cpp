#include "ring.h"

#include <cmath>

Ring::Ring(float innerRadius, float outerRadius, QObject *parent)
    : Pickable(parent)
    , m_vertexBuffer(QOpenGLBuffer::VertexBuffer)
    , m_texcoordBuffer(QOpenGLBuffer::VertexBuffer)
{
    setShaders(m_program, "ring");
    checkProgram(m_program);

    setShaders(m_programColor, "solidColor");
    checkProgram(m_programColor);

    const int slices = 180;
    for (int i=0; i<=slices; ++i) {
        float a = i*2*M_PI/slices;
        float c = cos(a);
        float s = sin(a);

        m_vertices.append(QVector3D(innerRadius*c, innerRadius*s, 0.0f));
        m_vertices.append(QVector3D(outerRadius*c, outerRadius*s, 0.0f));

        m_texCoords.append(0.0);
        m_texCoords.append(1.0);
    }
    m_program.bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
    m_program.bindAttributeLocation("texCoord", PROGRAM_TEXTURE_ATTRIBUTE);
    m_programColor.bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);

    m_dataSize = m_vertices.size();
    // Ring vertex buffer init
    m_vertexBuffer.create();
    m_vertexBuffer.bind();
    m_vertexBuffer.allocate(m_vertices.constData(), m_vertices.size() * sizeof(QVector3D));
    m_vertexBuffer.release();
    m_vertices.clear();
    // Ring texCoord buffer init
    m_texcoordBuffer.create();
    m_texcoordBuffer.bind();
    m_texcoordBuffer.allocate(m_texCoords.constData(), m_texCoords.size() * sizeof(float));
    m_texcoordBuffer.release();
    m_texCoords.clear();

    createVAO();
}

Ring::~Ring()
{
    m_vertexBuffer.destroy();
    m_texcoordBuffer.destroy();
}

void Ring::createVAO()
{
    m_vao.create();
    m_vao.bind();

    m_program.enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_program.enableAttributeArray(PROGRAM_TEXTURE_ATTRIBUTE);
    m_programColor.enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);

    m_vertexBuffer.bind();
    m_program.setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3);
    m_programColor.setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3);
    m_vertexBuffer.release();

    m_texcoordBuffer.bind();
    m_program.setAttributeBuffer(PROGRAM_TEXTURE_ATTRIBUTE, GL_FLOAT, 0, 1);
    m_texcoordBuffer.release();

    m_vao.release();

    m_program.disableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_program.disableAttributeArray(PROGRAM_TEXTURE_ATTRIBUTE);
    m_programColor.disableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
}

void Ring::render(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection)
{
    m_program.bind();

    setUniformMatrix(m_program.uniformLocation("modelViewMatrix"), view*model);
    setUniformMatrix(m_program.uniformLocation("projectionMatrix"), projection);
    setUniformVector(m_program.uniformLocation("light.Position"), view*Eigen::Vector4d(0.0, 0.0, 0.0, 1.0));
    m_program.setUniformValue("logZbufferC", m_logZbufferC);
    m_program.setUniformValue("C", m_C);

    m_vao.bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glEnable(GL_BLEND);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, m_dataSize);
    glDisable(GL_BLEND);
    m_vao.release();

    m_program.release();
}

void Ring::renderSolidColor(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection)
{
    m_programColor.bind();

    m_programColor.setUniformValue("color", m_color);
    setUniformMatrix(m_programColor.uniformLocation("modelViewMatrix"), view*model);
    setUniformMatrix(m_programColor.uniformLocation("projectionMatrix"), projection);
    m_programColor.setUniformValue("logZbufferC", m_logZbufferC);
    m_programColor.setUniformValue("C", m_C);

    m_vao.bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, m_dataSize);
    m_vao.release();

    m_programColor.release();
}
