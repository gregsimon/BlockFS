// Copyright 2012 Greg Simon under the GNU LGPL license
// version 2.0 or 2.1.  You should have received a copy of the LGPL
// license along with this library if you did not you can find
// it at http://www.gnu.org/.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>


#include "blockfs.h"
#include "blockfs_builder.h"

int main(int argc, char** argv)
{
  if (argc < 3) {
    printf("Usage: %s (build|test) <in_path> <out_file>\n", argv[0]);
    return -1;
  }

  if (!strcmp(argv[1], "build")) {
    if (argc < 4) {
      printf(" not enough args\n");
      return -2;
    }

    char* path = argv[2];
    char* output_name = argv[3];

    return blockfs_pack_dir(path, output_name);
  }
  else if (!strcmp(argv[1], "test")) {
    char* filename_block = argv[2];

    unsigned char* data;
    unsigned long sz;
    if (!read_file(filename_block, &data, &sz)) {
      printf("Unable to load file %s\n", filename_block);
      return -1;
    }

    BlockFs* fs = new BlockFs();
    fs->initWithMemory(data, sz);

    BlockfsFile* file = fs->open("url.amf", "");
    int ch = file->getc();
    while (ch != -1) {
      putc(ch, stdout);
      ch = file->getc();
    }
    delete file;

    delete fs;
    delete[] data;
  }

  return 0;
}