precision highp float;
varying vec4 textureVarying;
uniform vec2 repeatSize;
uniform vec2 cameraPos;
uniform vec2 texPortion;
uniform float parallaxFactor;
uniform float texScale;
uniform float repeats;
uniform sampler2D texture;

void main()
{
  vec2 texCoord = vec2(textureVarying.x, textureVarying.y);
  texCoord.x = mod(texCoord.x + cameraPos.x * parallaxFactor * texScale - 0.618,
                   repeatSize.x) / repeatSize.x;
  vec2 realCoord = texCoord * texPortion;
  vec4 texColor = texture2D(texture, realCoord);
  gl_FragColor = texColor;
}
