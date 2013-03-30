require "stringio"
require "base64"
require "zlib"

to_check = 'H4sIAAAAAAAAA2NgGAWjYBQMZcCIhMnVy4QkxoaEGUnAzGh6sZlHCQYBALr6ebewBAAA'

decoded = Base64.decode64(to_check)
gz = Zlib::GzipReader.new( StringIO.new (decoded) )
gunzipped = gz.read

i = 0
cell = 0
gids = []
bytes = []

gunzipped.bytes.each do |byte|
  bytes.push(byte)
end

puts bytes.length

while i < bytes.length
  gid = bytes[i]
  gid |= bytes[i + 1] << 8
  gid |= bytes[i + 2] << 16
  gid |= bytes[i + 3] << 24

  gids.push(gid)
  print gid.to_s + " "
  i += 4
  cell += 1
end

