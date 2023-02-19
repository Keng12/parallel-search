#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>

namespace kyc {
template <class T> class vector {
  int mSize{};
  int mCapacity{};
  const int mCapacityFactor{};
  std::shared_mutex mMutex{};
  std::unique_ptr<T[]> mArray{};

  void resize(int n) {
    if (n > mCapacity) {
      T *newArray = new T[n];
      std::copy_n(mArray.get(), mSize, newArray);
      mArray.reset(newArray);
      mCapacity = n;
    } else {
      std::cerr << "Resize failed" << std::endl;
    }
  };

public:
  vector() : mSize{0}, mCapacity{1}, mCapacityFactor{2} {
    mArray = std::make_unique<T[]>(mCapacity);
  };
  vector(const vector &) {}
  vector &operator=(const vector &a) { return *this; }
  T at(int n) {
    std::shared_lock<std::shared_mutex> lock{mMutex};
    if (n < 0) {
      throw std::invalid_argument{"vector::at: n " + std::to_string(n) +
                                  " < 0"};
    } else if (n >= mSize) {
      throw std::invalid_argument{"vector::at: n " + std::to_string(n) +
                                  " >= " + std::to_string(mSize)};
    } else {
      return mArray[n];
    }
  };

  int size() {
    std::shared_lock<std::shared_mutex> lock{mMutex};
    return mSize;
  };

  void push_back(const T &string) {
    std::unique_lock<std::shared_mutex> lock{mMutex};
    if (mSize == mCapacity) {
      resize(mCapacityFactor * mCapacity);
    }
    mArray[mSize] = string;
    ++mSize;
    return;
  };
};
} // namespace kyc

#endif
