#ifndef _MULTICAST_CONSUMER_H_
#define _MULTICAST_CONSUMER_H_

class MulticastConsumer {
public:
    MulticastConsumer();
    ~MulticastConsumer();
    bool receiveMessage() {
        return false;
    }
};

#endif