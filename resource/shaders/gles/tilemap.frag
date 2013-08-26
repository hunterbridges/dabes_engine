precision mediump float;
uniform vec2 tileSize;
uniform vec2 sheetRowsCols;
uniform vec2 sheetPotSize;
uniform vec2 mapRowsCols;
varying vec4 textureVarying;
uniform sampler2D atlas;
uniform sampler2D tileset;

void main()
{
  vec2 tilePos = floor(textureVarying.xy * mapRowsCols) / mapRowsCols;
  vec4 tileVal = texture2D(atlas, tilePos);

  int index_1 = int(tileVal.g * 255.0);
  int index_2 = int(tileVal.b * 255.0);
  int index_3 = int(tileVal.a * 255.0);
  
  int intindex = index_3;
  intindex = intindex + index_2 * 256;
  intindex = intindex + index_1 * 256 * 256;
  
  float index = float(intindex);
  float sheet_col = mod(index, sheetRowsCols.x);
  float sheet_row = floor(index / sheetRowsCols.y);
  float pot_index = sheet_row * sheetPotSize.x + sheet_col;
  vec2 baseTilePos = vec2(floor(mod(pot_index, sheetPotSize.x)),
                          floor(pot_index / sheetPotSize.y)) / sheetPotSize;

  vec2 perMap = 1.0 / mapRowsCols;
  vec2 internalPos = mod(textureVarying.xy, perMap) / (perMap * sheetPotSize);
  vec4 tileColor = texture2D(tileset, baseTilePos + internalPos);

  tileColor.a = tileColor.a * (1.0 - tileVal.r);
  gl_FragColor = tileColor;
}
