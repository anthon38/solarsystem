#include "sphere.h"

#include <cmath>

Sphere::Sphere(float radius, float flattening, QObject *parent)
    : Pickable(parent)
    , m_indexBuffer(QOpenGLBuffer::IndexBuffer)
    , m_vertexBuffer(QOpenGLBuffer::VertexBuffer)
    , m_normalBuffer(QOpenGLBuffer::VertexBuffer)
    , m_texcoordBuffer(QOpenGLBuffer::VertexBuffer)
{
    setShaders(m_program, "body");
    checkProgram(m_program);

    setShaders(m_programColor, "solidColor");
    checkProgram(m_programColor);

    const int nSub = 80;
    int indices[nSub+1][nSub+1];
    int count = 0;
    for( int i = 0; i <= nSub; ++i ) {
        float theta = (float)i/(float)nSub*2.0*M_PI;
        for( int j = 0; j <= nSub; ++j ) {
            float phi = (float)j/(float)nSub*M_PI;
            indices[i][j] = count++;
            QVector3D v = QVector3D(cos(theta)*sin(phi),
                                    sin(theta)*sin(phi),
                                    cos(phi));
            m_vertices.append(radius*QVector3D(1.0, 1.0, (1.0-flattening))*v); // Spheroid
            m_normals.append((QVector3D((1.0-flattening), (1.0-flattening), 1.0)*v).normalized());
            QVector2D uv = QVector2D((float)i/(float)nSub,
                                     1.0-(float)j/(float)nSub);
            m_texCoords.append(uv);
        }
    }

    for( int i = 0; i < nSub; ++i ) {
        for( int j = 0; j <= nSub; ++j ) {
            m_indices.append(indices[(i+1)][j]);
            m_indices.append(indices[i][j]);
        }
    }

    m_program.bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
    m_program.bindAttributeLocation("normal", PROGRAM_NORMAL_ATTRIBUTE);
    m_program.bindAttributeLocation("texCoord", PROGRAM_TEXTURE_ATTRIBUTE);
    m_programColor.bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);

    m_program.bind();
    // Textures
    m_program.setUniformValue("texture0", 0);
    m_program.setUniformValue("texture1", 1);

    // Light settings
    m_program.setUniformValue("light.Ld", QVector4D(1.0, 1.0, 1.0, 1.0));
    m_program.release();

    m_dataSize = m_indices.size();
    // Sphere indices buffer init
    m_indexBuffer.create();
    m_indexBuffer.bind();
    m_indexBuffer.allocate(m_indices.constData(), m_indices.size() * sizeof(int));
    m_indexBuffer.release();
    m_indices.clear();
    // Sphere vertex buffer init
    m_vertexBuffer.create();
    m_vertexBuffer.bind();
    m_vertexBuffer.allocate(m_vertices.constData(), m_vertices.size() * sizeof(QVector3D));
    m_vertexBuffer.release();
    m_vertices.clear();
    // Sphere normal buffer init
    m_normalBuffer.create();
    m_normalBuffer.bind();
    m_normalBuffer.allocate(m_normals.constData(), m_normals.size() * sizeof(QVector3D));
    m_normalBuffer.release();
    m_normals.clear();
    // Sphere texCoord buffer init
    m_texcoordBuffer.create();
    m_texcoordBuffer.bind();
    m_texcoordBuffer.allocate(m_texCoords.constData(), m_texCoords.size() * sizeof(QVector2D));
    m_texcoordBuffer.release();
    m_texCoords.clear();

    createVAO();
}

Sphere::~Sphere()
{
    m_indexBuffer.destroy();
    m_vertexBuffer.destroy();
    m_normalBuffer.destroy();
    m_texcoordBuffer.destroy();
}

void Sphere::createVAO()
{
    m_vao.create();
    m_vao.bind();

    m_program.enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_program.enableAttributeArray(PROGRAM_NORMAL_ATTRIBUTE);
    m_program.enableAttributeArray(PROGRAM_TEXTURE_ATTRIBUTE);
    m_programColor.enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);

    m_vertexBuffer.bind();
    m_program.setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3);
    m_programColor.setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3);
    m_vertexBuffer.release();

    m_normalBuffer.bind();
    m_program.setAttributeBuffer(PROGRAM_NORMAL_ATTRIBUTE, GL_FLOAT, 0, 3);
    m_normalBuffer.release();

    m_texcoordBuffer.bind();
    m_program.setAttributeBuffer(PROGRAM_TEXTURE_ATTRIBUTE, GL_FLOAT, 0, 2);
    m_texcoordBuffer.release();

    m_indexBuffer.bind();

    m_vao.release();

    m_indexBuffer.release();

    m_program.disableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_program.disableAttributeArray(PROGRAM_NORMAL_ATTRIBUTE);
    m_program.disableAttributeArray(PROGRAM_TEXTURE_ATTRIBUTE);
    m_programColor.disableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
}

void Sphere::render(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection)
{
    m_program.bind();

    setUniformMatrix(m_program.uniformLocation("modelViewMatrix"), view*model);
    setUniformMatrix(m_program.uniformLocation("projectionMatrix"), projection);
    Eigen::Matrix3d normal = (view*model).matrix().topLeftCorner<3,3>().transpose().inverse();
    setUniformMatrix(m_program.uniformLocation("normalMatrix"), normal);
    setUniformVector(m_program.uniformLocation("light.Position"), view*Eigen::Vector4d(0.0, 0.0, 0.0, 1.0));
    m_program.setUniformValue("logZbufferC", m_logZbufferC);
    m_program.setUniformValue("C", m_C);

    m_vao.bind();
    glEnable(GL_CULL_FACE);
    glDrawElements(GL_TRIANGLE_STRIP, m_dataSize, GL_UNSIGNED_INT, 0);
    glDisable(GL_CULL_FACE);
    m_vao.release();

    m_program.release();
}

void Sphere::renderSolidColor(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection)
{
    m_programColor.bind();

    m_programColor.setUniformValue("color", m_color);
    setUniformMatrix(m_programColor.uniformLocation("modelViewMatrix"), view*model);
    setUniformMatrix(m_programColor.uniformLocation("projectionMatrix"), projection);
    m_programColor.setUniformValue("logZbufferC", m_logZbufferC);
    m_programColor.setUniformValue("C", m_C);

    m_vao.bind();
    glEnable(GL_CULL_FACE);
    glDrawElements(GL_TRIANGLE_STRIP, m_dataSize, GL_UNSIGNED_INT, 0);
    glDisable(GL_CULL_FACE);
    m_vao.release();

    m_programColor.release();
}
