#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>

namespace kyc
{
  template <class T>
  class vector
  {
    int mSize{};
    int mCapacity{};
    const int mCapacityFactor{};
    std::shared_mutex mMutex{};
    std::unique_ptr<T[]> mArray{};

  public:
    vector() : mSize{0}, mCapacity{1}, mCapacityFactor{2}
    {
      mArray = std::make_unique<T[]>(mCapacity);
    };
    vector(std::unique_ptr<T[]> &&input_ptr) : mSize{0}, mCapacity{1}, mCapacityFactor{2}
    {
      mArray.swap(input_ptr);
    };
    vector(const vector &) {}
    vector &operator=(const vector &a) { return *this; }
    T at(int n)
    {
      std::shared_lock<std::shared_mutex> lock{mMutex};
      if (n < 0)
      {
        throw std::invalid_argument{"vector::at: n " + std::to_string(n) +
                                    " < 0"};
      }
      else if (n >= mSize)
      {
        throw std::invalid_argument{"vector::at: n " + std::to_string(n) +
                                    " >= " + std::to_string(mSize)};
      }
      else
      {
        return mArray[n];
      }
    };
    void reserve(int n)
    {
      if (n > mCapacity)
      {
        T *newArray = new T[n];
        std::copy_n(mArray.get(), mSize, newArray);
        mArray.reset(newArray);
        mCapacity = n;
      }
      else
      {
        std::cerr << "reserve failed" << std::endl;
      }
    };
    int size()
    {
      std::shared_lock<std::shared_mutex> lock{mMutex};
      return mSize;
    };

    void push_back(const T &string)
    {
      std::unique_lock<std::shared_mutex> lock{mMutex};
      if (mSize == mCapacity)
      {
        reserve(mCapacityFactor * mCapacity);
      }
      mArray[mSize] = string;
      ++mSize;
      return;
    };
    vector extract(const int src_begin_index, const int elements_to_copy)
    {
      std::unique_ptr<T[]> newArray = std::make_unique<T[]>(elements_to_copy);
      std::copy_n(mArray.get() + src_begin_index, elements_to_copy, newArray.get());
      return vector{std::move(newArray)};
    };
  };
} // namespace kyc

#endif
