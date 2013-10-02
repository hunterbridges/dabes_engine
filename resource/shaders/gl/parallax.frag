#version 110
varying vec2 textureVarying;
uniform vec2 repeatSize;

uniform vec2 cameraPos;
uniform float parallaxFactor;
uniform float texScale;
uniform float repeats;
uniform float xShift;

uniform sampler2D texture;
uniform vec2 texPortion;

uniform sampler2D cascade;
uniform vec2 cascadePortion;

void main()
{
  vec2 texCoord = textureVarying;
  vec2 cascadeCoord = vec2(0.5, texCoord.y) * cascadePortion;
  vec4 cascadeColor = texture2D(cascade, cascadeCoord);
    
  texCoord.x = texCoord.x + (cameraPos.x * parallaxFactor * cascadeColor.r * texScale) +
    (xShift * texScale) - 0.618;
  texCoord.x = mod(texCoord.x, repeatSize.x) / repeatSize.x;
  vec2 realCoord = texCoord * texPortion;
  vec4 texColor = texture2D(texture, realCoord);
  gl_FragColor = texColor;
}
