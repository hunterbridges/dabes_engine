#version 110
uniform vec2 tileSize;
uniform vec2 sheetRowsCols;
uniform vec2 sheetPotSize;
uniform vec2 mapRowsCols;
varying vec4 textureVarying;
uniform sampler2D atlas;
uniform sampler2D tileset;

void main()
{
  vec2 tilePos = (floor(textureVarying.xy * mapRowsCols) + 0.5) / mapRowsCols;
  vec4 tileVal = texture2D(atlas, tilePos);
  float cell = tileVal.b;

  float index = floor(cell * 255.0);
  float sheet_col = mod(index, sheetRowsCols.x);
  float sheet_row = floor(index / sheetRowsCols.y);
  float pot_index = sheet_row * sheetPotSize.x + sheet_col;
  vec2 baseTilePos = vec2(floor(mod(pot_index, sheetPotSize.x)),
                          floor(pot_index / sheetPotSize.y)) / sheetPotSize;

  vec2 perMap = 1.0 / mapRowsCols;
  vec2 internalPos = mod(textureVarying.xy, perMap) / (perMap * sheetPotSize);
  vec4 tileColor = texture2D(tileset, baseTilePos + internalPos);

  tileColor.a = tileColor.a * (1.0 - tileVal.a);
  gl_FragColor = tileColor;
}
