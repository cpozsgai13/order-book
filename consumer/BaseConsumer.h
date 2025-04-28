#ifndef _BASE_CONSUMER_H_
#define _BASE_CONSUMER_H_

template<typename Derived>
class BaseConsumer {
public:
    BaseConsumer() = default;
    virtual ~BaseConsumer() = default;

    bool receive() {
        return static_cast<Derived *>(this)->receiveMessage();
    }
};

#endif