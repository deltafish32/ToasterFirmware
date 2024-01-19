// #include <Arduino.h>
#include "random.h"

namespace toaster {

std::random_device Random::_rd{};
std::mt19937 Random::_gen{_rd()};
std::normal_distribution<> Random::_dist(0, 2);

};
