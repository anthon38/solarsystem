#include "textbillboard.h"

#include <QGuiApplication>
#include <QOpenGLPaintDevice>
#include <QPainter>

QSizeF TextBillboard::Resolution(1.0, 1.0);

TextBillboard::TextBillboard(const QString &text, const QVector3D &color, QObject *parent)
    : Pickable(parent)
    , m_vertexBuffer(QOpenGLBuffer::VertexBuffer)
    , m_texcoordBuffer(QOpenGLBuffer::VertexBuffer)
{
    setShaders(m_program, "textBillboard");
    checkProgram(m_program);

    setShaders(m_programColor, "billboardSolidColor");
    checkProgram(m_programColor);

    QColor qcolor(color.x()*255, color.y()*255, color.z()*255);
    QOpenGLFramebufferObjectFormat fboFormat;
    fboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    QFont font(QGuiApplication::font());
    font.setCapitalization(QFont::Capitalize);
    font.setPointSize(13);
    QFontMetrics metric(font);
    const QRect drawRect(0, 0, metric.width(text), metric.height());
    m_fbo = new QOpenGLFramebufferObject(drawRect.size(), fboFormat);
    m_fbo->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    QOpenGLPaintDevice device(drawRect.size());
    QPainter painter;
    painter.begin(&device);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing);
    painter.fillRect(drawRect, Qt::transparent);
    painter.setPen(QPen(qcolor, 0));
    painter.setFont(font);
    painter.drawText(drawRect, text);
    painter.end();
    m_fbo->release();

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
    m_texcoordBuffer.allocate(m_texCoords.constData(), m_texCoords.size() * sizeof(QVector2D));
    m_texcoordBuffer.release();
    m_texCoords.clear();

    createVAO();
}

TextBillboard::~TextBillboard()
{
    m_vertexBuffer.destroy();
    m_texcoordBuffer.destroy();
    delete m_fbo;
}

void TextBillboard::createVAO()
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
    m_program.setAttributeBuffer(PROGRAM_TEXTURE_ATTRIBUTE, GL_FLOAT, 0, 2);
    m_texcoordBuffer.release();

    m_vao.release();

    m_program.disableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_program.disableAttributeArray(PROGRAM_TEXTURE_ATTRIBUTE);
    m_programColor.disableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
}

void TextBillboard::render(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection)
{
    glBindTexture(GL_TEXTURE_2D, m_fbo->texture());
    m_program.bind();
    setUniformMatrix(m_program.uniformLocation("modelViewMatrix"), view*model);
    setUniformMatrix(m_program.uniformLocation("projectionMatrix"), projection);
    m_program.setUniformValue("logZbufferC", m_logZbufferC);
    m_program.setUniformValue("C", m_C);
    m_program.setUniformValue("alpha", m_alpha);
    m_program.setUniformValue("size", QSizeF(m_fbo->size().width()/Resolution.width(),
                                             m_fbo->size().height()/Resolution.height()));

    m_vao.bind();
    glEnable(GL_BLEND);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, m_dataSize);
    glDisable(GL_BLEND);
    m_vao.release();

    m_program.release();
}

void TextBillboard::renderSolidColor(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection)
{
    m_programColor.bind();

    m_programColor.setUniformValue("color", m_color);
    setUniformMatrix(m_programColor.uniformLocation("modelViewMatrix"), view*model);
    setUniformMatrix(m_programColor.uniformLocation("projectionMatrix"), projection);
    m_programColor.setUniformValue("logZbufferC", m_logZbufferC);
    m_programColor.setUniformValue("C", m_C);
    m_programColor.setUniformValue("size", QSizeF(m_fbo->size().width()/Resolution.width(),
                                                  m_fbo->size().height()/Resolution.height()));

    m_vao.bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, m_dataSize);
    m_vao.release();

    m_programColor.release();
}

unsigned int TextBillboard::nearestPowerOfTwo(unsigned int n) const
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}
