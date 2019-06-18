varying highp vec2 textureCoord;

uniform sampler2D texture2d;

void main()
{
    gl_FragColor = texture2D(texture2d, textureCoord);
}
