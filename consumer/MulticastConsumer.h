#ifndef _MULTICAST_CONSUMER_H_
#define _MULTICAST_CONSUMER_H_

#include "BaseConsumer.h"
class MulticastConsumer: public BaseConsumer<MulticastConsumer> {
public:
    MulticastConsumer();
    ~MulticastConsumer();
    bool receiveMessage() {
        return false;
    }
};

#endif