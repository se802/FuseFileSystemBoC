// Created by csdeptucy on 7/8/23.

#ifndef VERONA_RT_ALL_BLOCK_H
#define VERONA_RT_ALL_BLOCK_H

#include <cstdio>
#include <cstring>
#include <memory>

// The Block struct represents a block of data in the filesystem.
struct Block {
    // Constructor for creating a Block instance with a given size and block number.
    explicit Block(size_t size, int block_number)
            : size(size), block_number(block_number), buf(new char[size]) {
        // Initialize the buffer with zeroes.
        memset(buf.get(), 0, size);
    }

    // Size of the block buffer in bytes.
    size_t size;

    // Block number indicating the position of this block in the filesystem.
    int block_number = -1;

    // Unique pointer to a char array that serves as the buffer for the block's data.
    std::unique_ptr<char[]> buf;

    // Write data to the block's buffer.
    // Parameters:
    //   - data: A pointer to the source data that needs to be written to the block.
    //   - bytes_to_write: The number of bytes to write from the source data.
    //   - block_offset: The offset within the block's buffer where the data should be written.
    //   - bytes_written: The number of bytes already written to the block, used for positioning the data within the buffer.
    void write(const char* data, uint64_t bytes_to_write, uint64_t block_offset, uint64_t bytes_written) const;

    // Read data from the block's buffer.
    // Parameters:
    //   - data: A pointer to the destination buffer where the read data will be stored.
    //   - bytes_to_read: The number of bytes to read from the block's buffer.
    //   - block_offset: The offset within the block's buffer where the read should start.
    //   - bytes_read: The number of bytes already read from the block, used for positioning the read data within the destination buffer.
    void read(char* data, uint64_t bytes_to_read, uint64_t block_offset, uint64_t bytes_read) const;
};

#endif // VERONA_RT_ALL_BLOCK_H
