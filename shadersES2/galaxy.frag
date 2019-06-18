varying highp vec3 sColor;

void main(void)
{
    highp float radius = distance(vec2(0.5, 0.5), gl_PointCoord);
//    gl_FragColor = vec4(sColor, 2.0*(0.5-radius));
    highp float sigma2 = 0.04;
    gl_FragColor = vec4(sColor,  exp((-radius*radius)/(2.0*sigma2)));
}
