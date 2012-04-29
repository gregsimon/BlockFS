// Copyright 2012 Greg Simon under the GNU LGPL license
// version 2.0 or 2.1.  You should have received a copy of the LGPL
// license along with this library if you did not you can find
// it at http://www.gnu.org/.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include <sys/types.h>
#include <dirent.h>

#include "Blockfs.h"

static unsigned long read_u32(const unsigned char*& ptr) {
  unsigned long v = ((*ptr++) << 24);
  v |= ((*ptr++) << 16);
  v |= ((*ptr++) << 8);
  v |= (*ptr++);
  return v;
}


BlockfsFile::BlockfsFile(const unsigned char* base, unsigned long sz)
  : base_(base)
  , ptr_(base)
  , end_(base_ + sz)
  , size_(sz)
{
}

int BlockfsFile::GetC()
{
  if (ptr_ >= end_)
    return -1;

  return *ptr_++;
}

int BlockfsFile::UnGetC(int c)
{
  ptr_--;
  if (ptr_ < base_)
    ptr_ = base_;
  return 0;
}

int BlockfsFile::Read(unsigned char* buffer, int len)
{
  if ((ptr_ + len) >= end_)
    len = (end_ - ptr_);

  memcpy(buffer, ptr_, len);

  ptr_ += len;
  return (int)len;
}

int BlockfsFile::Seek(int offset)
{
  ptr_ = base_ + offset;
  return 0;
}

// --------------------------------------------------------------

BlockFs::BlockFs()
{
}

BlockFs::~BlockFs()
{
}

bool BlockFs::initWithMemory(const unsigned char* data, unsigned long sz)
{
  data_ = data;
  sz_ = sz;

  // fs block is read-only. Let's collect the TOC right now.
  const unsigned char* ptr = data_;

  if (ptr[0] != 0xff || ptr[1] != 0x00 || ptr[2] != 0xff || ptr[3] != 0xdd) {
    printf("cookie in blockfs does not match\n");
    return false;
  }
  ptr += 4;

  unsigned long numFiles = read_u32(ptr);
  for (unsigned long i=0; i<numFiles; ++i)
  {
    FileEntry entry;
    entry.offset = read_u32(ptr);
    entry.size = read_u32(ptr);
    entry.name = std::string((const char*)ptr);
    files_.push_back(entry);
    //printf("%s %ld bytes\n", entry.name.c_str(), entry.size);
    ptr += (entry.name.size() + 1);
  }

  return true;
}

int BlockFs::fileList(std::list<std::string>& files)
{
  for (std::list<FileEntry>::const_iterator it=files_.begin(); it != files_.end(); ++it) {
    files.push_back(it->name);
  }
  return files.size();
}

bool BlockFs::fileExists(const char* filename)
{
  for (std::list<FileEntry>::const_iterator it=files_.begin(); it != files_.end(); ++it) {
    if (it->name == std::string(filename)) {
      return true;
    }
  }
  return false;
}

BlockfsFile* BlockFs::open(const char* filename, const char* flags)
{
  for (std::list<FileEntry>::const_iterator it=files_.begin(); it != files_.end(); ++it) {
    if (it->name == std::string(filename)) {
      BlockfsFile* file = new BlockfsFile(data_ + it->offset, it->size);
      return file;
    }
  }
  return 0;
}




