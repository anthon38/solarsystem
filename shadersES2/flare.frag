varying highp vec2 textureCoord;

uniform sampler2D texture2d;

uniform float alpha;

void main()
{
    vec4 color = texture2D(texture2d, textureCoord);
    if (color.rgb == vec3(0.0))
        discard;
//    color.a = color.a*alpha;
//    color.a = alpha;
    gl_FragColor = color;
}
