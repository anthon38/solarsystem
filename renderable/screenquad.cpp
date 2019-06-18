#include "screenquad.h"

ScreenQuad::ScreenQuad(QObject *parent)
    : Renderable(parent)
    , m_vertexBuffer(QOpenGLBuffer::VertexBuffer)
    , m_texcoordBuffer(QOpenGLBuffer::VertexBuffer)
{
    setShaders(m_program, "simpleTexture");
    checkProgram(m_program);

    setShaders(m_programCombined, "combineTexture");
    checkProgram(m_programCombined);

    setShaders(m_programBlurred, "blur");
    checkProgram(m_programBlurred);

//    setShaders(m_programFXAA, "fxaa");
    setShaders(m_programFXAA, "FXAA");
//    setShaders(m_programFXAA, "FXAA3.11");
    checkProgram(m_programFXAA);

    m_vertices.append(QVector3D(-1.0, 1.0, 0.0));
    m_texCoords.append(QVector2D(0.0, 1.0));
    m_vertices.append(QVector3D(-1.0, -1.0, 0.0));
    m_texCoords.append(QVector2D(0.0, 0.0));
    m_vertices.append(QVector3D(1.0, 1.0, 0.0));
    m_texCoords.append(QVector2D(1.0, 1.0));
    m_vertices.append(QVector3D(1.0, -1.0, 0.0));
    m_texCoords.append(QVector2D(1.0, 0.0));
    m_vertices.append(QVector3D(-1.0, -1.0, 0.0));
    m_texCoords.append(QVector2D(0.0, 0.0));

    m_program.bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
    m_program.bindAttributeLocation("texCoord", PROGRAM_TEXTURE_ATTRIBUTE);
    m_programCombined.bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
    m_programCombined.bindAttributeLocation("texCoord", PROGRAM_TEXTURE_ATTRIBUTE);
    m_programBlurred.bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
    m_programBlurred.bindAttributeLocation("texCoord", PROGRAM_TEXTURE_ATTRIBUTE);
    m_programFXAA.bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
    m_programFXAA.bindAttributeLocation("texCoord", PROGRAM_TEXTURE_ATTRIBUTE);

    // Textures
    m_programCombined.bind();
    m_programCombined.setUniformValue("texture0", 0);
    m_programCombined.setUniformValue("texture1", 1);
    m_programCombined.release();
    m_programBlurred.bind();
    m_programBlurred.setUniformValue("texture0", 0);
    m_programBlurred.release();
    m_programFXAA.bind();
    m_programFXAA.setUniformValue("texture0", 0);
    m_programFXAA.release();

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
    m_texcoordBuffer.allocate(m_texCoords.constData(), m_texCoords.size() * sizeof(QVector2D));
    m_texcoordBuffer.release();
    m_texCoords.clear();

    createVAO();
}

ScreenQuad::~ScreenQuad()
{
    m_vertexBuffer.destroy();
    m_texcoordBuffer.destroy();
}

void ScreenQuad::createVAO()
{
    m_vao.create();
    m_vao.bind();

    m_program.enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_program.enableAttributeArray(PROGRAM_TEXTURE_ATTRIBUTE);
    m_programCombined.enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_programCombined.enableAttributeArray(PROGRAM_TEXTURE_ATTRIBUTE);
    m_programBlurred.enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_programBlurred.enableAttributeArray(PROGRAM_TEXTURE_ATTRIBUTE);
    m_programFXAA.enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_programFXAA.enableAttributeArray(PROGRAM_TEXTURE_ATTRIBUTE);

    m_vertexBuffer.bind();
    m_program.setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3);
    m_programCombined.setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3);
    m_programBlurred.setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3);
    m_programFXAA.setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3);
    m_vertexBuffer.release();

    m_texcoordBuffer.bind();
    m_program.setAttributeBuffer(PROGRAM_TEXTURE_ATTRIBUTE, GL_FLOAT, 0, 2);
    m_programCombined.setAttributeBuffer(PROGRAM_TEXTURE_ATTRIBUTE, GL_FLOAT, 0, 2);
    m_programBlurred.setAttributeBuffer(PROGRAM_TEXTURE_ATTRIBUTE, GL_FLOAT, 0, 2);
    m_programFXAA.setAttributeBuffer(PROGRAM_TEXTURE_ATTRIBUTE, GL_FLOAT, 0, 2);
    m_texcoordBuffer.release();

    m_vao.release();

    m_program.disableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_program.disableAttributeArray(PROGRAM_TEXTURE_ATTRIBUTE);
    m_programCombined.disableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_programCombined.disableAttributeArray(PROGRAM_TEXTURE_ATTRIBUTE);
    m_programBlurred.disableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_programBlurred.disableAttributeArray(PROGRAM_TEXTURE_ATTRIBUTE);
    m_programFXAA.disableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_programFXAA.disableAttributeArray(PROGRAM_TEXTURE_ATTRIBUTE);
}

void ScreenQuad::render(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection)
{
    m_program.bind();
    setUniformMatrix(m_program.uniformLocation("modelViewMatrix"), view*model);
    setUniformMatrix(m_program.uniformLocation("projectionMatrix"), projection);

    m_vao.bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, m_dataSize);
    m_vao.release();

    m_program.release();
}

void ScreenQuad::setResolution(int width, int height)
{
    m_width = width;
    m_height = height;
}

void ScreenQuad::setBlurResolution(int width, int height)
{
    m_programBlurred.bind();
    m_programBlurred.setUniformValue("resolution", QVector2D(width, height));
    m_programBlurred.release();
}

void ScreenQuad::renderBlurred(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection, Blur blurType)
{
    m_programBlurred.bind();
    m_programBlurred.setUniformValue("blurType", blurType);
    setUniformMatrix(m_programBlurred.uniformLocation("modelViewMatrix"), view*model);
    setUniformMatrix(m_programBlurred.uniformLocation("projectionMatrix"), projection);

    m_vao.bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, m_dataSize);
    m_vao.release();

    m_programBlurred.release();
}

void ScreenQuad::renderCombinedTextures(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection)
{
    m_programCombined.bind();
    setUniformMatrix(m_programCombined.uniformLocation("modelViewMatrix"), view*model);
    setUniformMatrix(m_programCombined.uniformLocation("projectionMatrix"), projection);

    m_vao.bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, m_dataSize);
    m_vao.release();

    m_programCombined.release();
}

void ScreenQuad::renderFXAA(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection)
{
    m_programFXAA.bind();
    setUniformMatrix(m_programFXAA.uniformLocation("modelViewMatrix"), view*model);
    setUniformMatrix(m_programFXAA.uniformLocation("projectionMatrix"), projection);
//    m_programFXAA.setUniformValue("step", QVector2D(1.0/(float)m_width, 1.0/(float)m_height));
    m_programFXAA.setUniformValue("resolution", QVector2D(m_width, m_height));

    m_vao.bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, m_dataSize);
    m_vao.release();

    m_programFXAA.release();
}
