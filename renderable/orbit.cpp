#include "orbit.h"

#include <cmath>

Orbit::Orbit(const OrbitalElements &elements, const QVector3D &color, QObject *parent)
    : Renderable(parent)
    , m_elements(elements)
    , m_vertexHighBuffer(QOpenGLBuffer::VertexBuffer)
    , m_vertexLowBuffer(QOpenGLBuffer::VertexBuffer)
    , m_colorBuffer(QOpenGLBuffer::VertexBuffer)
    , m_color(color)
{
    setShaders(m_program, "orbit");
    checkProgram(m_program);

    Eigen::Vector2d pos;
    QVector2D xpos, ypos; // For these, .x() represents the high component of a double and .y() the low component.
    // Set a dummy vertex at t = 0
    m_lastIndex = 0;
    pos = position(0);
    xpos = doubleToTwoFloats(pos.x());
    ypos = doubleToTwoFloats(pos.y());
    m_verticesHigh.append(QVector3D(xpos.x(), ypos.x(), 0.0));
    m_verticesLow.append(QVector3D(xpos.y(), ypos.y(), 0.0));
    m_colors.append(color);
    // We double the vertex. See why in Orbit::setBodyPosition
    m_verticesHigh.append(QVector3D(xpos.x(), ypos.x(), 0.0));
    m_verticesLow.append(QVector3D(xpos.y(), ypos.y(), 0.0));
    m_colors.append(color);

    double t;
    const int steps = 360;
    for (int theta=0; theta<steps; ++theta) {
        t = theta/(double)steps*m_elements.revolutionPeriod;
        m_timeTable << t;
        pos = position(t);
        xpos = doubleToTwoFloats(pos.x());
        ypos = doubleToTwoFloats(pos.y());
        m_verticesHigh.append(QVector3D(xpos.x(), ypos.x(), 0.0));
        m_verticesLow.append(QVector3D(xpos.y(), ypos.y(), 0.0));
        m_colors.append(color);
//        m_colors.append((theta%2)?QVector3D(1, 0, 0):QVector3D(0, 0, 1));
    }
    double angle = 0.0;
    t = (angle-m_elements.meanAnomalyAtEpoch*M_PI/180.0)/(2*M_PI/m_elements.revolutionPeriod);
    pos = position(t);
    xpos = doubleToTwoFloats(pos.x());
    ypos = doubleToTwoFloats(pos.y());
    m_verticesHigh.append(QVector3D(xpos.x(), ypos.x(), 0.0));
    m_verticesLow.append(QVector3D(xpos.y(), ypos.y(), 0.0));
    m_colors.append(QVector3D(1.0, 0.0, 0.0));
    angle = M_PI;
    t = (angle-m_elements.meanAnomalyAtEpoch*M_PI/180.0)/(2*M_PI/m_elements.revolutionPeriod);
    pos = position(t);
    xpos = doubleToTwoFloats(pos.x());
    ypos = doubleToTwoFloats(pos.y());
    m_verticesHigh.append(QVector3D(xpos.x(), ypos.x(), 0.0));
    m_verticesLow.append(QVector3D(xpos.y(), ypos.y(), 0.0));
    m_colors.append(QVector3D(0.0, 0.0, 1.0));

    m_orientation.setIdentity();
    m_orientation.rotate(Eigen::AngleAxisd(m_elements.longitudeOfAscendingNode, Eigen::Vector3d::UnitZ()));
    m_orientation.rotate(Eigen::AngleAxisd(m_elements.inclination, Eigen::Vector3d::UnitX()));
    m_orientation.rotate(Eigen::AngleAxisd(m_elements.argumentOfPeriapsis, Eigen::Vector3d::UnitZ()));

    m_program.bindAttributeLocation("vertexHigh", PROGRAM_VERTEX_HIGH_ATTRIBUTE);
    m_program.bindAttributeLocation("vertexLow", PROGRAM_VERTEX_LOW_ATTRIBUTE);
    m_program.bindAttributeLocation("color", PROGRAM_COLOR_ATTRIBUTE);

    m_dataSize = m_verticesHigh.size();
    // Orbit vertex high buffer init
    m_vertexHighBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_vertexHighBuffer.create();
    m_vertexHighBuffer.bind();
    m_vertexHighBuffer.allocate(m_verticesHigh.constData(), m_verticesHigh.size() * sizeof(QVector3D));
    m_vertexHighBuffer.release();
    // Orbit vertex low component buffer init
    m_vertexLowBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_vertexLowBuffer.create();
    m_vertexLowBuffer.bind();
    m_vertexLowBuffer.allocate(m_verticesLow.constData(), m_verticesLow.size() * sizeof(QVector3D));
    m_vertexLowBuffer.release();
    // Orbit color buffer init
    m_colorBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_colorBuffer.create();
    m_colorBuffer.bind();
    m_colorBuffer.allocate(m_colors.constData(), m_colors.size() * sizeof(QVector4D));
    m_colorBuffer.release();

    createVAO();
}

