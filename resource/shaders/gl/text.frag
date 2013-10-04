#version 110
varying vec2 texPosVarying;
uniform vec4 color;
uniform sampler2D texture;

void main()
{
  vec2 texCoord = texPosVarying;
  vec4 texColor = texture2D(texture, texCoord);
  vec4 glyphColor = vec4(1.0, 1.0, 1.0, texColor.r);
  vec4 mixed = glyphColor * color;
  gl_FragColor = mixed;
}
