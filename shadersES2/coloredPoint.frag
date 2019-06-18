varying highp vec3 sColor;

uniform float alpha;

void main(void)
{
    highp float radius = distance(vec2(0.5, 0.5), gl_PointCoord);
    if (radius > 0.5)
        discard;
    highp float sigma2 = 0.02;
    highp float a = clamp(exp((-radius*radius+0.2)/(2.0*sigma2)), 0.0, 1.0);
    gl_FragColor = vec4(sColor, alpha*a);
}
