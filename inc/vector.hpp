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

    T *get()
    {
      std::shared_lock<std::shared_mutex> lock{mMutex};
      return mArray.get();
    }

    void reserve_nonlocking(int n)
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
        std::cerr << "Reserve failed with n: " << n << " and mCapacity: " << mCapacity << std::endl;
      }
    }

  public:
    vector() : mSize{0}, mCapacity{1}, mCapacityFactor{2}
    {
      mArray = std::make_unique<T[]>(mCapacity);
    }

    vector(T *input_ptr, int size) : mSize{size}, mCapacity{size}, mCapacityFactor{2}
    {
      mArray.reset(input_ptr);
    }

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
      std::unique_lock<std::shared_mutex> lock{mMutex};
      reserve_nonlocking(n);
    }

    int getSize()
    {
      std::shared_lock<std::shared_mutex> lock{mMutex};
      return mSize;
    }

    void push_back(const T &string)
    {
      std::unique_lock<std::shared_mutex> lock{mMutex};
      if (mSize == mCapacity)
      {
        reserve_nonlocking(mCapacityFactor * mCapacity);
      }
      mArray[mSize] = string;
      ++mSize;
      return;
    }

    vector extract(const int src_begin_index, const int elements_to_copy)
    {
      std::shared_lock<std::shared_mutex> lock{mMutex};
      if (elements_to_copy > mSize){
        throw std::invalid_argument{"Planning to copy " + std::to_string(elements_to_copy) + " elements of vector with size " + std::to_string(mSize)};
      }
      T *newArray = new T[elements_to_copy];
      std::copy_n(mArray.get() + src_begin_index, elements_to_copy, newArray);
      return vector{newArray, elements_to_copy};
    }

    void append(const vector &appendage)
    {
      std::unique_lock<std::shared_mutex> lock{mMutex};
      T *newArray = new T[mSize + appendage.getSize()];
      std::copy_n(mArray.get(), mSize, newArray);
      std::copy_n(appendage.get(), appendage.getSize(), newArray + mSize);
      mArray.reset(newArray);
      mSize += appendage.getSize();
      mCapacity = mSize;
    }
  };
} // namespace kyc

#endif
