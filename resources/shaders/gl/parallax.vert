#version 110
attribute vec4 position;
attribute vec4 texture;
varying vec2 textureVarying;
uniform mat4 projection;
uniform mat4 modelView;
uniform vec2 cameraPos;
uniform float parallaxFactor;
uniform float texScale;
uniform float repeats;

void main()
{
  gl_Position = projection * modelView * position;
  vec2 texCoord = vec2(texture.x, texture.y);
  texCoord.x = texCoord.x + cameraPos.x * parallaxFactor * texScale - 0.618;
  textureVarying = texCoord;
}
