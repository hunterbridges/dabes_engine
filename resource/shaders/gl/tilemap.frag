#version 110
uniform vec2 tileSize;
uniform vec2 sheetRowsCols;
uniform vec2 sheetPotSize;
uniform vec2 sheetPortion;
uniform vec2 mapRowsCols;
uniform vec2 texelPerMap;
varying vec4 textureVarying;
uniform sampler2D atlas;
uniform sampler2D tileset;

void main()
{
  vec4 tileVal = texture2D(atlas, textureVarying.xy);

  vec2 baseTilePos = tileVal.xy * sheetPotSize;

  vec2 internalPos = mod(textureVarying.xy, texelPerMap) * mapRowsCols * sheetPortion;
  vec4 tileColor = texture2D(tileset, baseTilePos + internalPos);

  tileColor.a = tileColor.a * tileVal.a;
  gl_FragColor = tileColor;
}
