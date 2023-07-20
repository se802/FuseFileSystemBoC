//
// Created by csdeptucy on 7/8/23.
//

#ifndef VERONA_RT_ALL_INODE_H
#define VERONA_RT_ALL_INODE_H

#include "main.h"
#define BLOCK_SIZE 4096

class FileSystem;

  class Inode {
public:
  Inode(
    fuse_ino_t ino,
    time_t time,
    uid_t uid,
    gid_t gid,
    blksize_t blksize,
    FileSystem* fs)
  : ino(ino)
    , fs_(fs) {
    memset(&i_st, 0, sizeof(struct stat));
    i_st.st_ino = ino;
    i_st.st_atime = time;
    i_st.st_mtime = time;
    i_st.st_ctime = time;
    i_st.st_uid = uid;
    i_st.st_gid = gid;
    i_st.st_blksize = blksize;
  }
  virtual ~Inode() = 0;

  fuse_ino_t ino;

  struct stat i_st;

  bool is_regular() const;
  long int krefs = 0;
protected:
  FileSystem* fs_;
};

#endif // VERONA_RT_ALL_INODE_H