Orbit::~Orbit()
{
    m_vertexHighBuffer.destroy();
    m_vertexLowBuffer.destroy();
    m_colorBuffer.destroy();
}

Eigen::Vector2d Orbit::position(double time/*seconds past epoch*/) const
{
    // http://en.wikipedia.org/wiki/Mean_anomaly
    double meanAnomaly = m_elements.meanAnomalyAtEpoch + 2.0*M_PI/m_elements.revolutionPeriod * time;
    double E = eccentricAnomaly(m_elements.eccentricity, meanAnomaly, 0.001);
    double x = m_elements.semiMajorAxis*(cos(E)-m_elements.eccentricity);
    double y = m_elements.semiMajorAxis*sqrt(1.0-m_elements.eccentricity*m_elements.eccentricity)*sin(E);

    return Eigen::Vector2d(x, y);
}

double Orbit::eccentricAnomaly(double ecc, double M, double epsilon) const
{
    // http://www.jgiesen.de/kepler/kepler.html
    // http://www.stargazing.net/kepler/kepler.html
    // We need to solve the Kepler's equation E = M + ecc*sin(E)
    // Finds the eccentric anomaly with Newton-Raphson
    double delta = 0.05;
    // First guess
    double E = M;
    while ( std::abs(delta) >= epsilon ) {
        delta = E - ecc * sin(E) - M;
        E = E - delta / (1.0 - ecc * cos(E));
    }
    return E;
}

QVector2D Orbit::doubleToTwoFloats(double value)
{
    float high = (float)value;
    float low = (float)(value-high);
    return QVector2D(high, low);
}

void Orbit::createVAO()
{
    m_vao.create();
    m_vao.bind();

    m_program.enableAttributeArray(PROGRAM_VERTEX_HIGH_ATTRIBUTE);
    m_program.enableAttributeArray(PROGRAM_VERTEX_LOW_ATTRIBUTE);
    m_program.enableAttributeArray(PROGRAM_COLOR_ATTRIBUTE);

    m_vertexHighBuffer.bind();
    m_program.setAttributeBuffer(PROGRAM_VERTEX_HIGH_ATTRIBUTE, GL_FLOAT, 0, 3);
    m_vertexHighBuffer.release();

    m_vertexLowBuffer.bind();
    m_program.setAttributeBuffer(PROGRAM_VERTEX_LOW_ATTRIBUTE, GL_FLOAT, 0, 3);
    m_vertexLowBuffer.release();

    m_colorBuffer.bind();
    m_program.setAttributeBuffer(PROGRAM_COLOR_ATTRIBUTE, GL_FLOAT, 0, 4);
    m_colorBuffer.release();

    m_vao.release();

    m_program.disableAttributeArray(PROGRAM_VERTEX_HIGH_ATTRIBUTE);
    m_program.disableAttributeArray(PROGRAM_VERTEX_LOW_ATTRIBUTE);
    m_program.disableAttributeArray(PROGRAM_COLOR_ATTRIBUTE);
}

