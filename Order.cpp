#include "Order.h"
#include <iostream>
#include <iomanip>

namespace MarketData {

std::ostream& operator<<(std::ostream& os, const Price& price) {
  os << std::setprecision(price.numPlaces() + price.numDigits())  << (double)price;
  return os;
};

std::ostream& PrintBid(std::ostream& os, const Volume& vol, const Price& price, int precision) {
  int pad_left = 5;
  std::string pad(pad_left, ' ');

  os << pad << std::fixed << std::setw(6) << std::setfill(' ') << std::right << vol << " " << std::setprecision(precision) <<  std::setw(7) << std::setfill(' ') << std::right << (double)price;
  return os;
}

std::ostream& PrintAsk(std::ostream& os, const Volume& vol, const Price& price, int precision) {
  os << std::setprecision(precision) << std::setw(7) << std::setfill(' ') << std::right << (double)price << " " << std::setw(6) << std::setfill(' ') << std::right << vol;
  //os << std::setw(6) << std::setfill(' ') << std::right << (double)price << " " << std::setw(6) << std::setfill(' ') << std::right << vol;
  return os;
}

}
