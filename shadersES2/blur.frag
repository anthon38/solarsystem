varying highp vec2 textureCoord;

uniform sampler2D texture2d;
uniform int blurType;
uniform highp vec2 resolution;

const int gaussSize = 9;
const highp float sigma2 = 4.0;
const highp float strength = 0.2;

highp float gaussianFilter(highp float x, highp float s2)
{
    //1D
    return 1.0/sqrt(2.0*3.1415926535*s2)*exp((-x*x)/(2.0*s2));
}

void horizontalBlur()
{
    highp float pixelSize = 1.0/resolution.x;
    highp float blurStrength = 1.0-strength;

    gl_FragColor = vec4(0.0);
    for (int i=0; i<gaussSize; ++i) {
        highp float offset = float(-(gaussSize-1)/2+i);
        gl_FragColor += texture2D(texture2d, textureCoord+vec2(offset*pixelSize, 0.0)) * gaussianFilter(offset*blurStrength, sigma2);
    }
}

void verticalBlur()
{
    highp float pixelSize = 1.0/resolution.y;
    highp float blurStrength = 1.0-strength;

    gl_FragColor = vec4(0.0);
    for (int i=0; i<gaussSize; ++i) {
        highp float offset = float(-(gaussSize-1)/2+i);
        gl_FragColor += texture2D(texture2d, textureCoord+vec2(0.0, offset*pixelSize)) * gaussianFilter(offset*blurStrength, sigma2);
    }
}

void main()
{
    if (blurType == 1 ) {
        horizontalBlur();
    } else {
        verticalBlur();
    }
}
