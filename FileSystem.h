//
// Created by csdeptucy on 7/8/23.
//

#ifndef VERONA_RT_ALL_FILESYSTEM_H
#define VERONA_RT_ALL_FILESYSTEM_H

#include "DirInode.h"
#include "RegInode.h"
#include "fuse3/fuse_lowlevel.h"

#include <cstring>
#include <string>

// Structure representing a file handle
struct FileHandle {
    cown_ptr<RegInode> in;
    int flags;

    // Constructor for FileHandle
    FileHandle(cown_ptr<RegInode> in, int flags)
            : in(in), flags(flags) {}
};

// Abstract base class for the file system implementation
class filesystem_base {
public:
    // Constructor
    filesystem_base() {
        // Initialize the fuse_lowlevel_ops struct with zero values
        std::memset(&ops_, 0, sizeof(ops_));
        // Assign function pointers to the corresponding fuse operations
        ops_.init = ll_init;
        ops_.create = ll_create;
        ops_.release = ll_release;
        ops_.unlink = ll_unlink;
        ops_.forget = ll_forget;
        ops_.getattr = ll_getattr;
        ops_.lookup = ll_lookup;
        ops_.opendir = ll_opendir;
        ops_.readdir = ll_readdir;
        ops_.open = ll_open;
        ops_.write = ll_write;
        ops_.read = ll_read;
        ops_.mkdir = ll_mkdir;
        ops_.rmdir = ll_rmdir;
        ops_.rename = ll_rename;
        ops_.setattr = ll_setattr;
        ops_.access = ll_access;
        ops_.fallocate = ll_fallocate;
    }

public:
    // Pure virtual functions representing various FUSE operations that the derived class must implement
    virtual int lookup(fuse_ino_t parent_ino, const std::string& name, fuse_req_t req) = 0;
    virtual void forget(fuse_ino_t ino, long unsigned nlookup, fuse_req_t req) = 0;
    virtual void init(void* userdata, struct fuse_conn_info* conn) = 0;
    virtual int rename(fuse_ino_t parent_ino, const std::string& name, fuse_ino_t newparent_ino, const std::string& newname, uid_t uid, gid_t gid, fuse_req_t req) = 0;
    virtual int unlink(fuse_ino_t parent_ino, const std::string& name, uid_t uid, gid_t gid, fuse_req_t req) = 0;
    virtual int access(fuse_ino_t ino, int mask, uid_t uid, gid_t gid, fuse_req_t req) = 0;
    virtual int getattr(fuse_ino_t ino, uid_t uid, gid_t gid, fuse_req_t req, int* ptr) = 0;
    virtual int setattr(fuse_ino_t ino, FileHandle* fh, struct stat* attr, int to_set, uid_t uid, gid_t gid, fuse_req_t req) = 0;
    virtual int mkdir(fuse_ino_t parent_ino, const std::string& name, mode_t mode, uid_t uid, gid_t gid, fuse_req_t req) = 0;
    virtual ssize_t readdir(fuse_req_t req, fuse_ino_t ino, size_t bufsize, off_t off) = 0;
    virtual int rmdir(fuse_ino_t parent_ino, const std::string& name, uid_t uid, gid_t gid, fuse_req_t req) = 0;
    virtual int opendir(fuse_ino_t ino, int flags, uid_t uid, gid_t gid, fuse_req_t req, struct fuse_file_info* fi) = 0;
    virtual void my_fallocate(fuse_req_t req, fuse_ino_t ino, int mode, off_t offset, off_t length, fuse_file_info* fi) = 0;
    virtual int create(fuse_ino_t parent_ino, const std::string& name, mode_t mode, int flags, uid_t uid, gid_t gid, fuse_req_t req, struct fuse_file_info* fi) = 0;
    virtual int open(fuse_ino_t ino, int flags, FileHandle** fhp, uid_t uid, gid_t gid, struct fuse_file_info* fi, fuse_req_t req) = 0;
    virtual ssize_t write(FileHandle* fh, const char* buf, size_t size, off_t off, struct fuse_file_info* fi, fuse_req_t req, char* ptr, fuse_ino_t ino) = 0;
    virtual ssize_t read(FileHandle* fh, off_t offset, size_t size, fuse_req_t req, fuse_ino_t ino) = 0;
    virtual void release(fuse_ino_t ino, FileHandle* fh) = 0;

private:
    // Static functions to retrieve the filesystem_base instance from FUSE request userdata
    static filesystem_base* get(fuse_req_t req) {
        return get(fuse_req_userdata(req));
    }

