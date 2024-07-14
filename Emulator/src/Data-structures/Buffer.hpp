/*
Copyright (©) 2023  Frosty515

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _BUFFER_HPP
#define _BUFFER_HPP

#include <stddef.h>
#include <stdint.h>

#include "LinkedList.hpp"

#define DEFAULT_BUFFER_BLOCK_SIZE 64

// A dynamic buffer created from multiple blocks
class Buffer {
public:
    Buffer();
    Buffer(size_t size, size_t blockSize = DEFAULT_BUFFER_BLOCK_SIZE);
    ~Buffer();

    // Write size bytes from data to the buffer at offset
    void Write(uint64_t offset, const uint8_t* data, size_t size);

    // Read size bytes from the buffer at offset to data
    void Read(uint64_t offset, uint8_t* data, size_t size) const;

    // Clear size bytes starting at offset. Potentially could remove the block if it is empty
    void Clear(uint64_t offset, size_t size);

    // Remove any unused blocks at the end
    void AutoShrink();

    // Clear the buffer until offset. Will delete any empty blocks. Returns the number of blocks deleted.
    uint64_t ClearUntil(uint64_t offset);

    // Get the size of the buffer
    size_t GetSize() const;

private:
    struct Block {
        uint8_t* data;
        size_t size;
        bool empty;
    };
    
    Block* AddBlock(size_t size);
    void DeleteBlock(uint64_t index);

private:
    size_t m_size;
    size_t m_blockSize;
    LinkedList::SimpleLinkedList<Block> m_blocks;
};

#endif /* _BUFFER_HPP */