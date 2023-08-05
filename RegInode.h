//
// Created by csdeptucy on 7/8/23.
//

#ifndef VERONA_RT_ALL_REGINODE_H
#define VERONA_RT_ALL_REGINODE_H

#include "Inode.h"
#include "Block.h"

class RegInode : public Inode {
public:
  RegInode(
    fuse_ino_t &ino,
    time_t time,
    uid_t uid,
    gid_t gid,
    blksize_t blksize,
    mode_t mode,
    FileSystem* fs)
  : Inode(reinterpret_cast<fuse_ino_t >(this), time, uid, gid, blksize,  fs) {
    i_st.st_nlink = 1;
    i_st.st_size = 0;
    i_st.st_mode = S_IFREG | mode;
    ino = reinterpret_cast<fuse_ino_t >(this);
  }

  // Default constructor
  RegInode() : Inode(-1, -1, -1, -1, -1, nullptr) {
    // Initialize other members if needed
    i_st.st_nlink = -1;
    i_st.st_size = -1;
    i_st.st_mode = S_IFREG;
    fake = true;
  }

  bool fake = false;

  ~RegInode() override;

  void write(const char *buf,size_t size,off_t offset,fuse_req_t req, std::atomic<size_t >&avail_bytes, char *ptr);
  int read(size_t size,off_t offset,fuse_req_t req);
  int allocate_space(uint64_t blockID, std::atomic<size_t> &avail_bytes);
  std::map<off_t, cown_ptr<Block>> data_blocks;
};


struct my_fuse_req {
    struct fuse_session *se;
    uint64_t unique;
    int ctr;
    pthread_mutex_t lock;
    struct fuse_ctx ctx;
    struct fuse_chan *ch;
    int interrupted;
    unsigned int ioctl_64bit : 1;
    union {
        struct {
            uint64_t unique;
        } i;
        struct {
            fuse_interrupt_func_t func;
            void *data;
        } ni;
    } u;
    struct fuse_req *next;
    struct fuse_req *prev;
};

#endif // VERONA_RT_ALL_REGINODE_H
