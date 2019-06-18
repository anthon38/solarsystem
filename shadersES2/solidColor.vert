attribute vec4 vertex;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

uniform float logZbufferC;
uniform float C;

void main()
{
    gl_Position = projectionMatrix * modelViewMatrix * vertex;

//    gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * logZbufferC - 1.0;
    gl_Position.z = log(gl_Position.w*C + 1.0) * logZbufferC - 1.0;
    gl_Position.z *= gl_Position.w;
}