void Orbit::render(const Eigen::Affine3d &model, const Eigen::Affine3d &view, const Eigen::Affine3d &projection)
{
    updateBuffers();

    m_program.bind();

    // To prevent jitter, we use the GPU RTE DSFUN90 method - 3D Engine Design for Virtual Globes chap5.4
    Eigen::Affine3d modelviewRTE = view*model;
    modelviewRTE.translation() = Eigen::Vector3d::Zero();
    setUniformMatrix(m_program.uniformLocation("modelViewMatrixRTE"), modelviewRTE);
    setUniformMatrix(m_program.uniformLocation("projectionMatrix"), projection);

    Eigen::Vector3d cameraPosition = (view*model).inverse().translation();
    QVector2D doubleX = doubleToTwoFloats(cameraPosition.x());
    QVector2D doubleY = doubleToTwoFloats(cameraPosition.y());
    QVector2D doubleZ = doubleToTwoFloats(cameraPosition.z());
    QVector3D cameraPosHigh(doubleX.x(), doubleY.x(), doubleZ.x());
    QVector3D cameraPosLow(doubleX.y(), doubleY.y(), doubleZ.y());
    m_program.setUniformValue("cameraPosHigh", cameraPosHigh);
    m_program.setUniformValue("cameraPosLow", cameraPosLow);

    m_program.setUniformValue("logZbufferC", m_logZbufferC);
    m_program.setUniformValue("C", m_C);
    m_program.setUniformValue("alpha", m_alpha);

    m_vao.bind();
    glEnable(GL_BLEND);
    glDrawArrays(GL_LINE_LOOP, 0, m_dataSize-2);
    glDisable(GL_BLEND);
//    glDrawArrays(GL_LINES, m_dataSize-2, 2);
    m_vao.release();

    m_program.release();
}

void Orbit::updateBuffers()
{
    // Orbit vertex high buffer
    m_vertexHighBuffer.bind();
    m_vertexHighBuffer.write(0, m_verticesHigh.constData(), m_verticesHigh.size() * sizeof(QVector3D));
    m_vertexHighBuffer.release();
    // Orbit vertex low component buffer
    m_vertexLowBuffer.bind();
    m_vertexLowBuffer.write(0, m_verticesLow.constData(), m_verticesLow.size() * sizeof(QVector3D));
    m_vertexLowBuffer.release();
    // Orbit color buffer
    m_colorBuffer.bind();
    m_colorBuffer.write(0, m_colors.constData(), m_colors.size() * sizeof(QVector4D));
    m_colorBuffer.release();
}


void Orbit::setBodyPosition(Eigen::Vector2d position, double time)
{
    // Insert a vertex defined by position based on its timestamp
    // We don't want the transparency at both ends of the loop to be interpolated,
    // so we actually insert (and remove) the vertex twice to prevent that.
    // Using the flat interpolator would be better but it is unavailable in gles 2.0.
    // Remove the old vertex
    m_verticesHigh.removeAt(m_lastIndex);
    m_verticesLow.removeAt(m_lastIndex);
    m_colors.removeAt(m_lastIndex);
    m_verticesHigh.removeAt(m_lastIndex);
    m_verticesLow.removeAt(m_lastIndex);
    m_colors.removeAt(m_lastIndex);
    // Find the position in the list to insert the vertex
    double t = fmod(time, m_elements.revolutionPeriod);
    int index = 0;
    while ( (index < m_timeTable.size()) && (m_timeTable.at(index) < t) ) {
        ++index;
    }
    m_lastIndex = index;
    // Insertions
    QVector2D xpos = doubleToTwoFloats(position.x());
    QVector2D ypos = doubleToTwoFloats(position.y());
    m_verticesHigh.insert(index, QVector3D(xpos.x(), ypos.x(), 0.0));
    m_verticesLow.insert(index, QVector3D(xpos.y(), ypos.y(), 0.0));
    m_colors.insert(index, m_color);
    m_verticesHigh.insert(index, QVector3D(xpos.x(), ypos.x(), 0.0));
    m_verticesLow.insert(index, QVector3D(xpos.y(), ypos.y(), 0.0));
    m_colors.insert(index, m_color);
    // Update transparency
    int n = m_timeTable.size()+2; // +2 because we added 2 vertices.
    for (int i = 0; i < n; ++i) {
        float alpha = (float)i/(float)n;
        m_colors[(i+index+1)%n].setW(alpha);
    }
}

