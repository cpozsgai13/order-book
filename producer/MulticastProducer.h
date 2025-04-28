#ifndef _MULTICAST_PRODUCER_H_
#define _MULTICAST_PRODUCER_H_

#include "BaseProducer.h"
#include "CoreMessages.h"
class MulticastProducer: public BaseProducer<MulticastProducer> {
public:
    MulticastProducer();
    ~MulticastProducer();
    bool postMessage();

    bool run();

    void stop();
};

#endif