    static filesystem_base* get(void* userdata) {
        return reinterpret_cast<filesystem_base*>(userdata);
    }

// Function to initialize FUSE connection options based on capabilities.
    static void ll_init(void* userdata, struct fuse_conn_info* conn) {
        // Disable automatic invalidation of cached data on open.
        conn->want &= ~FUSE_CAP_AUTO_INVAL_DATA;

        // Set the maximum number of background requests to 12.
        conn->max_background = 12;

        // Check if the file system supports parallel directory operations.
        if (conn->capable & FUSE_CAP_PARALLEL_DIROPS)
            conn->want |= FUSE_CAP_PARALLEL_DIROPS;

        // Check if the file system supports asynchronous reads.
        if (conn->capable & FUSE_CAP_ASYNC_READ)
            conn->want |= FUSE_CAP_ASYNC_READ;

        // Check if the file system supports asynchronous direct I/O.
        if (conn->capable & FUSE_CAP_ASYNC_DIO)
            conn->want |= FUSE_CAP_ASYNC_DIO;

        // Check if the file system supports splice-based write.
        if (conn->capable & FUSE_CAP_SPLICE_WRITE)
            conn->want |= FUSE_CAP_SPLICE_WRITE;

        // Check if the file system supports splice-based read.
        if (conn->capable & FUSE_CAP_SPLICE_READ)
            conn->want |= FUSE_CAP_SPLICE_READ;

        // Get the file system instance from userdata and call its init() function.
        auto fs = get(userdata);
        fs->init(userdata, conn);
    }



// Function to handle the create FUSE operation.
    static void ll_create(fuse_req_t req, fuse_ino_t parent, const char* name, mode_t mode, struct fuse_file_info* fi) {
        // Get the file system instance from the request and context.
        auto fs = get(req);
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        // Call the file system's create() function to create a new file.
        fs->create(parent, name, mode, fi->flags, ctx->uid, ctx->gid, req, fi);
    }



// Function to handle the release FUSE operation (close a file).
    static void ll_release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
        // Get the file system instance from the request.
        auto fs = get(req);

        // Get the file handle from the fuse_file_info struct.
        auto fh = reinterpret_cast<FileHandle*>(fi->fh);

        // Call the file system's release() function to release the file.
        fs->release(ino, fh);

        // Reply to the request with success (0).
        fuse_reply_err(req, 0);
    }

  // Function to handle the unlink FUSE operation (delete a file).
  static void ll_unlink(fuse_req_t req, fuse_ino_t parent, const char* name) {
     // Get the file system instance from the request and context.
     auto fs = get(req);
     const struct fuse_ctx* ctx = fuse_req_ctx(req);

     // Call the file system's unlink() function to delete the file.
     fs->unlink(parent, name, ctx->uid, ctx->gid, req);
  }

// Function to handle the forget FUSE operation (request to forget about an inode).
// The FUSE library uses this operation to inform the file system that the kernel is no longer
// interested in the given inode. The file system should decrease the inode's reference count
// by the specified nlookup value.
    static void ll_forget(fuse_req_t req, fuse_ino_t ino, long unsigned nlookup) {
        // Get the file system instance from the request.
        auto fs = get(req);

        // Call the file system's forget() function to decrement the inode's reference count.
        // This is done to track the number of active references to the inode.
        // When the reference count becomes zero, the file system can consider the inode as "forgotten"
        // by the kernel and can potentially release any resources associated with it.
        // The 'nlookup' argument indicates the number of lookups that the kernel is "forgetting" for this inode.
        fs->forget(ino, nlookup, req);
    }


