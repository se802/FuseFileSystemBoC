#pragma once
#define FUSE_USE_VERSION 35

#include "cpp/when.h"
#include "debug/harness.h"

#include <atomic>
#include <cassert>
#include <csignal>
#include <cstring>
#include <fuse3/fuse_lowlevel.h>
#include <linux/limits.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

using namespace verona::cpp;
struct FileHandle;



struct filesystem_opts {
  size_t size;
  bool debug;
};

enum {
  KEY_HELP,
};

#define FS_OPT(t, p, v)                                                        \
    { t, offsetof(struct filesystem_opts, p), v }

static struct fuse_opt fs_fuse_opts[] = {
  FS_OPT("size=%llu", size, 0),
  FS_OPT("-debug", debug, 1),
  FUSE_OPT_KEY("-h", KEY_HELP),
  FUSE_OPT_KEY("--help", KEY_HELP),
  FUSE_OPT_END};


static void usage(const char* progname) {
  printf("file ssystem options:\n"
    "    -o size=N          max file system size (bytes)\n"
    "    -debug             turn on verbose logging\n");
}



static int fs_opt_proc(void* data, const char* arg, int key, struct fuse_args* outargs) {
  switch (key) {
    case FUSE_OPT_KEY_OPT:
      return 1;
    case FUSE_OPT_KEY_NONOPT:
      return 1;
    case KEY_HELP:
      usage(NULL);
      exit(1);
    default:
      assert(0);
      exit(1);
  }
}
