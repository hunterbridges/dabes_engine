precision lowp float;
varying vec4 colorVarying;
varying vec4 textureVarying;
uniform sampler2D texture;

void main()
{
  vec2 texCoord = vec2(textureVarying.x, textureVarying.y);
  vec4 color = texture2D(texture, texCoord);
  vec4 mixed = color + colorVarying * (1.0 - color.a);
  gl_FragColor = mixed;
}
