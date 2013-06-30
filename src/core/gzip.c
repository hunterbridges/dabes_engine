#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <lcthw/bstrlib.h>
#include "../dbg.h"

unsigned long int decompress_data(unsigned char* abSrc, unsigned long nLenSrc, unsigned char **abDst,
                    int nLenDst) {
  z_stream zInfo = {.avail_in = 0};
  int prevSize = nLenDst;
  unsigned char *src_start = abSrc;
  unsigned char *dst_start = *abDst;

  zInfo.total_in = zInfo.avail_in = (int)nLenSrc;
  zInfo.total_out = zInfo.avail_out = nLenDst;
  zInfo.next_in = src_start;
  zInfo.next_out = dst_start;

  long int nErr, nRet= -1;
  nErr = inflateInit2(&zInfo, 15 + 32);
  if (nErr == Z_OK) {
    do {
      nErr = inflate(&zInfo, Z_SYNC_FLUSH);

      switch (nErr) {
        case Z_NEED_DICT:
        case Z_STREAM_ERROR:
          nErr = Z_DATA_ERROR;
          printf("Data error %ld", nErr);
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
          inflateEnd(&zInfo);
          printf("%s\n", zInfo.msg);
          return zInfo.total_out;
      }

      if (nErr != Z_STREAM_END) {
        *abDst = realloc(*abDst, prevSize + nLenDst);
        zInfo.next_out = *abDst + prevSize;
        zInfo.avail_out = nLenDst;
        prevSize += nLenDst;
      }
    } while (nErr != Z_STREAM_END);

    nRet = zInfo.total_out;
  }
  //*abDst = realloc(*abDst, zInfo.total_out);
  inflateEnd(&zInfo);
  return(nRet);
}
