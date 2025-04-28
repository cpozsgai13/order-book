#ifndef _MD_FILE_READER_H_
#define _MD_FILE_READER_H_

#include <string>
#include "CoreMessages.h"

namespace MarketData 
{

class FileReader {
public:
    FileReader() = default;
    ~FileReader() = default;

    bool loadFile(const std::string& path, std::vector<Packet>& packets);
};

}

#endif