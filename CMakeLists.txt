cmake_minimum_required(VERSION 3.25)
project(FuseFileSystemBoC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)
add_subdirectory(verona-rt)

#include_directories(/usr/local/include/fuse3)

add_executable(FuseFileSystemBoC main.cpp Block.cpp DirInode.cpp FileSystem.cpp Inode.cpp RegInode.cpp)

target_include_directories(FuseFileSystemBoC
        PUBLIC verona-rt/src/rt/
        PUBLIC /usr/local/include/fuse3)

target_link_libraries(FuseFileSystemBoC
        PUBLIC snmalloc
        PUBLIC uring
        PUBLIC fuse3)
