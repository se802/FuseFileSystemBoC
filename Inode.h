//
// Created by csdeptucy on 7/8/23.
//

#ifndef VERONA_RT_ALL_INODE_H
#define VERONA_RT_ALL_INODE_H

#include "main.h"
#define BLOCK_SIZE 4096

// Forward declaration of the FileSystem class.
class FileSystem;

// Base class for representing Inodes in the file system.
class Inode {
public:
    // Constructor for the Inode class.
    Inode(
            fuse_ino_t ino,    // Inode number.
            time_t time,        // Time of creation/modification.
            uid_t uid,          // User ID of the owner.
            gid_t gid,          // Group ID of the owner.
            blksize_t blksize,  // Block size.
            FileSystem* fs)     // Pointer to the file system instance.
            : ino(ino)
            , fs_(fs) {
        // Initialize the stat structure for the inode.
        memset(&i_st, 0, sizeof(struct stat));
        i_st.st_ino = ino;
        i_st.st_atime = time;
        i_st.st_mtime = time;
        i_st.st_ctime = time;
        i_st.st_uid = uid;
        i_st.st_gid = gid;
        i_st.st_blksize = blksize;
    }

    // Virtual destructor for the Inode class.
    virtual ~Inode() = 0;

    // Inode number.
    fuse_ino_t ino;

    // Stat structure representing the attributes of the inode.
    struct stat i_st;

    // Check if the inode is a regular file (not a directory or symbolic link).
    bool is_regular() const;

    // Reference count for the inode.
    long int krefs = 0;

protected:
    // Pointer to the FileSystem instance that owns the inode.
    FileSystem* fs_;
};

#endif // VERONA_RT_ALL_INODE_H
