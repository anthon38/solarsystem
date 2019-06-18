attribute vec4 vertex;
attribute vec2 texCoord;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

uniform vec2 size;

uniform float logZbufferC;
uniform float C;

void main()
{
    gl_Position = projectionMatrix * modelViewMatrix * vec4(0.0, 0.0, 0.0, 1.0);

//    gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * logZbufferC - 1.0;
    gl_Position.z = log(gl_Position.w*C + 1.0) * logZbufferC - 1.0;
    gl_Position.z *= gl_Position.w;

    gl_Position /= gl_Position.w;
    gl_Position.xy += (vertex.xy+vec2(1.2)) * size;
}
