#version 110
uniform sampler2D tex;
void main()
{
    vec4 color = texture2D(tex, gl_TexCoord[0].st);
    vec4 mixed = max(gl_Color, color);
    gl_FragColor = mixed;
}
