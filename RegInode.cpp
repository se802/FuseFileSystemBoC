//
// Created by csdeptucy on 7/8/23.
//

#include "RegInode.h"
#include "my_fuse_loop.h"


class Counter {
public:
  u_int64_t count = 0;
};


int RegInode::allocate_space(uint64_t blockID, std::atomic<size_t >&avail_bytes)
{
  //if (avail_bytes < BLOCK_SIZE) return -ENOSPC;
  //avail_bytes -= BLOCK_SIZE;
  data_blocks.emplace(blockID, make_cown<Block>(BLOCK_SIZE,blockID));
  return 0;
}


class write_data
{
public:
  write_data(cown_ptr<Block>& ptr, int bytes_to_write, int block_offset, int bytes_written,int total_bytes,const char *buf)
  : ptr_(ptr), bytes_to_write_(bytes_to_write), block_offset_(block_offset), bytes_written_(bytes_written), read_buf_(buf) {}


public:
  cown_ptr<Block> ptr_;
  int bytes_to_write_;
  int block_offset_;
  int bytes_written_;
  const char *read_buf_;
};

class read_data
{
public:
  read_data(cown_ptr<Block>& ptr, int bytes_to_read, int block_offset, int bytes_read,int total_bytes, char *buf)
  : ptr_(ptr), bytes_to_read_(bytes_to_read), block_offset_(block_offset), bytes_read_(bytes_read), read_buf_(buf) {}


public:
  cown_ptr<Block> ptr_;
  int bytes_to_read_;
  int block_offset_;
  int bytes_read_;
  char *read_buf_;
};




void batch_write(const std::vector<write_data>& blocks, fuse_req_t req, size_t size,char *ptr) {
  if (blocks.size() == 1) {
    when(blocks[0].ptr_) << [=](acquired_cown<Block> blk0) {
      blk0->write(blocks[0].read_buf_, blocks[0].bytes_to_write_, blocks[0].block_offset_, blocks[0].bytes_written_);
      free(ptr);
    };
  }

  if (blocks.size() == 2) {
    when(blocks[0].ptr_, blocks[1].ptr_) << [=](acquired_cown<Block> blk0, acquired_cown<Block> blk1) {
      blk0->write(blocks[0].read_buf_, blocks[0].bytes_to_write_, blocks[0].block_offset_, blocks[0].bytes_written_);
      blk1->write(blocks[1].read_buf_, blocks[1].bytes_to_write_, blocks[1].block_offset_, blocks[1].bytes_written_);
      free(ptr);
    };
  }

  if (blocks.size() == 3) {
    when(blocks[0].ptr_, blocks[1].ptr_, blocks[2].ptr_) << [=](acquired_cown<Block> blk0, acquired_cown<Block> blk1, acquired_cown<Block> blk2) {
      blk0->write(blocks[0].read_buf_, blocks[0].bytes_to_write_, blocks[0].block_offset_, blocks[0].bytes_written_);
      blk1->write(blocks[1].read_buf_, blocks[1].bytes_to_write_, blocks[1].block_offset_, blocks[1].bytes_written_);
      blk2->write(blocks[2].read_buf_, blocks[2].bytes_to_write_, blocks[2].block_offset_, blocks[2].bytes_written_);
      free(ptr);
    };
  }

  if (blocks.size() == 4) {
    when(blocks[0].ptr_, blocks[1].ptr_, blocks[2].ptr_, blocks[3].ptr_) << [=](acquired_cown<Block> blk0, acquired_cown<Block> blk1, acquired_cown<Block> blk2, acquired_cown<Block> blk3) {
      blk0->write(blocks[0].read_buf_, blocks[0].bytes_to_write_, blocks[0].block_offset_, blocks[0].bytes_written_);
      blk1->write(blocks[1].read_buf_, blocks[1].bytes_to_write_, blocks[1].block_offset_, blocks[1].bytes_written_);
      blk2->write(blocks[2].read_buf_, blocks[2].bytes_to_write_, blocks[2].block_offset_, blocks[2].bytes_written_);
      blk3->write(blocks[3].read_buf_, blocks[3].bytes_to_write_, blocks[3].block_offset_, blocks[3].bytes_written_);
      free(ptr);
    };
  }
}


