// Created by csdeptucy on 7/8/23.

#ifndef VERONA_RT_ALL_DIRINODE_H
#define VERONA_RT_ALL_DIRINODE_H

#include "Inode.h"

// The DirInode class represents a directory inode in the filesystem, inheriting from the Inode base class.
class DirInode : public Inode {
public:
    // Type alias for the directory entries, mapping file names to their corresponding inodes.
    using dir_t = std::map<std::string, uint64_t>;

    // Constructor for creating a directory inode instance.
    DirInode(
            fuse_ino_t &ino,
            time_t time,
            uid_t uid,
            gid_t gid,
            blksize_t blksize,
            mode_t mode,
            FileSystem* fs
    ) : Inode(ino == 1 ? 1 : reinterpret_cast<fuse_ino_t>(this), time, uid, gid, blksize, fs) {
        i_st.st_nlink = 2;
        i_st.st_blocks = 1;
        i_st.st_mode = S_IFDIR | mode;
        ino = reinterpret_cast<fuse_ino_t>(this);
    }

    // Default constructor for the directory inode.
    DirInode() : Inode(-1, -1, -1, -1, -1, nullptr) {
        // Initialize other members if needed
        i_st.st_nlink = -1;
        i_st.st_blocks = -1;
        i_st.st_mode = -1;
        fake = true;
    }

    // Destructor for the directory inode.
    ~DirInode();

    // Boolean flag indicating if the directory inode is a fake (empty) inode.
    bool fake = false;

    // A map representing directory entries (file names to their corresponding inodes) for this directory inode.
    dir_t dentries;
};

#endif // VERONA_RT_ALL_DIRINODE_H
