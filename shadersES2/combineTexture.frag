varying highp vec2 textureCoord;

uniform sampler2D texture0;
uniform sampler2D texture1;

void main(void)
{
    gl_FragColor = texture2D(texture0, textureCoord) +  texture2D(texture1, textureCoord);
}
