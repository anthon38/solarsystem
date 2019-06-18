varying highp vec2 textureCoord;

uniform sampler2D texture2d;

void main(void)
{
    gl_FragColor = vec4( texture2D(texture2d, textureCoord) );
}
