#version 110
varying vec2 textureVarying;
uniform sampler2D texture;
uniform vec2 repeatSize;
uniform vec2 texPortion;

void main()
{
  vec2 texCoord = textureVarying;
  texCoord.x = mod(texCoord.x, repeatSize.x) / repeatSize.x;
  vec2 realCoord = texCoord * texPortion;
  vec4 texColor = texture2D(texture, realCoord);
  gl_FragColor = texColor;
}
