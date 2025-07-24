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

template<typename T, int Exp>
struct Power {
    static constexpr T value = 10 * Power<T, Exp - 1>::value;
};

template<typename T>
struct Power<T, 0> {
    static constexpr T value = 1;
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
    FixedPrecisionPrice() :
    value(0), 
    divisor(Power<T, Places>::value),
    places(Places) {}

    FixedPrecisionPrice(T t): 
    value(t),
    divisor(Power<T, Places>::value),
    places(Places) {
    }

    FixedPrecisionPrice(double d): 
    divisor(Power<T, Places>::value),
    places(Places) {
        value = static_cast<T>(d*divisor);
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
    T divisor{0};
    uint8_t places{0};
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