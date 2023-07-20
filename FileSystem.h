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

struct FileHandle {
  cown_ptr<RegInode> in;
  int flags;

  FileHandle(cown_ptr<RegInode> in, int flags)
  : in(in)
    , flags(flags) {}


};

class filesystem_base {
public:
  filesystem_base() {
    std::memset(&ops_, 0, sizeof(ops_));
    ops_.init = ll_init,
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
  virtual int lookup(fuse_ino_t parent_ino, const std::string& name,fuse_req_t req) = 0;
  virtual void forget(fuse_ino_t ino, long unsigned nlookup,fuse_req_t req) = 0;
  virtual void init(void *userdata,struct fuse_conn_info *conn) = 0;
  virtual int rename(fuse_ino_t parent_ino,const std::string& name,fuse_ino_t newparent_ino,const std::string& newname,uid_t uid,gid_t gid,fuse_req_t req)= 0;
  virtual int unlink(fuse_ino_t parent_ino, const std::string& name, uid_t uid, gid_t gid,fuse_req_t req) = 0;
  virtual int access(fuse_ino_t ino, int mask, uid_t uid, gid_t gid, fuse_req_t req) = 0;
  virtual int getattr(fuse_ino_t ino, uid_t uid, gid_t gid,fuse_req_t req,int *ptr) = 0;
  virtual int setattr(fuse_ino_t ino,FileHandle* fh,struct stat* attr,int to_set,uid_t uid,gid_t gid,fuse_req_t req)= 0;
  virtual int mkdir(fuse_ino_t parent_ino,const std::string& name,mode_t mode,uid_t uid,gid_t gid, fuse_req_t req)= 0;
  virtual ssize_t readdir(fuse_req_t req, fuse_ino_t ino, size_t bufsize, off_t off)= 0;
  virtual int rmdir(fuse_ino_t parent_ino, const std::string& name, uid_t uid, gid_t gid,fuse_req_t req) = 0;
  virtual int opendir(fuse_ino_t ino, int flags, uid_t uid, gid_t gid,fuse_req_t req,struct fuse_file_info *fi) = 0;
  virtual void my_fallocate(fuse_req_t req, fuse_ino_t ino, int mode,off_t offset, off_t length, fuse_file_info *fi) = 0;
  virtual int create(fuse_ino_t parent_ino,const std::string& name,mode_t mode,int flags,uid_t uid,gid_t gid,fuse_req_t req,struct fuse_file_info* fi)= 0;
  virtual int open(fuse_ino_t ino, int flags, FileHandle** fhp, uid_t uid, gid_t gid,  struct fuse_file_info* fi,fuse_req_t req) = 0;
  virtual ssize_t write(FileHandle* fh, const char * buf, size_t size, off_t off, struct fuse_file_info *fi,fuse_req_t req, char *ptr, fuse_ino_t ino)= 0;
  virtual ssize_t read(FileHandle* fh, off_t offset, size_t size, fuse_req_t req,fuse_ino_t ino) = 0;
  virtual void release(fuse_ino_t ino, FileHandle* fh) = 0;

private:
  static filesystem_base* get(fuse_req_t req) {
    return get(fuse_req_userdata(req));
  }

  static filesystem_base* get(void* userdata) {
    return reinterpret_cast<filesystem_base*>(userdata);
  }

  static void ll_init(void *userdata,struct fuse_conn_info *conn)
  {
    conn->want &= ~FUSE_CAP_AUTO_INVAL_DATA;
    conn->max_background=12;
    if(conn->capable & FUSE_CAP_PARALLEL_DIROPS)
      conn->want |= FUSE_CAP_PARALLEL_DIROPS;

    if(conn->capable & FUSE_CAP_ASYNC_READ)
      conn->want |= FUSE_CAP_ASYNC_READ;

    if(conn->capable & FUSE_CAP_ASYNC_DIO)
      conn->want |= FUSE_CAP_ASYNC_DIO;

    if (conn->capable & FUSE_CAP_SPLICE_WRITE)
      conn->want |= FUSE_CAP_SPLICE_WRITE;

    if (conn->capable & FUSE_CAP_SPLICE_READ)
      conn->want |= FUSE_CAP_SPLICE_READ;

    auto fs = get(userdata);
    fs->init(userdata,conn);
  }



  static void ll_create(
    fuse_req_t req,
    fuse_ino_t parent,
    const char* name,
    mode_t mode,
    struct fuse_file_info* fi) {
    auto fs = get(req);
    const struct fuse_ctx* ctx = fuse_req_ctx(req);
    fs->create(parent, name, mode, fi->flags, ctx->uid, ctx->gid,req,fi);
  }



  static void ll_release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
    auto fs = get(req);
    auto fh = reinterpret_cast<FileHandle*>(fi->fh);
    fs->release(ino, fh);
    fuse_reply_err(req, 0);
  }