// Function to handle the getattr FUSE operation (get file attributes).
  static void ll_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
    // Get the file system instance from the request and context.
    auto fs = get(req);
    const struct fuse_ctx* ctx = fuse_req_ctx(req);

    // Call the file system's getattr() function to get the file attributes.
    // Note: The 'NULL' argument indicates that the attribute pointer is not provided.
    fs->getattr(ino, ctx->uid, ctx->gid, req, NULL);
  }

// Function to handle the lookup FUSE operation (look up a directory entry by name).
  static void ll_lookup(fuse_req_t req, fuse_ino_t parent, const char* name) {
    // Get the file system instance from the request.
    auto fs = get(req);

    // Call the file system's lookup() function to find the entry by name in the parent directory.
    fs->lookup(parent, name, req);
  }

    // Handle the opendir FUSE operation (open directory for reading).
    // Get the file system instance from the request and extract the user and group context.
    // Call the file system's opendir() function with appropriate arguments.
    // The 'fi' argument will be filled with the file handle representing the opened directory.
    static void ll_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
        auto fs = get(req); // Get file system instance from the request.
        const struct fuse_ctx* ctx = fuse_req_ctx(req); // Get user and group context.
        fs->opendir(ino, fi->flags, ctx->uid, ctx->gid, req, fi); // Call the file system's opendir() function.
    }




    // Handle the readdir FUSE operation (read directory entries).
    // Get the file system instance from the request and call the file system's readdir() function with the provided arguments.
    // This function reads directory entries starting from the given offset 'off' and fills the buffer 'size' with directory data.
    static void ll_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                           struct fuse_file_info* fi) {
        auto fs = get(req); // Get file system instance from the request.
        fs->readdir(req, ino, size, off); // Call the file system's readdir() function.
    }


    // Handle the ll_open FUSE operation (open a file or directory).
    // Get the file system instance from the request, check if the file is not being created, and call the file system's open() function with the provided arguments.
    static void ll_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
        auto fs = get(req); // Get file system instance from the request.
        const struct fuse_ctx* ctx = fuse_req_ctx(req); // Get the context of the request.
        assert(!(fi->flags & O_CREAT)); // Assert that the file is not being created.
        FileHandle* fh;
        fs->open(ino, fi->flags, &fh, ctx->uid, ctx->gid, fi, req); // Call the file system's open() function.
    }

    // Handle the ll_write FUSE operation (write data to a file).
    // Get the file system instance from the request and call the file system's write() function with the provided arguments.
    static void ll_write(fuse_req_t req, fuse_ino_t ino, const char* buf, size_t size, off_t off, struct fuse_file_info* fi) {
        auto fs = get(req); // Get file system instance from the request.
        //auto fh = reinterpret_cast<FileHandle*>(fi->fh);
        fs->write(nullptr, buf, size, off, fi, req, NULL, ino); // Call the file system's write() function.
    }

    // Handle the ll_read FUSE operation (read data from a file).
    // Get the file system instance from the request, extract the file handle from the file info, and call the file system's read() function with the provided arguments.
    static void ll_read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info* fi) {
        auto fs = get(req); // Get file system instance from the request.
        auto fh = reinterpret_cast<FileHandle*>(fi->fh); // Extract the file handle from the file info.
        fs->read(fh, off, size, req, ino); // Call the file system's read() function.
    }

    // Handle the ll_mkdir FUSE operation (create a new directory).
    // Get the file system instance from the request, get the context of the request, and call the file system's mkdir() function with the provided arguments.
    static void ll_mkdir(fuse_req_t req, fuse_ino_t parent, const char* name, mode_t mode) {
        auto fs = get(req); // Get file system instance from the request.
        const struct fuse_ctx* ctx = fuse_req_ctx(req); // Get the context of the request.
        fs->mkdir(parent, name, mode, ctx->uid, ctx->gid, req); // Call the file system's mkdir() function.
    }

    // Handle the ll_rmdir FUSE operation (remove a directory).
    // Get the file system instance from the request, get the context of the request, and call the file system's rmdir() function with the provided arguments.
    static void ll_rmdir(fuse_req_t req, fuse_ino_t parent, const char* name) {
        auto fs = get(req); // Get file system instance from the request.
        const struct fuse_ctx* ctx = fuse_req_ctx(req); // Get the context of the request.
        fs->rmdir(parent, name, ctx->uid, ctx->gid, req); // Call the file system's rmdir() function.
    }

    // Handle the ll_rename FUSE operation (rename a file or directory).
    // Get the file system instance from the request, get the context of the request, and call the file system's rename() function with the provided arguments.
    static void ll_rename(fuse_req_t req, fuse_ino_t parent, const char* name, fuse_ino_t newparent, const char* newname, unsigned int idk) {
        auto fs = get(req); // Get file system instance from the request.
        const struct fuse_ctx* ctx = fuse_req_ctx(req); // Get the context of the request.
        fs->rename(parent, name, newparent, newname, ctx->uid, ctx->gid, req); // Call the file system's rename() function.
    }

    // Handle the ll_setattr FUSE operation (set file attributes).
    // Get the file system instance from the request and the file handle from the file info (if available), and call the file system's setattr() function with the provided arguments.
    static void ll_setattr(fuse_req_t req, fuse_ino_t ino, struct stat* attr, int to_set, struct fuse_file_info* fi) {
        auto fs = get(req); // Get file system instance from the request.
        auto fh = fi ? reinterpret_cast<FileHandle*>(fi->fh) : nullptr; // Get the file handle from the file info (if available).
        const struct fuse_ctx* ctx = fuse_req_ctx(req); // Get the context of the request.
        fs->setattr(ino, fh, attr, to_set, ctx->uid, ctx->gid, req); // Call the file system's setattr() function.
    }

    // Handle the ll_access FUSE operation (check file access permissions).
    // Get the file system instance from the request, get the context of the request, and call the file system's access() function with the provided arguments.
    static void ll_access(fuse_req_t req, fuse_ino_t ino, int mask) {
        auto fs = get(req); // Get file system instance from the request.
        const struct fuse_ctx* ctx = fuse_req_ctx(req); // Get the context of the request.
        fs->access(ino, mask, ctx->uid, ctx->gid, req); // Call the file system's access() function.
    }

    // Handle the ll_fallocate FUSE operation (pre-allocate space for a file).
    // Get the file system instance from the request and call the file system's my_fallocate() function with the provided arguments.
    static void ll_fallocate(fuse_req_t req, fuse_ino_t ino, int mode, off_t offset, off_t length, struct fuse_file_info* fi) {
        auto fs = get(req); // Get file system instance from the request.
        fs->my_fallocate(req, ino, mode, offset, length, fi); // Call the file system's my_fallocate() function.
    }


