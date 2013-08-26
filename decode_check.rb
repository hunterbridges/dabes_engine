require "stringio"
require "base64"
require "zlib"

to_check = 'H4sIAAAAAAAAA+3bRcwcNxTA8ecvTXIoMzecHMrMbTiHlJnb8KFtmLHhQ5g5KfcQZk7KPSRlZm4PZWb4j1aj7XovHttTz2b2ST9ppR17vNb3nsaez/VEpL5UI2nUwl64GM3DDqUioxEa43bcEXYoFRmt0Br1pJq/NtEJnaWav7YxGmOkmr+28QAelGr+2sYTeFKq+WsbH+MTqeavbdRSPP+pys/f7uiBnuiF3uiDvuiH/hiAgRiEwRiCoRiG4Rghxefh2qhjcN9GzF1jVfn5OxlTMBXTMB0zMBOzMBtzMBfzMB8LsBCLsBhLpPg83ARNDe7birlrrSo/f1dgJVZhNdZgLdZhPTZgIzZhM7ZgK7ZhO3ZgpxSfh9ugrcF9OzF3nfeA/H0Oz+MFvIiX8DJewat4Da/jDbyJt/A23sG7eA/vS/F5uAu6Gtx3NHM3JgP561q/vsY3+Bbf4Xv8gB/xE37GL/gVv+F3/IE/8Rf+xj9SfB4ei3EG447nL3T+utav/fgN++MAHIiDcDAOwaE4DIfjCByJo3A0jsGxOC7KQVV8Hn4IDxuMO65/ofPXtX6dxG84GafgVJyG03EGzsRZOBvn4Fych/NxAS7ERVEOquLz8FN42mDccf0Lnb+u9esyfsPluAJX4ipcjWtwLa7D9bgBN+Im3IxbcCtui3JQFZ+HP8VnBuP2lb+h61d3fkMP9EQv9EYf9EU/9McADMQgDMYQDMUwDMcIVXwero06ynz+XPM3dP2ajCmYimmYjhmYiVmYjTmYi3mYjwVYiEVYjCWq+DzcBE0N5i+uf675G7p+rcBKrMJqrMFarMN6bMBGbMJmbMFWbMN27MBOVZyPNmhrMH9x/XPN39D1S4+kf0dxxPPRBV3/p/z9SsLXLz2S/h3p8zEW4xLMn0v+7qvC1y89kv4d6fNhOn8+1r8nqvD1y3Ue9PlIWv9c8vdSlZ36FYft/NnWP5f8vVtlp365tretfy75O0llp365tretfy75uzxD9cu1vW39873+DVW/XNvb1j/f+1eh6pdre9v653v/KlT9cm1vW/9852+o/HNpH71zC7X+/W9E67kaTfx+8J6Mfxdi/atHtJ5rKKXi94P3Sra/C7H+1SNaz7WUUvH7wUcl29+FWP/qEa3nOkqp+P3gB5Lt70Ksf/WI1nOjpFT8flAy/l2I9a8e0XrufikVvx+sn/HvQqx/9YjWc49Lqfj9YPOMfxdi/bsnRVrr3wmYiEmO4/PVT1p9pbX+XYplWO44Tl/9pNFXFGmtf3dhN551HKevfnz3ZRum698v8KWH+/nqJ42+bMJ0/bs31+xjsY5Pq580+rIJ0/w9nmtO8DBWX/2k0ZdNmD4/t+eaSzyM1Vc/afRlE6br3zu55i4PY/XVTxp92YTp+ncC10z0MFZf/aTRl02Y5u9SrlnmYay++kmjL5swfX7exTW7PYzVVz9p9GUT+vo36qZGys/h1JVke+J5aa+vfxtIYR9bP4fTTJLtieelvX5+v4UU9rH1czjtJNmeeF7a6+f3O0hhH1s/h9NNku2J56W9fn5/pBT2sfVzOOMl2Z54Xtrr5/fvk8I+tn4O5xFJtieel/b6+f3HpLCPrZ/DeUaS7Ynnpb1+fv9DfCTl53A+F/PIU3v9/L5ibmtU+TmcugmeL/PUXj+/34DPDVX5/+81S3D/PLXXz++34HNLVf4eql2C++epvb5/1YHPHVX5e6huCe6fp/b6/tVIPo9S5e+hxie4f57ax9f9C+082r4AUAAA'

def getbytes(str)
  bytes = []

  str.bytes.each do |byte|
    bytes.push(byte)
  end

  bytes
end

def printbytes(bytes)
  str = ""
  bytes.each do |byte|
    str << byte.to_s(16) + ' '
  end
  puts str
end

decoded = Base64.decode64(to_check)

puts "B64 Decoded"
puts "==========="
puts printbytes(getbytes(decoded))
puts ""

gz = Zlib::GzipReader.new( StringIO.new (decoded) )
gunzipped = gz.read

i = 0
cell = 0
gids = []
bytes = getbytes(gunzipped)

puts "Total Gunzipped Bytes: #{bytes.length}"
puts ""

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

puts ""
