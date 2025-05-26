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

    std::vector<Packet> loadDataFile(const std::string& path);
    bool loadSymbolFile(const std::string& path, std::vector<Symbol>& symbols);

private:
    bool parseSymbol(const std::string& line, Symbol& sym);
};

}

#endif