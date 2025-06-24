#include "Order.h"
#include <iostream>
#include <iomanip>

namespace MarketData {

std::ostream& operator<<(std::ostream& os, const FixedPrecisionPrice<Price, 6>& price) {
  os << std::setprecision(price.numPlaces() + price.numDigits())  << (double)price;
  return os;
};

std::ostream& PrintBid(std::ostream& os, const Volume& vol, const FixedPrecisionPrice<Price, 6>& price, int precision) {
  int pad_left = 5;
  std::string pad(pad_left, ' ');

  os << pad << std::fixed << std::setw(6) << std::setfill(' ') << std::right << (int)vol << " " << std::setprecision(precision) <<  std::setw(7) << std::setfill(' ') << std::right << (double)price;
  return os;
}

std::ostream& PrintAsk(std::ostream& os, const Volume& vol, const FixedPrecisionPrice<Price, 6>& price, int precision) {
  os << std::setprecision(precision) << std::setw(7) << std::setfill(' ') << std::right << (double)price << " " << std::setw(6) << std::setfill(' ') << std::right << (int)vol;
  return os;
}

}
