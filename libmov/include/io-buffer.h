/*
 * @Author: kay1ess 
 * @Date: 2021-10-18 23:20:03 
 * @Last Modified by: kay1ess
 * @Last Modified time: 2021-10-19 00:33:54
 */

#ifndef _io_buffer_h_
#define _io_buffer_h_

#include "mov-buffer.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

class io_buffer
{
// #define _DUMP_IO_BUFFER_
#ifdef _DUMP_IO_BUFFER_
public:
    static int _io_buffer_count_;
    static int64_t _io_buffer_size_;
#endif

public:
    io_buffer(size_t capacity)
    {
        buffer_ = (uint8_t *)::malloc(capacity);
        ref_count_ = 1;
        capacity_ = capacity;
        buffer_end_ = buffer_ + capacity;
        read_ptr_ = buffer_;
        write_ptr_ = buffer_;
        total_moved_ = 0;

#ifdef _DUMP_IO_BUFFER_
        _io_buffer_count_++;
        _io_buffer_size_ += capacity;
#endif
    }

    ~io_buffer()
    {
        if (buffer_ != NULL)
            ::free(buffer_);

#ifdef _DUMP_IO_BUFFER_
        _io_buffer_count_--;
        _io_buffer_size_ -= capacity_;
        printf("io-buffer: %d count, %lld bytes\n", _io_buffer_count_, _io_buffer_size_);
#endif
    }

    inline io_buffer* aquire()
    {
        ref_count_++;
        return this;
    }

    inline void release()
    {
        assert(ref_count_ > 0);
        if (--ref_count_ == 0)
            delete this;
    }

    inline void clear()
    {
        read_ptr_ = buffer_;
        write_ptr_ = buffer_;
        total_moved_ = 0;
    }

    int realloc(size_t capacity)
    {
        size_t read_offset = read_ptr_ - buffer_;
        size_t write_offset = write_ptr_ - buffer_;
        buffer_ = (uint8_t *)::realloc(buffer_, capacity);
#ifdef _DUMP_IO_BUFFER_
        _io_buffer_size_ -= capacity_;
#endif
        if (buffer_ == NULL)
        {
            capacity_ = 0;
            buffer_end_ = NULL;
            read_ptr_ = write_ptr_ = NULL;
            return -ENOMEM;
        }

#ifdef _DUMP_IO_BUFFER_
        _io_buffer_size_ += capacity;
#endif

        capacity_ = capacity;
        buffer_end_ = buffer_ + capacity;
        read_ptr_ = buffer_ + read_offset;
        write_ptr_ = buffer_ + write_offset;
        return 0;
    }

    inline size_t capacity() const
    {
        return capacity_;
    }

    inline size_t can_read() const
    {
        return write_ptr_ - read_ptr_;
    }

    inline size_t read(void *data, uint64_t bytes)
    {
        assert(read_ptr_ + bytes <= buffer_end_);
        memcpy(data, read_ptr_, bytes);
        read_ptr_ += bytes;
        total_read_ += bytes;
        return 0;
    }

    inline uint8_t* read_ptr() const
    {
        return read_ptr_;
    }

    inline size_t can_write() const
    {
        assert(buffer_end_ >= write_ptr_);
        assert(read_ptr_ >= buffer_);
        return (buffer_end_ - write_ptr_) + (read_ptr_ - buffer_);
    }

    uint8_t* begin_write(size_t bytes)
    {
        if (buffer_ == NULL)
            return NULL;

        if (write_ptr_ + bytes <= buffer_end_)
            return write_ptr_;

        // 说明 read_ptr 前面还有 空间 可以移动到后面 作为写的空间
        if (read_ptr_ > buffer_)
        {
            assert(write_ptr_ >= read_ptr_);
            int moved = (int)(write_ptr_ - read_ptr_);
            if (moved > 0)
                memmove(buffer_, read_ptr_, moved);
            size_t offset = read_ptr_ - buffer_;
            total_moved_ += offset;
            read_ptr_ = buffer_;
            write_ptr_ -= offset;
            if (write_ptr_ + bytes <= buffer_end_)
                return write_ptr_;
        }
        // 如果空间不够 算一下 新的空间    
        size_t new_size = capacity_ + bytes - can_write();
        if (new_size < capacity_ << 1)
            new_size = capacity_ << 1;
        if (realloc(new_size) == 0)
            return write_ptr_;

        return NULL;
    }

    inline void end_write(size_t bytes)
    {
        assert(bytes <= can_write());
        write_ptr_ += bytes;
    }

    int write(const uint8_t* data, uint64_t bytes)
    {
        uint8_t *ptr = begin_write(bytes);
        if (ptr != NULL)
        {
            memcpy(ptr, data, bytes);
            end_write(bytes);
            return 0;
        }
        return -EOVERFLOW;
    }

    inline uint64_t tellw() const
    {
        return total_moved_ + (write_ptr_ - buffer_);
    }

    // 写指针 seek 
    inline int seekw(int64_t offset)
    {
        int64_t off = offset - total_moved_;
        assert(off >= 0 && off < (int64_t)capacity_);
        if (off >= 0 && off < (int64_t)capacity_)
        {
            write_ptr_ = buffer_ + off;
            return 0;
        }
        return -EINVAL;
    }

    inline int64_t tellr()
    {
        return total_read_;
    }

    inline int seekr(int64_t offset)
    {
        int64_t off = offset - total_read_;
        if ((off >= 0 && (can_read() >= off)) || (off < 0 && read_ptr_ - buffer_ > -off))
        {
            read_ptr_ += off;
            total_read_ += off;
            return 0;
        }
        return -EINVAL;
    }

private:
    int ref_count_;
    size_t capacity_;
    uint8_t *buffer_;
    uint8_t *buffer_end_;
    uint8_t *read_ptr_;
    uint8_t *write_ptr_;
    uint64_t total_moved_;
    uint64_t total_read_;
};


int io_buffer_read(void *param, void *data, uint64_t bytes)
{
    return ((io_buffer*)param)->read((uint8_t*)data, (size_t)bytes);
}

int io_buffer_seek(void *param, int64_t offset)
{
    return ((io_buffer *)param)->seekr(offset);
}

int64_t io_buffer_tell(void *param)
{
    return ((io_buffer *)param)->tellr();
}

struct mov_buffer_t buffer_reader =
{
    io_buffer_read,
    NULL,
    io_buffer_seek,
    io_buffer_tell,
};


#ifdef __cplusplus
}
#endif

#endif