// Copyright 2012 Greg Simon under the GNU LGPL license
// version 2.0 or 2.1.  You should have received a copy of the LGPL
// license along with this library if you did not you can find
// it at http://www.gnu.org/.

#ifndef __blockfs_h__
#define __blockfs_h__

#include <list>
#include <string>

class BlockfsFile;

class BlockFs
{
public:
  BlockFs();
  ~BlockFs();

  bool initWithMemory(const unsigned char *data, unsigned long sz);

  // directory listings
  int fileList(std::list<std::string>&);

  bool fileExists(const char* filename);
  BlockfsFile* open(const char* filename, const char* flags);

private:
  const unsigned char* data_;
  unsigned long sz_;

  struct FileEntry {
  	std::string name;
  	unsigned long offset;
  	unsigned long size;
  };
  std::list<FileEntry> files_;

};

class BlockfsFile
{
public:
  BlockfsFile(const unsigned char* base, unsigned long sz);
  ~BlockfsFile() {}

  int GetC();
  int UnGetC(int c);
  int Read(unsigned char* buffer, int len);
  int SeekFromStart(int offset);
  int SeekFromCurrent(int offset);
  int Peek();
  long int Tell();

private:
	const unsigned char* base_;
	const unsigned char* ptr_;
	const unsigned char* end_;
	unsigned long size_;
};

#endif
