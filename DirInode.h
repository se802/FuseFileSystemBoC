//
// Created by csdeptucy on 7/8/23.
//

#ifndef VERONA_RT_ALL_DIRINODE_H
#define VERONA_RT_ALL_DIRINODE_H

#include "Inode.h"

class DirInode : public Inode {
public:
  //using name = type;
  using dir_t = std::map<std::string, uint64_t>  ;

  DirInode(
    fuse_ino_t &ino,
    time_t time,
    uid_t uid,
    gid_t gid,
    blksize_t blksize,
    mode_t mode,
    FileSystem* fs)
  : Inode(ino==1? 1: reinterpret_cast<fuse_ino_t >(this), time, uid, gid, blksize, fs) {
    i_st.st_nlink = 2;
    i_st.st_blocks = 1;
    i_st.st_mode = S_IFDIR | mode;
    ino = reinterpret_cast<fuse_ino_t >(this);
  }

  // Default constructor
  DirInode() : Inode(-1, -1, -1, -1, -1, nullptr) {
    // Initialize other members if needed
    i_st.st_nlink  = -1;
    i_st.st_blocks = -1;
    i_st.st_mode   = -1;
    fake = true;
  }

  bool fake = false;

  ~DirInode();

  dir_t dentries;
};

#endif // VERONA_RT_ALL_DIRINODE_H
