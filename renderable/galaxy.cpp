#include "galaxy.h"
#include "path.h"

#include <QFile>
#include <QTextStream>

Galaxy::Galaxy(QObject *parent)
    : Renderable(parent)
    , m_pointSizeCoeff(1.0)
    , m_vertexBuffer(QOpenGLBuffer::VertexBuffer)
    , m_colorBuffer(QOpenGLBuffer::VertexBuffer)
{
    setShaders(m_program, "galaxy");
    checkProgram(m_program);

    QFile file(resPath()+"data/hygxyz.csv");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning()<<"Error loading file "<<file.fileName();
    } else {
        QTextStream data(&file);
        //discard header
        data.readLine();
        //discard Sol
        data.readLine();
        while (!data.atEnd()) {
            QString line = data.readLine();
            QStringList properties = line.split(",");
            float magnitude = properties.at(13).toFloat();
            // Discard stars invisible to the naked eye
            if (magnitude > 7.0)
                continue;
            QVector3D position = QVector3D(properties.at(17).toFloat(),
                                           properties.at(18).toFloat(),
                                           properties.at(19).toFloat());
            double parsecToKm = 3.08567758e13;
            m_vertices.append(position*parsecToKm);

            QVector3D color;
//            QString spectrum = properties.at(15);
//            color = spectrumToRgb(spectrum);
            float colorIndex = properties.at(16).toFloat();
            color = colorIndexToRgb(colorIndex);
            color /= 255.0;
            m_colors.append(QVector4D(color.x(), color.y(), color.z(), magnitude));
        }
    }

    m_program.bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
    m_program.bindAttributeLocation("color", PROGRAM_COLOR_ATTRIBUTE);

    m_dataSize = m_vertices.size();
    // Galaxy vertex buffer init
    m_vertexBuffer.create();
    m_vertexBuffer.bind();
    m_vertexBuffer.allocate(m_vertices.constData(), m_vertices.size() * sizeof(QVector3D));
    m_vertexBuffer.release();
    m_vertices.clear();
    // Galaxy color buffer init
    m_colorBuffer.create();
    m_colorBuffer.bind();
    m_colorBuffer.allocate(m_colors.constData(), m_colors.size() * sizeof(QVector4D));
    m_colorBuffer.release();
    m_colors.clear();

    createVAO();
}

Galaxy::~Galaxy()
{
    m_vertexBuffer.destroy();
    m_colorBuffer.destroy();
}

void Galaxy::createVAO()
{
    m_vao.create();
    m_vao.bind();

    m_program.enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_program.enableAttributeArray(PROGRAM_COLOR_ATTRIBUTE);

    m_vertexBuffer.bind();
    m_program.setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3);
    m_vertexBuffer.release();

    m_colorBuffer.bind();
    m_program.setAttributeBuffer(PROGRAM_COLOR_ATTRIBUTE, GL_FLOAT, 0, 4);
    m_colorBuffer.release();

    m_vao.release();

    m_program.disableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_program.disableAttributeArray(PROGRAM_COLOR_ATTRIBUTE);
}

void Galaxy::render(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection)
{
    m_program.bind();

    setUniformMatrix(m_program.uniformLocation("modelViewMatrix"), view*model);
    setUniformMatrix(m_program.uniformLocation("projectionMatrix"), projection);
    m_program.setUniformValue("pointSizeCoeff", m_pointSizeCoeff);

    m_vao.bind();
    glEnable(GL_BLEND);
    glDrawArrays(GL_POINTS, 0, m_dataSize);
    glDisable(GL_BLEND);
    m_vao.release();

    m_program.release();
}

QVector3D Galaxy::spectrumToRgb(const QString &spectrum)
{
    QVector3D color;
    if (spectrum.startsWith("O")) {
        color = QVector3D(157.0, 180.0, 255.0);
    } else if (spectrum.startsWith("B")) {
        color = QVector3D(170.0, 191.0, 255.0);
    } else if (spectrum.startsWith("A")) {
        color = QVector3D(202.0, 216.0, 255.0);
    } else if (spectrum.startsWith("F")) {
        color = QVector3D(255.0, 255.0, 255.0);
    } else if (spectrum.startsWith("G")) {
        color = QVector3D(255.0, 244.0, 232.0);
    } else if (spectrum.startsWith("K")) {
        color = QVector3D(255.0, 221.0, 180.0);
    } else if (spectrum.startsWith("M")) {
        color = QVector3D(255.0, 189.0, 111.0);
    } else if (spectrum.startsWith("L")) {
        color = QVector3D(210.0, 0.0, 51.0);
    } else if (spectrum.startsWith("T")) {
        color = QVector3D(204.0, 0.0, 153.0);
    } else if (spectrum.startsWith("Y")) {
        color = QVector3D(153.0, 102.0, 51.0);
    } else {
        color = QVector3D(255.0, 255.0, 255.0);
    }
    return color;
}

