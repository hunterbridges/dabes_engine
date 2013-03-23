precision mediump float;
varying vec4 colorVarying;

void main()
{
  /*
    vec4 color = texture2D(tex, gl_TexCoord[0].st);
    vec4 mixed = color + gl_Color * (1.0 - color.a);
    gl_FragColor = mixed;
   */
  gl_FragColor = colorVarying;
}
