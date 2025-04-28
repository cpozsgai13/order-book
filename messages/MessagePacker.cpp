#include "MessagePacker.h"

namespace MarketData 
{

size_t MessagePacker::size() {
    return msg_size;
}

bool MessagePacker::addMessage(CoreMessage&& message) {
    return false;
}

bool MessagePacker::addMessage(const CoreMessage& message) {
    return false;
}

}
