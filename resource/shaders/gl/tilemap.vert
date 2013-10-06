#version 110
attribute vec4 position;
attribute vec4 texture;
varying vec4 textureVarying;
uniform mat4 projection;
uniform mat4 modelView;
uniform vec2 tileSize;
uniform vec2 mapSize;

void main()
{
  gl_Position = projection * (modelView * position);
  textureVarying = texture;
}
