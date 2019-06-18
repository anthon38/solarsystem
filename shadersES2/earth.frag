varying highp vec2 textureCoord;
varying highp float lambertTerm;

struct LightInfo {
  highp vec4 Position; // Light position in eye coords.
  highp vec4 La; // Ambient light intensity
  highp vec4 Ld; // Diffuse light intensity
  highp vec4 Ls; // Specular light intensity
};
//struct MaterialInfo {
//  vec4 Ka; // Ambient reflectivity
//  vec4 Kd; // Diffuse reflectivity
//  vec4 Ks; // Specular reflectivity
//  float Shininess; // Specular shininess factor
//};

uniform LightInfo light;
//uniform MaterialInfo material;

uniform sampler2D texture0;
uniform sampler2D texture1;

void main(void)
{
  highp vec4 diffuse = light.Ld*lambertTerm/*+material.Kd*lambertTerm*/;
  //Don't let the dark face all black
//  diffuse = max( diffuse, 0.1 );
  diffuse = clamp(diffuse, 0.05, 1.0);

  highp vec4 tex0_color = texture2D(texture0, textureCoord);
  highp vec4 tex1_color = texture2D(texture1, textureCoord);
//  vec4 ambient_color = gl_FrontLightModelProduct.sceneColor+light.La/**gl_FrontMaterial.ambient*/;
//    vec4 ambient_color = light.La/**gl_FrontMaterial.ambient*/;
//  vec4 specular_color = light.Ls*material.Ks/**spec_factor*/;

//  gl_FragColor = (diffuse)*tex0_color/*+specular_color*tex_color*/;
    gl_FragColor = mix(tex1_color, tex0_color*diffuse, min(1.0, lambertTerm+0.9));
}
