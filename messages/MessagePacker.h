#ifndef _MESSAGE_PACKER_H_
#define _MESSAGE_PACKER_H_

#include "CoreMessages.h"

namespace MarketData
{

template<size_t SZ = 1500>
struct MessagePacket {

    char buffer[SZ] = {0};
};

class MessagePacker {
public:
    MessagePacker() = default;
    ~MessagePacker() = default;
    
    size_t size();
    bool addMessage(CoreMessage&& message);

    bool addMessage(const CoreMessage& message);

private:
    size_t msg_size{0};

};

}

#endif