void RegInode::write(const char* buf, size_t size, off_t offset,fuse_req_t req,std::atomic<size_t >&avail_bytes, char *ptr)
{
  auto now = std::time(nullptr);
  i_st.st_ctime = now;
  i_st.st_mtime = now;
  // Calculate the block ID based on the offset
  u_int64_t bytes_written = 0;
  u_int64_t blockId = offset/ BLOCK_SIZE;
  u_int64_t block_offset;
  u_int64_t remaining_size = size;

  off_t var = offset+size;
  i_st.st_size = std::max(i_st.st_size, var);



  while (remaining_size>0)
  {
    blockId = (offset + bytes_written) / BLOCK_SIZE;
    block_offset = (offset + bytes_written) % BLOCK_SIZE;

    if (data_blocks.find(blockId) == data_blocks.end())
    {
      data_blocks.emplace(blockId, make_cown<Block>(BLOCK_SIZE,blockId));
      int ret = allocate_space(blockId,avail_bytes);

      //if (ret){
      //  fuse_reply_err(req,-ret);
      //  exit(-99);
      //}

      i_st.st_blocks = std::min(i_st.st_blocks+1,(__blkcnt_t)data_blocks.size());
    }

    size_t bytes_to_write = std::min(BLOCK_SIZE - block_offset, remaining_size);
    cown_ptr<Block> blk = data_blocks.at(blockId);
    blk.allocated_cown->value.write(buf,bytes_to_write,block_offset,bytes_written);

    bytes_written += bytes_to_write;
    remaining_size -= bytes_to_write;
  }

  auto end = std::time(nullptr);
  auto start = write_time[req->unique];
  fuse_reply_buf(req,buf,size);
  time_t duration = end - start;
  write_time[req->unique] = duration;
  free((void *) buf);
}





//FIXME: What if someone does lseek(fd,1000,SEEK_CUR) --> write(fd,"a",1), and someone tries to read from offset 0? what will it return
int RegInode::read( size_t size, off_t offset, fuse_req_t req)
{

  // Calculate the block ID based on the offset
  u_int64_t bytes_read = 0;
  u_int64_t blockId;
  u_int64_t block_offset;
  long remaining_size = std::min((long)size, (long)i_st.st_size -offset );
  remaining_size = std::max(remaining_size,0l);

  if(remaining_size == 0)
  {
    fuse_reply_buf(req, NULL, 0);
    return 0;
  }
  char *buf = static_cast<char*>(malloc(sizeof(char) * remaining_size));

  while (remaining_size > 0)
  {
    blockId = (offset + bytes_read) / BLOCK_SIZE;
    block_offset = (offset + bytes_read) % BLOCK_SIZE;

    if(data_blocks.find(blockId) == data_blocks.end()){
      bytes_read += BLOCK_SIZE;
      remaining_size -= BLOCK_SIZE;
      continue ;
    }
    cown_ptr<Block> blk = data_blocks.at(blockId);
    size_t bytes_to_read = std::min((long)(BLOCK_SIZE - block_offset), remaining_size);
    blk.allocated_cown->value.read(buf,bytes_to_read,block_offset,bytes_read);
    bytes_read += bytes_to_read;
    remaining_size -= bytes_to_read;

  }
  auto end = std::time(nullptr);
  auto start = read_time[req->unique];
  fuse_reply_buf(req,buf,size);
  time_t duration = end - start;
  read_time[req->unique] = duration;
  free(buf);
  return 0;
}

RegInode::~RegInode() {
  std::cout << "hey" << std::endl;
};
