//
// Created by csdeptucy on 7/8/23.
//

#ifndef VERONA_RT_ALL_MY_FUSE_LOOP_H
#define VERONA_RT_ALL_MY_FUSE_LOOP_H


#define FUSE_USE_VERSION 34
#include "liburing.h"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <fuse3/fuse_lowlevel.h>
#include <fuse3/fuse_i.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <unordered_map>
#include <signal.h>
#include <ctime>
#include <iostream>

#define QUEUE_DEPTH 8
#define BATCH_SIZE 4

extern std::unordered_map<uint64_t, time_t> write_time;
extern std::unordered_map<uint64_t, time_t> read_time;

struct my_fuse_in_header {
    uint32_t len;
    uint32_t opcode;
    uint64_t unique;
    uint64_t nodeid;
    uint32_t uid;
    uint32_t gid;
    uint32_t pid;
    uint16_t total_extlen; /* length of extensions in 8byte units */
    uint16_t padding;
};

int my_fuse_session_receive_buf_int(struct fuse_session *se, struct fuse_buf *buf,
                                    struct fuse_chan *ch, struct io_uring *ring);

int my_fuse_session_receive_buf_int(struct fuse_session *se, struct fuse_buf *buf);

void write_request_to_log(FILE *log_file, const void *request, size_t request_len);

int read_request_from_log(FILE *log_file, void **request, size_t *request_len);

void sigint_handler(int signum);

int my_fuse_session_loop(struct fuse_session *se);

int my_fuse_session_loop_uring(struct fuse_session *se);

#endif // VERONA_RT_ALL_MY_FUSE_LOOP_H
