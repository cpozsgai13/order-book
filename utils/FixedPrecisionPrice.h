#ifndef _FIXED_PRECISION_PRICE_H_
#define _FIXED_PRECISION_PRICE_H_

#include <cmath>

template<int V>
int NextPowerOfTwo(int value = V) {
    int exponent = 0;
    while(V > 0) {
        ++exponent;
    }
    return 1 << exponent;
}
template<typename T, int exponent>
struct Power {
    Power(T base):
    value(base)
    {
    }

    T operator()() {
        return calculate();
    }
    constexpr T calculate(int exp = exponent) {
        if(exp > 1) {
            return value*calculate(exp - 1);
        } else {
            return value;
        }
    }
    T value;
};


//  Representation of a price with a fixed number
//  of decimal places.  
//  For example, to represent the price 100.0243
//  FixedPrice<uint32_t, 4> price(1000243)
//template<typename T = uint32_t, uint8_t Places = 4>
template<typename T, int Places>
class FixedPrecisionPrice {
public:
    FixedPrecisionPrice(T t): 
    value(t),
    places(Places) {
        divisor = Power<T, Places>(10)();
    }

    FixedPrecisionPrice(double d): 
    places(Places) {
        divisor = Power<T, Places>(10)();
        //  We need to know how many places to pad.

        T raw = static_cast<T>(d*divisor);
        value = raw;
    }

    operator double() {
        return (double)value/divisor;
    }

    T rawValue() const {
        return value;
    }
private:
    T value{0};
    uint8_t places{0};
    T divisor{0};
};

#endif