#pragma once
#include <vector>


namespace toaster {

template<typename T_FROM, typename T_TO = T_FROM>
class LinearCalibrate {
public:
  typedef struct _DATA_POINT {
    T_FROM data_from;
    T_TO data_to;
  } DATA_POINT;

public:
  LinearCalibrate() {
  }

  void clear() {
    _data_points.clear();
  }

  size_t size() const {
    return _data_points.size();
  }

  void pushDataPoint(DATA_POINT dp) {
    _data_points.push_back(dp);
    sortDataPoints();
  }

  void pushDataPoint(T_FROM data_from, T_TO data_to) {
    DATA_POINT dp;
    dp.data_from = data_from;
    dp.data_to = data_to;
    _data_points.push_back(dp);
    sortDataPoints();
  }

  bool getValue(T_FROM data_from, T_TO& data_to, bool nofail = false) const {
    if (_data_points.size() < 2) {
      return false;
    }

    const size_t data_points_size = _data_points.size();

    if (nofail) {
      if (data_from < _data_points.front().data_from) {
        data_to = _data_points.front().data_to;
        return true;
      }
      if (data_from > _data_points.back().data_from) {
        data_to = _data_points.back().data_to;
        return true;
      }
    }
    else {
      if (data_from < _data_points.front().data_from || data_from > _data_points.back().data_from) {
        return false;
      }
    }

    // 1. find value exactly
    for (const auto& it : _data_points) {
      if (it.data_from == data_from) {
        data_to = it.data_to;
        return true;
      }
    }

    // 2. find nearest
    T_FROM nearest_lower = _data_points[0].data_from;
    T_TO nearest_lower_value = _data_points[0].data_to;
    for (int i = 1; i < data_points_size; i++) {
      if (_data_points[i].data_from > data_from) continue;

      if ((data_from - nearest_lower) >= (data_from - _data_points[i].data_from)) {
        nearest_lower = _data_points[i].data_from;
        nearest_lower_value = _data_points[i].data_to;
      }
    }

    T_FROM nearest_upper = _data_points[data_points_size - 1].data_from;
    T_TO nearest_upper_value = _data_points[data_points_size - 1].data_to;
    for (int i = data_points_size - 1; i >= 0; i--) {
      if (_data_points[i].data_from < data_from) continue;

      if ((nearest_upper - data_from) >= (_data_points[i].data_from - data_from)) {
        nearest_upper = _data_points[i].data_from;
        nearest_upper_value = _data_points[i].data_to;
      }
    }

    // linear
    data_to = ((data_from - nearest_lower) * nearest_upper_value + (nearest_upper - data_from) * nearest_lower_value) / (nearest_upper - nearest_lower);
    return true;
  }

protected:
  std::vector<DATA_POINT> _data_points;

  void sortDataPoints() {
    std::stable_sort(_data_points.begin(), _data_points.end(), [](DATA_POINT a, DATA_POINT b) {
      return a.data_from < b.data_from;
      });
  }
};

};
