#ifdef GL_EXT_frag_depth
#extension GL_EXT_frag_depth: enable
#endif

varying highp vec4 sColor;
varying highp float zClip;

uniform float alpha;

void main(void)
{
    if (sColor.a < 0.01)
        discard;
    gl_FragColor = vec4(sColor.rgb, sColor.a*alpha);

#ifndef GL_ES
    const highp float C = 1.0;
    const highp float far = 1000.0*5874000.000;
    gl_FragDepth = log(zClip*C + 1.0)/log(far*C + 1.0);
#elif GL_EXT_frag_depth
    const highp float C = 1.0;
    const highp float far = 1000.0*5874000.000;
    gl_FragDepthEXT = log(zClip*C + 1.0)/log(far*C + 1.0);
#endif
}
