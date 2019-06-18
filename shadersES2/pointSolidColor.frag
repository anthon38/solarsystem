uniform highp vec3 color;

void main(void)
{
    highp float radius = distance(vec2(0.5, 0.5), gl_PointCoord);
    if (radius > 0.5)
        discard;
    gl_FragColor = vec4(color, 1.0);
}
