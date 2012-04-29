// Copyright 2012 Greg Simon under the GNU LGPL license
// version 2.0 or 2.1.  You should have received a copy of the LGPL
// license along with this library if you did not you can find
// it at http://www.gnu.org/.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <list>

#include <sys/types.h>
#include <dirent.h>

#include "blockfs_builder.h"


static bool processFile(const char* inFileName, const char*, FILE* outFile);

static int write_u32(FILE* f, unsigned long v) {
  fputc(v >> 24, f);
  fputc((v >> 16)&0xff, f);
  fputc((v >> 8)&0xff, f);
  fputc((v)&0xff, f);
  return 0;
}

int blockfs_pack_dir(const char* in_folder, const char* output_name)
{
  FILE* in_file;
  DIR* d;
  const char* path = in_folder;

  d = opendir(path);
  if (!d) {
    printf("Error opening folder %s\n", path);
    return -1;
  }


  std::list<std::string> file_list;

  unsigned long toc_size = 8;

  // read in all files
  while (1) {
    struct dirent * entry;

    entry = readdir (d);
    if (! entry)
      break;

    if (entry->d_name[0] == '.')
      continue;

    file_list.push_back(entry->d_name);

    toc_size += 4; // offset
    toc_size += 4; // filesize
    toc_size += strlen(entry->d_name) + 1;
  }
  closedir(d);

  printf("toc is %d bytes.\n", (int)toc_size);


  // now create the packed output file. We'll create the TOC first.
  FILE* out_file = fopen(output_name, "wb");
  if (!out_file) {
    printf("Failed to open output file %s\n", output_name);
    return -2;
  }

  // write the TOC.
  unsigned long running_file_offset = toc_size;

  // TODO : really could just do this with one pass

  write_u32(out_file, 0xff00ffdd); // cookie
  write_u32(out_file, file_list.size()); // # of files
  for (std::list<std::string>::const_iterator it=file_list.begin(); 
        it != file_list.end(); ++it)
  {
    unsigned char* data;
    unsigned long sz;
    char full_path[1024];
  
    sprintf(full_path, "%s/%s", path, it->c_str());

    if (!read_file(full_path, &data, &sz))
      continue;

    // write TOC entry.
    write_u32(out_file, running_file_offset);
    write_u32(out_file, sz);
    fwrite(it->c_str(), 1, it->size(), out_file);
    fputc(0, out_file);

    delete[] data;

    running_file_offset += sz;

    printf("%s %ld bytes ...\n", it->c_str(), sz);
  }

  // write file payloads... 
  for (std::list<std::string>::const_iterator it=file_list.begin(); 
        it != file_list.end(); ++it)
  {
    unsigned char* data;
    unsigned long sz;
    char full_path[1024];
  
    sprintf(full_path, "%s/%s", path, it->c_str());

    if (!read_file(full_path, &data, &sz))
      continue;

    fwrite(data, 1, sz, out_file);

    delete[] data;
  }

  fclose(out_file);

  return 0;
}

bool read_file(const char* full_path, unsigned char** outdata, unsigned long* sz)
{
  FILE* f = fopen(full_path, "rb");
  if (!f)
    return false;

  fseek(f, 0, SEEK_END);
  *sz = ftell(f);
  fseek(f, 0, SEEK_SET);

  unsigned char* data = new unsigned char[*sz];
  if (!data) {
    printf("failed to allocate %ld bytes\n", *sz);
    fclose(f);
    return false;
  }

  *outdata = data;

  fread(data, 1, *sz, f);
  fclose(f);
  return true;
}


bool processFile(const char *path, const char* file_name, FILE* out_file)
{
  char full_path[1024];
  sprintf(full_path, "%s/%s", path, file_name);

  FILE* f = fopen(full_path, "rb");
  if (!f)
    return false;

  fseek(f, 0, SEEK_END);
  unsigned long sz = ftell(f);
  fseek(f, 0, SEEK_SET);

  printf("processing %s %d bytes\n", file_name,  (int)sz);

  unsigned char* data = new unsigned char[sz];
  if (!data) {
    printf("failed to allocate %d bytes\n", (int)sz);
    fclose(f);
    return false;
  }

  fread(data, 1, sz, f);
  fclose(f);

  // write the filename
  fwrite(file_name, 1, strlen(file_name), out_file);
  fputc(0, out_file);
  
  // write the filesize 32bit BE
  fputc((sz >> 24), out_file);
  fputc((sz >> 16), out_file);
  fputc((sz >> 8), out_file);
  fputc(sz, out_file);

  fwrite(data, 1, sz, out_file);

  delete[] data;
  return true;
}
