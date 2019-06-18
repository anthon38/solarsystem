attribute vec4 vertex;
attribute vec4 color;

varying vec3 sColor;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

uniform float pointSizeCoeff;

void main()
{
    sColor = color.rgb;
    float magnitude = color.a;
    if (magnitude > 6.5)
        magnitude = 6.5;
    gl_PointSize = (8.5-magnitude)*pointSizeCoeff;
    gl_Position = projectionMatrix * modelViewMatrix * vertex;
}
