//
// Created by csdeptucy on 7/8/23.
//

#include "Inode.h"

bool Inode::is_regular() const { return i_st.st_mode & S_IFREG; }

//bool Inode::is_directory() const { return i_st.st_mode & S_IFDIR; }

//bool Inode::is_symlink() const { return i_st.st_mode & S_IFLNK; }

Inode::~Inode() {
  std::cout << "hey" << std::endl;
}