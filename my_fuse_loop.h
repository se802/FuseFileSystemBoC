//
// Created by csdeptucy on 7/8/23.
//

#ifndef VERONA_RT_ALL_MY_FUSE_LOOP_H
#define VERONA_RT_ALL_MY_FUSE_LOOP_H



#include "liburing.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <fuse3/fuse_lowlevel.h>
#include <fuse3/fuse_i.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define QUEUE_DEPTH 8
#define BATCH_SIZE 4






int my_fuse_session_receive_buf_int(struct fuse_session *se, struct fuse_buf *buf,
                                    struct fuse_chan *ch,struct io_uring *ring)
{
  for(int i=0;i<BATCH_SIZE;i++){
    if (!buf[i].mem) {
      buf[i].mem = malloc(se->bufsize);
      if (!buf[i].mem) {
        printf("fuse: failed to allocate read buffer\n");
        exit(1);
      }
    }
  }

  for(int i=0;i<BATCH_SIZE;i++){
    // Prepare the read request using liburing
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    io_uring_prep_read(sqe,  se->fd, buf[i].mem, se->bufsize, 0);
    sqe->flags |= IOSQE_ASYNC;
    sqe->flags |= IOSQE_IO_LINK;

    io_uring_sqe_set_data(sqe, &buf[i]);
  }
  io_uring_submit(ring);
  return 0;
}







int my_fuse_session_loop_uring(struct fuse_session *se)
{
  struct io_uring ring;
  int ret = io_uring_queue_init(QUEUE_DEPTH, &ring, 0);
  if (ret < 0) {
    fprintf(stderr, "io_uring_queue_init error: %s\n", strerror(-ret));
    exit(1);
  }

  int res = 0;
  struct fuse_buf fbuf[BATCH_SIZE]{};
  for(auto & i : fbuf)
    i.mem = NULL;




  while (!fuse_session_exited(se)) {
    // Use liburing to read data into fbuf
    my_fuse_session_receive_buf_int(se, fbuf, NULL,&ring);

    for (int i = 0; i < BATCH_SIZE; i++)
    {
      // Wait for completion event
      struct io_uring_cqe *cqe;
      int ret = io_uring_wait_cqe(&ring, &cqe);

      if (ret < 0) {
        if (ret == -EINTR || ret == -EINVAL) {
          printf("No pending requests");
        }
        fprintf(stderr, "io_uring_wait_cqe error: %s\n", strerror(-ret));
        exit(1);
      }

      // Process the completion event
      if (cqe->res >= 0) {
        fbuf[0].size = cqe->res;
        fuse_session_process_buf(se, &fbuf[0]);
      } //else {
        //fprintf(stderr, "Read error: %s\n", strerror(-cqe->res));
      //}

      // Mark the completion event as seen
      io_uring_cqe_seen(&ring, cqe);
    }




  }

  free(fbuf[0].mem);

  // Handle the result and error state of the FUSE session
  if (res > 0) {
    res = 0;  // No error, just the length of the most recently read request
  }
  if (se->error != 0) {
    res = se->error;
  }
  fuse_session_reset(se);

  return res;
}


// NOT USED

#define FUSE_DEV_IOC_MAGIC		229
#define FUSE_DEV_IOC_CLONE		_IOR(FUSE_DEV_IOC_MAGIC, 0, uint32_t)

int clone_device_fd(int fd,struct fuse_session *se){
  const char *devname = "/dev/fuse";
  int clonefd = open(devname, O_RDWR | O_CLOEXEC);
  fcntl(clonefd, F_SETFD, FD_CLOEXEC);
  int masterfd = se->fd;
  ioctl(clonefd, FUSE_DEV_IOC_CLONE, &masterfd);
  return clonefd;
}


int my_fuse_session_receive_buf_int(struct fuse_session *se, struct fuse_buf *buf,
                                    struct fuse_chan *ch)
{
  ssize_t res;
  if (!buf->mem) {
    buf->mem = malloc(se->bufsize);
  }
  res = read(ch ? ch->fd : se->fd, buf->mem, se->bufsize);
  buf->size = res;
  return res;
}


#endif // VERONA_RT_ALL_MY_FUSE_LOOP_H
