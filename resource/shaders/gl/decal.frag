#version 110
varying vec4 colorVarying;
varying vec4 alphaVarying;
varying vec4 textureVarying;
uniform float texAlphaAdj;
uniform sampler2D texture;

void main()
{
  vec4 texColor = texture2D(texture, textureVarying.xy);
  texColor.a = texColor.a * texAlphaAdj;
  
  vec4 bgColor = colorVarying;
  
	vec4 mixed = mix(texColor, bgColor, 1.0 - texColor.a);
  
	mixed.a = mixed.a * alphaVarying.a;
  
  gl_FragColor = mixed;
}
