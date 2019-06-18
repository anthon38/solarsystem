attribute vec4 vertex;
attribute vec4 normal;
attribute vec2 texCoord;

varying vec2 textureCoord;
varying float lambertTerm;

struct LightInfo {
    vec4 Position; // Light position in eye coords.
    vec4 La; // Ambient light intensity
    vec4 Ld; // Diffuse light intensity
    vec4 Ls; // Specular light intensity
};
uniform LightInfo light;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

uniform float logZbufferC;
uniform float C;

void main()
{
    // transform the vertex position and its normal into view (or eye) space
    vec3 pv = vec3(modelViewMatrix * vertex);
    vec3 nv = normalMatrix * vec3(normal);

    // light vector (from vertex to light) in view space
    vec3 lv = light.Position.xyz - pv;

    vec3 Nv = normalize(nv);
    vec3 Lv = normalize(lv);

    lambertTerm = max(dot(Nv,Lv), 0.0);

    // connect first texture coordinates of the vertex
    textureCoord = texCoord;
    gl_Position = projectionMatrix * modelViewMatrix * vertex;

//    gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * logZbufferC - 1.0;
    gl_Position.z = log(gl_Position.w*C + 1.0) * logZbufferC - 1.0;
    gl_Position.z *= gl_Position.w;
}
