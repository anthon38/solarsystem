attribute vec3 vertexHigh;
attribute vec3 vertexLow;
attribute vec4 color;

varying vec4 sColor;
varying float zClip;

uniform mat4 modelViewMatrixRTE;
uniform mat4 projectionMatrix;

uniform vec3 cameraPosHigh;
uniform vec3 cameraPosLow;

uniform float logZbufferC;
uniform float C;

void main()
{
    sColor = color;

    //
    // Emulated double precision subtraction ported from dssub() in DSFUN90.
    // http://crd.lbl.gov/~dhbailey/mpdist/
    //
    vec3 t1 = vertexLow - cameraPosLow;
    vec3 e = t1 - vertexLow;
    vec3 t2 = ((-cameraPosLow - e) + (vertexLow - (t1 - e))) + vertexHigh - cameraPosHigh;
    vec3 highDifference = t1 + t2;
    vec3 lowDifference = t2 - (highDifference - t1);
    gl_Position = projectionMatrix * modelViewMatrixRTE * vec4(highDifference + lowDifference, 1.0);

    zClip = gl_Position.z;
    //MARCHE PAS!!! faut tester l'extension
#ifdef GL_ES
    //    gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * logZbufferC - 1.0;
        gl_Position.z = log(gl_Position.w*C + 1.0) * logZbufferC - 1.0;
        gl_Position.z *= gl_Position.w;
#endif
}