QVector3D Galaxy::colorIndexToRgb(qreal colorIndex)
{
    QVector3D color;
    if (colorIndex < -0.40) {
        color = QVector3D(155.0, 178.0, 255.0);
    } else if (colorIndex < -0.35) {
        color = QVector3D(158.0, 181.0, 255.0);
    } else if (colorIndex < -0.30) {
        color = QVector3D(163.0, 185.0, 255.0);
    } else if (colorIndex < -0.25) {
        color = QVector3D(170.0, 191.0, 255.0);
    } else if (colorIndex < -0.20) {
        color = QVector3D(178.0, 197.0, 255.0);
    } else if (colorIndex < -0.15) {
        color = QVector3D(187.0, 204.0, 255.0);
    } else if (colorIndex < -0.10) {
        color = QVector3D(196.0, 210.0, 255.0);
    } else if (colorIndex < -0.05) {
        color = QVector3D(204.0, 216.0, 255.0);
    } else if (colorIndex < 0.0) {
        color = QVector3D(211.0, 221.0, 255.0);
    } else if (colorIndex < 0.05) {
        color = QVector3D(218.0, 226.0, 255.0);
    } else if (colorIndex < 0.10) {
        color = QVector3D(223.0, 229.0, 255.0);
    } else if (colorIndex < 0.15) {
        color = QVector3D(228.0, 233.0, 255.0);
    } else if (colorIndex < 0.20) {
        color = QVector3D(233.0, 236.0, 255.0);
    } else if (colorIndex < 0.25) {
        color = QVector3D(238.0, 239.0, 255.0);
    } else if (colorIndex < 0.30) {
        color = QVector3D(243.0, 242.0, 255.0);
    } else if (colorIndex < 0.35) {
        color = QVector3D(248.0, 246.0, 255.0);
    } else if (colorIndex < 0.40) {
        color = QVector3D(254.0, 249.0, 255.0);
    } else if (colorIndex < 0.45) {
        color = QVector3D(255.0, 249.0, 251.0);
    } else if (colorIndex < 0.50) {
        color = QVector3D(255.0, 247.0, 245.0);
    } else if (colorIndex < 0.55) {
        color = QVector3D(255.0, 245.0, 239.0);
    } else if (colorIndex < 0.60) {
        color = QVector3D(255.0, 243.0, 234.0);
    } else if (colorIndex < 0.65) {
        color = QVector3D(255.0, 241.0, 229.0);
    } else if (colorIndex < 0.70) {
        color = QVector3D(255.0, 239.0, 224.0);
    } else if (colorIndex < 0.75) {
        color = QVector3D(255.0, 237.0, 219.0);
    } else if (colorIndex < 0.80) {
        color = QVector3D(255.0, 235.0, 214.0);
    } else if (colorIndex < 0.85) {
        color = QVector3D(255.0, 233.0, 210.0);
    } else if (colorIndex < 0.90) {
        color = QVector3D(255.0, 232.0, 206.0);
    } else if (colorIndex < 0.95) {
        color = QVector3D(255.0, 230.0, 202.0);
    } else if (colorIndex < 1.00) {
        color = QVector3D(255.0, 229.0, 198.0);
    } else if (colorIndex < 1.05) {
        color = QVector3D(255.0, 227.0, 195.0);
    } else if (colorIndex < 1.10) {
        color = QVector3D(255.0, 226.0, 191.0);
    } else if (colorIndex < 1.15) {
        color = QVector3D(255.0, 224.0, 187.0);
    } else if (colorIndex < 1.20) {
        color = QVector3D(255.0, 223.0, 184.0);
    } else if (colorIndex < 1.25) {
        color = QVector3D(255.0, 221.0, 180.0);
    } else if (colorIndex < 1.30) {
        color = QVector3D(255.0, 219.0, 176.0);
    } else if (colorIndex < 1.35) {
        color = QVector3D(255.0, 218.0, 173.0);
    } else if (colorIndex < 1.40) {
        color = QVector3D(255.0, 216.0, 169.0);
    } else if (colorIndex < 1.45) {
        color = QVector3D(255.0, 214.0, 165.0);
    } else if (colorIndex < 1.50) {
        color = QVector3D(255.0, 213.0, 161.0);
    } else if (colorIndex < 1.55) {
        color = QVector3D(255.0, 210.0, 156.0);
    } else if (colorIndex < 1.60) {
        color = QVector3D(255.0, 208.0, 150.0);
    } else if (colorIndex < 1.65) {
        color = QVector3D(255.0, 204.0, 143.0);
    } else if (colorIndex < 1.70) {
        color = QVector3D(255.0, 200.0, 133.0);
    } else if (colorIndex < 1.75) {
        color = QVector3D(255.0, 193.0, 120.0);
    } else if (colorIndex < 1.80) {
        color = QVector3D(255.0, 183.0, 101.0);
    } else if (colorIndex < 1.85) {
        color = QVector3D(255.0, 169.0, 75.0);
    } else if (colorIndex < 1.90) {
        color = QVector3D(255.0, 149.0, 35.0);
    } else if (colorIndex < 1.95) {
        color = QVector3D(255.0, 123.0, 0.0);
    } else {
        color = QVector3D(255.0, 82.0, 0.0);
    }
    return color;
}