  static void ll_unlink(fuse_req_t req, fuse_ino_t parent, const char* name) {
    auto fs = get(req);
    const struct fuse_ctx* ctx = fuse_req_ctx(req);
    fs->unlink(parent, name, ctx->uid, ctx->gid,req);
  }

  static void
  ll_forget(fuse_req_t req, fuse_ino_t ino, long unsigned nlookup) {
    auto fs = get(req);
    fs->forget(ino, nlookup,req);
  }


  static void reply_getattr_success(fuse_req_t req,struct stat st,int code){

  }

  static void
  ll_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
    auto fs = get(req);
    const struct fuse_ctx* ctx = fuse_req_ctx(req);
    fs->getattr(ino, ctx->uid, ctx->gid,req,NULL);
  }

  static void ll_lookup(fuse_req_t req, fuse_ino_t parent, const char* name) {
    auto fs = get(req);
    fs->lookup(parent, name,req);
  }

  static void ll_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
    auto fs = get(req);
    const struct fuse_ctx* ctx = fuse_req_ctx(req);
    fs->opendir(ino, fi->flags, ctx->uid, ctx->gid,req,fi);
  }



  static void ll_readdir(fuse_req_t req,fuse_ino_t ino,size_t size,off_t off,
                         struct fuse_file_info* fi) {
    auto fs = get(req);
    fs->readdir(req, ino,  size, off);
  }


  static void ll_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
    auto fs = get(req);
    const struct fuse_ctx* ctx = fuse_req_ctx(req);
    assert(!(fi->flags & O_CREAT));
    FileHandle* fh;
    fs->open(ino, fi->flags, &fh, ctx->uid, ctx->gid,fi,req);
  }



  static void ll_write (fuse_req_t req, fuse_ino_t ino, const char *buf,size_t size, off_t off, struct fuse_file_info *fi) {

    auto fs = get(req);
    //auto fh = reinterpret_cast<FileHandle*>(fi->fh);

    fs->write(nullptr,buf,size,off,fi,req,NULL,ino);
  }

  static void ll_read(fuse_req_t req,fuse_ino_t ino,size_t size,off_t off,struct fuse_file_info* fi) {
    auto fs = get(req);
    auto fh = reinterpret_cast<FileHandle*>(fi->fh);
    fs->read(fh, off, size, req, ino);
  }



  static void ll_mkdir(fuse_req_t req, fuse_ino_t parent, const char* name, mode_t mode) {
    auto fs = get(req);
    const struct fuse_ctx* ctx = fuse_req_ctx(req);
    fs->mkdir(parent, name, mode, ctx->uid, ctx->gid,req);
  }

  static void ll_rmdir(fuse_req_t req, fuse_ino_t parent, const char* name) {
    auto fs = get(req);
    const struct fuse_ctx* ctx = fuse_req_ctx(req);

    fs->rmdir(parent, name, ctx->uid, ctx->gid,req);
  }

  static void ll_rename(fuse_req_t req,fuse_ino_t parent,const char* name,fuse_ino_t newparent,const char* newname,unsigned int idk) {
    auto fs = get(req);
    const struct fuse_ctx* ctx = fuse_req_ctx(req);
    fs->rename(parent, name, newparent, newname, ctx->uid, ctx->gid,req);
  }

  static void ll_setattr(fuse_req_t req,fuse_ino_t ino,struct stat* attr,int to_set,struct fuse_file_info* fi) {auto fs = get(req);auto fh = fi ? reinterpret_cast<FileHandle*>(fi->fh) : nullptr;const struct fuse_ctx* ctx = fuse_req_ctx(req);
    fs->setattr(ino, fh, attr, to_set, ctx->uid, ctx->gid,req);
  }


  static void ll_access(fuse_req_t req, fuse_ino_t ino, int mask) {
    auto fs = get(req);
    const struct fuse_ctx* ctx = fuse_req_ctx(req);
    fs->access(ino, mask, ctx->uid, ctx->gid, req);
  }


  static void ll_fallocate(fuse_req_t req,fuse_ino_t ino,int mode,off_t offset,off_t length,struct fuse_file_info* fi) {
    auto fs = get(req);
    fs->my_fallocate(req,ino,mode,offset,length,fi);
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
