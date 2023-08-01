#include "main.h"                 // Main header file (presumably for the current project)


#include <cassert>                // Standard C assert macro

#include <cstdio>                 // Standard input/output operations
#include <cstdlib>                // Standard library functions

// FUSE Libraries
#include "my_fuse_loop.h"         // Custom FUSE loop implementation
#include <fuse3/fuse_opt.h>       // FUSE library options

// Filesystem Data Structures
#include "FileSystem.h"

#define FUSE_VERSION 34

using namespace verona::cpp;



#define PATH "/home/sevag/fuse_boc"


short CorePin(int coreID)
{
  short status=0;
  int nThreads = std::thread::hardware_concurrency();
  //std::cout<<nThreads;
  cpu_set_t set;
  std::cout<<"\nPinning to Core:"<<coreID<<"\n";
  CPU_ZERO(&set);

  if(coreID == -1)
  {
    status=-1;
    std::cout<<"CoreID is -1"<<"\n";
    return status;
  }

  if(coreID > nThreads)
  {
    std::cout<<"Invalid CORE ID"<<"\n";
    return status;
  }

  CPU_SET(coreID,&set);
  if(sched_setaffinity(0, sizeof(cpu_set_t), &set) < 0)
  {
    std::cout<<"Unable to Set Affinity"<<"\n";
    return -1;
  }
  return 1;
}


void start_dispatcher(int argc, char* argv[], SystematicTestHarness* harness) {
  when() << [=]() {
    Scheduler::add_external_event_source();
    harness->external_thread([=]() {
      auto x = CorePin(40);
      if(x!=1)
        exit(1);

      setbuf(stdout, 0);
      int returnValue = system("fusermount -u " PATH);

      if (returnValue == -1) {
        // Error executing the command
        perror("system");
        exit(EXIT_FAILURE);
      }

      struct filesystem_opts opts;
      // option defaults

      opts.size = 512 << 20;
      opts.debug = false;
      struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
      if (fuse_opt_parse(&args, &opts, fs_fuse_opts, fs_opt_proc) == -1) {
        exit(1);
      }

      assert(opts.size > 0);

      const char* mountpoint = PATH;

      FileSystem fs(opts.size);

      auto se = fuse_session_new(&args, &fs.ops(), sizeof(fs.ops()), &fs);
      fuse_set_signal_handlers(se);
      fuse_session_mount(se, mountpoint);
      fuse_daemonize(true);

      fuse_session_loop_uring(se);

      fuse_session_unmount(se);
      fuse_remove_signal_handlers(se);
      fuse_session_destroy(se);
    });
  };
}






int main(int argc, char *argv[]) {


  SystematicTestHarness harness(argc, argv);


  Scheduler& sched = Scheduler::get();
  sched.init(10);
  start_dispatcher(argc, argv, &harness);  // Pass the validated path argument

  sched.run();

  while (!harness.external_threads.empty()) {
    auto& thread = harness.external_threads.front();
    thread.join();
    harness.external_threads.pop_front();
  }

  Logging::cout() << "External threads joined" << std::endl;

  LocalEpochPool::sort();

  if (harness.detect_leaks)
    snmalloc::debug_check_empty<snmalloc::Alloc::Config>();

  return 0;
}
