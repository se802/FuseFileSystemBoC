//
// Created by csdeptucy on 7/8/23.
//

#include "Block.h"

void Block::write(const char* data, uint64_t bytes_to_write, uint64_t block_offset,uint64_t bytes_written) const
{
  memcpy(buf.get()+block_offset,data+bytes_written,bytes_to_write);
}

void Block::read(char* data, uint64_t bytes_to_read, uint64_t block_offset, uint64_t bytes_read) const
{
  memcpy(data+bytes_read, buf.get()+block_offset, bytes_to_read);
}