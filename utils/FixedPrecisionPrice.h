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
#pragma pack(push, 1)
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
        value = static_cast<T>(d*divisor);
    }

    FixedPrecisionPrice(const FixedPrecisionPrice& other) {
        *this = other;
    }

    FixedPrecisionPrice& operator=(const FixedPrecisionPrice& other) {
        value = other.value;
        divisor = Power<T, Places>(10)();
        places = Places;
        return *this;
    }

    bool operator==(const FixedPrecisionPrice& other) {
        return value == other.value && places == other.places;
    }

    operator double() const {
        return (double)value/divisor;
    }

    T rawValue() const {
        return value;
    }

    bool operator<(const FixedPrecisionPrice<T, Places>& other) const {
        return value < other.value;
    }

    uint8_t numPlaces() const {
        return places;
    }

    uint8_t numDigits() const {
        auto norm_value = value/divisor;
        if(norm_value > 0) {
            return (uint8_t)(1 + log10(value/divisor));
        }
        return 0;
    }
private:
    T value{0};
    uint8_t places{0};
    T divisor{0};
};
#pragma pack(pop)

static constexpr size_t FPSZ = sizeof(FixedPrecisionPrice<uint64_t, 2>);

template<typename T, int Places>
struct KeyHash {
    size_t operator()(const FixedPrecisionPrice<T, Places>& key) const {
        return std::hash<T>()(key.rawValue());
    }
};

#endif