protected:
  fuse_lowlevel_ops ops_;
  static void reply_fail(int ret,fuse_req_t req){
    fuse_reply_err(req, -ret);
  }
};

class FileSystem : public filesystem_base{
public:
  explicit FileSystem(size_t size);
  FileSystem(const FileSystem& other) = delete;
  FileSystem(FileSystem&& other) = delete;
  ~FileSystem() = default;
  FileSystem& operator=(const FileSystem& other) = delete;
  FileSystem& operator=(const FileSystem&& other) = delete;


  const fuse_lowlevel_ops& ops() const {
    return ops_;
  }

  // Fuse operations
public:






  // file handle operation
public:
  int create(fuse_ino_t parent_ino,const std::string& name,mode_t mode,int flags,uid_t uid,gid_t gid, fuse_req_t req,struct fuse_file_info* fi) override;
  int getattr(fuse_ino_t ino,  uid_t uid, gid_t gid, fuse_req_t req, int *ptr) override;
  ssize_t readdir(fuse_req_t req, fuse_ino_t ino,  size_t bufsize, off_t off) override;
  int mkdir(fuse_ino_t parent_ino,const std::string& name,mode_t mode, uid_t uid,gid_t gid, fuse_req_t req) override;
  //int mknod(fuse_ino_t parent_ino,const std::string& name,mode_t mode,dev_t rdev,struct stat* st,uid_t uid,gid_t gid);
  int access(fuse_ino_t ino, int mask, uid_t uid, gid_t gid,fuse_req_t req) override;
  int setattr(fuse_ino_t ino,FileHandle* fh,struct stat* x,int to_set,uid_t uid,gid_t gid,fuse_req_t req) override;
  int lookup(fuse_ino_t parent_ino, const std::string& name,fuse_req_t req) override;
  void init(void *userdata,struct fuse_conn_info *conn) override;
  int open(fuse_ino_t ino, int flags, FileHandle** fhp, uid_t uid, gid_t gid,  struct fuse_file_info* fi,fuse_req_t req) override;
  ssize_t write(FileHandle* fh, const char * buf, size_t size, off_t off, struct fuse_file_info *fi,fuse_req_t req,char * ptr, fuse_ino_t ino) override;
  ssize_t read(FileHandle* fh, off_t offset, size_t size,fuse_req_t req, fuse_ino_t ino) override;
  int unlink(fuse_ino_t parent_ino, const std::string& name, uid_t uid, gid_t gid,fuse_req_t req) override;
  int rmdir(fuse_ino_t parent_ino, const std::string& name, uid_t uid, gid_t gid,fuse_req_t req) override;
  void forget(fuse_ino_t ino, long unsigned nlookup,fuse_req_t req) override;
  int rename(fuse_ino_t acq_old_parent_in,const std::string& oldname,fuse_ino_t newparent_ino,const std::string& dupl_reg_in,uid_t uid,gid_t dupl_dir_in,fuse_req_t req) override;
  int opendir(fuse_ino_t ino, int flags, uid_t uid, gid_t gid,fuse_req_t req,struct fuse_file_info *fi) override;
  void release(fuse_ino_t ino, FileHandle* fh) override;
  void my_fallocate(fuse_req_t req, fuse_ino_t ino, int mode,off_t offset, off_t length, fuse_file_info *fi) override;
  // helper functions
private:
  void replace_entry_same_parent(uint64_t  uid,cown_ptr<DirInode> old_parent_inode,acquired_cown<DirInode> &acq_old_parent_in,std::string newname, std::map<std::basic_string<char>, uint64_t>::iterator old_entry,fuse_req_t req );
  void replace_entry(cown_ptr<DirInode> old_parent_inode,cown_ptr<DirInode> new_parent_inode,uint64_t  uid,acquired_cown<DirInode> &acq_old_parent_in,acquired_cown<DirInode> &acq_new_parent_in,std::string newname,std::map<std::basic_string<char>, uint64_t>::iterator old_entry,fuse_req_t req);
  void free_space(acquired_cown<Block> &blk);
  int truncate(acquired_cown<RegInode> &in, off_t newsize, uid_t uid, gid_t gid);

