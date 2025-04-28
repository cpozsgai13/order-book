#ifndef _BASE_PRODUCER_H_
#define _BASE_PRODUCER_H_

template<typename Derived>
class BaseProducer {
public:
    BaseProducer() = default;
    virtual ~BaseProducer() = default;

    bool post() {
        return static_cast<Derived *>(this)->postMessage();
    }
};

#endif