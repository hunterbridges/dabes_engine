precision highp float;
varying vec4 colorVarying;
varying vec4 texPosVarying;
uniform sampler2D texture;

void main()
{
  vec2 texCoord = vec2(texPosVarying.x, texPosVarying.y);
  vec4 texColor = texture2D(texture, texCoord);
  vec4 glyphColor = vec4(1.0, 1.0, 1.0, texColor.r);
  vec4 mixed = glyphColor * colorVarying;
  gl_FragColor = mixed;
}