  void add_inode(cown_ptr<DirInode> inode,struct stat st);
  void add_inode(cown_ptr<RegInode> inode,struct stat st);

  static void get_inode(const cown_ptr<RegInode>& inode_cown, acquired_cown<RegInode> &acq_inode,  acquired_cown<std::unordered_map<fuse_ino_t, cown_ptr<RegInode>>> &regular_inode_table);
  static void get_inode(const cown_ptr<DirInode>& in_cown, acquired_cown<DirInode> &acq_inode,  acquired_cown<std::unordered_map<fuse_ino_t, cown_ptr<DirInode>>> &dir_inode_table);



  // private fields;
  cown_ptr<std::unordered_map<fuse_ino_t, cown_ptr<RegInode>>> regular_inode_table = make_cown<std::unordered_map<fuse_ino_t, cown_ptr<RegInode>>>();
  cown_ptr<std::unordered_map<fuse_ino_t, cown_ptr<DirInode>>> dir_inode_table = make_cown<std::unordered_map<fuse_ino_t, cown_ptr<DirInode>>>();

private:
  int access(struct stat st, int mask, uid_t uid, gid_t gid);
  static void init_stat(fuse_ino_t ino, time_t time, uid_t uid, gid_t gid, blksize_t blksize, mode_t mode,struct stat *i_st,bool is_regular);

private:
  std::atomic<fuse_ino_t> next_ino_;
  //FIXME, SHOULD BE ATOMIC
  std::atomic<size_t> avail_bytes_;
  struct statvfs stat;
  cown_ptr<DirInode> root;

};

#endif // VERONA_RT_ALL_FILESYSTEM_H
