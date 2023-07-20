//
// Created by csdeptucy on 7/8/23.
//

#ifndef VERONA_RT_ALL_BLOCK_H
#define VERONA_RT_ALL_BLOCK_H

#include <cstdio>
#include <cstring>
#include <memory>
struct Block {
  explicit Block(size_t size,int block_number)
  : size(size),block_number(block_number)
    , buf(new char[size]) {
    memset(buf.get(),0,size);
  }

  size_t size;
  int block_number = -1;
  std::unique_ptr<char[]> buf;


  void write(const char* data, uint64_t bytes_to_write, uint64_t block_offset,uint64_t bytes_written) const;
  void read(char* data, uint64_t bytes_to_read, uint64_t block_offset,uint64_t bytes_read) const;
};

#endif // VERONA_RT_ALL_BLOCK_H
