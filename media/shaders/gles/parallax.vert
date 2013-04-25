precision highp float;
attribute vec4 position;
attribute vec4 texture;
varying vec4 textureVarying;
uniform mat4 projection;
uniform mat4 modelView;

void main()
{
  gl_Position = projection * modelView * position;
  textureVarying = texture;
}
