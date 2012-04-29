// Copyright 2012 Greg Simon under the GNU LGPL license
// version 2.0 or 2.1.  You should have received a copy of the LGPL
// license along with this library if you did not you can find
// it at http://www.gnu.org/.

#ifndef __blockfs_builder_h__
#define __blockfs_builder_h__


int blockfs_pack_dir(const char* in_folder, const char* output_name);
bool read_file(const char* full_path, unsigned char** outdata, unsigned long* sz);

#endif
