#ifndef _FAST_FILE_READER_H_
#define _FAST_FILE_READER_H_

#include <string>
#include <sys/mman.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

class FastFileReader {
public:
    explicit FastFileReader(const std::string& path) {
        fd = open(path.c_str(), O_RDONLY);
    }
    ~FastFileReader() {

        munmap(mapped_file, file_size);
        close(fd);
        //std::cout << "FastFileReader dtor" << std::endl;
    }
    FastFileReader(const FastFileReader&) = delete;

    // bool open(const std::string& path) {
    //     int fd = open(path.c_str(), O_RDONLY);
    //     if(fd == -1) {
    //         return false;
    //     }


    //     return true;
    // }

    bool initialize() {
        struct stat file_stat;
        if(fstat(fd, &file_stat) == -1) {
            return false;
        }

        file_size = file_stat.st_size;

        mapped_file = mmap(nullptr, file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if(mapped_file == MAP_FAILED) {
            return false;
        }

        file_cursor = (char *)mapped_file;
        file_end = file_cursor + file_stat.st_size;

        return true;
    }

    bool getNextLine(std::string& line) {
        if(!mapped_file) {
            initialize();
        }
        if(file_cursor < file_end) {
            char *nl = static_cast<char*>(memchr(file_cursor, '\n', file_end - file_cursor));
            if(!nl) {
                nl = file_end;
            }
            line.assign(file_cursor, nl);
            file_cursor = nl + 1;
            return true;
        }

        return false;
    }
private:
    int fd;

    void *mapped_file{nullptr};
    long file_size{0};
    char *file_cursor{nullptr};
    char *file_end{nullptr};
};

#endif