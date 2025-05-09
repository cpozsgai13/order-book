#ifndef _MULTICAST_PRODUCER_H_
#define _MULTICAST_PRODUCER_H_

#include "CoreMessages.h"
class MulticastProducer {
public:
    MulticastProducer();
    ~MulticastProducer();
    bool postMessage();

    bool run();

    void stop();
};

#endif