attribute vec4 position;
attribute vec4 color;
varying vec4 colorVarying;
uniform mat4 projection;
uniform mat4 modelView;

void main()
{
  gl_Position = projection * modelView * position;
  colorVarying = color;
}
