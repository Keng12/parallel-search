#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <mutex>

namespace kyc
{
  template <class T>
  class vector
  {
    int mSize{};
    int mCapacity{};
    const int mCapacityFactor{};
    std::unique_ptr<T[]> mArray{};

    T *get() const
    {
      return mArray.get();
    }

    vector(T *input_ptr, const int size) : mSize{size}, mCapacity{size}, mCapacityFactor{2}
    {
      assert(size > 0);
      mArray.reset(input_ptr);
    }

    std::unique_ptr<T[]> moveUniquePtr()
    {
      return std::move(mArray);
    }

  public:
    vector() : mSize{0}, mCapacity{0}, mCapacityFactor{2}
    {
      mArray = std::make_unique<T[]>(mCapacity);
    }

    vector(const vector &input) : mSize{input.getSize()}, mCapacity{input.getSize()}, mCapacityFactor{2}
    {
      T *newArray = new T[mSize];
      std::copy_n(input.get(), mSize, newArray);
      mArray.reset(newArray);
    }

    vector &operator=(vector input)
    {
      mSize = input.getSize();
      mCapacity = mSize;
      std::unique_ptr<T[]> tmpPtr = input.moveUniquePtr();
      mArray.swap(tmpPtr);
      return *this;
    }

    T at(const int n) const
    {
      assert(n >= 0 && n < mSize);
      return mArray[n];
    };

    void reserve(const int n)
    {
      assert(n > mCapacity);
      T *newArray = new T[n];
      std::copy_n(mArray.get(), mSize, newArray);
      mArray.reset(newArray);
      mCapacity = n;
    }

    int getSize() const
    {
      return mSize;
    }

    void push_back(const T &input)
    {
      if (mSize == mCapacity)
      {
        reserve(mCapacityFactor * mCapacity + 1);
      }
      mArray[mSize] = input;
      ++mSize;
      return;
    }

    vector extract(const int src_begin_index, const int elements_to_copy) const
    {
      assert(elements_to_copy > 0 && elements_to_copy <= mSize && src_begin_index >= 0 && src_begin_index < mSize);
      T *newArray = new T[elements_to_copy];
      std::copy_n(mArray.get() + src_begin_index, elements_to_copy, newArray);
      vector output{newArray, elements_to_copy};
      return output;
    }

    void append(vector const& appendage)
    {
      if (appendage.getSize() > 0)
      {
        T *newArray = new T[mSize + appendage.getSize()];
        std::copy_n(mArray.get(), mSize, newArray);
        std::copy_n(appendage.get(), appendage.getSize(), newArray + mSize);
        mArray.reset(newArray);
        mSize += appendage.getSize();
        mCapacity = mSize;
      }
    }
  };
} // namespace kyc

#endif
