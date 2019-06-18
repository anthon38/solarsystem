#include "flare.h"
#include "path.h"

Flare::Flare(QObject *parent)
    : Renderable(parent)
    , m_vertexBuffer(QOpenGLBuffer::VertexBuffer)
    , m_texcoordBuffer(QOpenGLBuffer::VertexBuffer)
{
    setShaders(m_program, "flare");
    checkProgram(m_program);

    QImage img(resPath()+"textures/flare/256flare4.png");
    if (img.isNull()) {
        qWarning("Flare: Unable to load file, unsupported file format");
    }

    m_texture = new QOpenGLTexture(img.mirrored());
    m_texture->setWrapMode(QOpenGLTexture::Repeat);
    m_texture->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::Linear);

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

    createVAO();m_useVao = false;
}

Flare::~Flare()
{
    m_vertexBuffer.destroy();
    m_texcoordBuffer.destroy();
    delete m_texture;
}

void Flare::cleanup()
{
    qDebug()<<"cleanup flare";
    m_texture->destroy();
}

void Flare::createVAO()
{
    m_vao.create();
    m_vao.bind();

    m_program.enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_program.enableAttributeArray(PROGRAM_TEXTURE_ATTRIBUTE);

    m_vertexBuffer.bind();
    m_program.setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3);
    m_vertexBuffer.release();

    m_texcoordBuffer.bind();
    m_program.setAttributeBuffer(PROGRAM_TEXTURE_ATTRIBUTE, GL_FLOAT, 0, 2);
    m_texcoordBuffer.release();

    m_vao.release();

    m_program.disableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_program.disableAttributeArray(PROGRAM_TEXTURE_ATTRIBUTE);
}

void Flare::render(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection)
{
    glBindTexture(GL_TEXTURE_2D, m_texture->textureId());
    m_program.bind();
    setUniformMatrix(m_program.uniformLocation("modelViewMatrix"), view*model);
    setUniformMatrix(m_program.uniformLocation("projectionMatrix"), projection);
    m_program.setUniformValue("logZbufferC", m_logZbufferC);
    m_program.setUniformValue("C", m_C);
    m_program.setUniformValue("alpha", m_alpha);
    m_program.setUniformValue("size", QSizeF(m_texture->width()/1280.0,
                                             m_texture->height()/800.0));

    glBlendFunc (GL_ONE, GL_ONE);
    m_vao.bind();
    glEnable(GL_BLEND);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, m_dataSize);
    glDisable(GL_BLEND);
    m_vao.release();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_program.release();
}
