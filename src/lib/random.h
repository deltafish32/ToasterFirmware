#pragma once
#include <cmath>
#include <random>


namespace toaster {

class Random {
public:
  Random() {
  }

public:
  static int random(int n) {
    // srand(micros());
    // return rand() % n;

    std::uniform_int_distribution<int> dist(0, n - 1);
    return dist(_gen);
  }

  static double grandom() {
    return _dist(_gen);
  }

  static int gaussianRandom(int n) {
    static const double RMIN = -8;
    static const double RMAX = 8;

    double r = grandom();
    r = std::min(std::max(r, RMIN), RMAX);
    double r1 = (r - RMIN) / (RMAX - RMIN);
    return (int)(r1 * n);
  }

protected:
  static std::random_device _rd;
  static std::mt19937 _gen;
  static std::normal_distribution<> _dist;

};

};
