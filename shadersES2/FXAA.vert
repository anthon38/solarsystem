attribute vec4 vertex;
attribute vec2 texCoord;

varying vec2 textureCoord;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    // connect first texture coordinates of the vertex
    textureCoord = texCoord;
    gl_Position = projectionMatrix * modelViewMatrix * vertex;
}
