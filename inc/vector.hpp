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

    T *constGet() const
    {
      return mArray.get();
    }

    void reserve_nonlocking(const int n)
    {
      assert(n > mCapacity);
      T *newArray = new T[n];
      std::copy_n(mArray.get(), mSize, newArray);
      mArray.reset(newArray);
      mCapacity = n;
    }

    vector(T *input_ptr, const int size) : mSize{size}, mCapacity{size}, mCapacityFactor{2}
    {
      assert(size > 0);
      mArray.reset(input_ptr);
    }

    int constGetSize() const
    {
      std::cout << "constGetSize " << mSize << std::endl;
      return mSize;
    }

    std::unique_ptr<T[]> moveUniquePtr()
    {
      std::shared_lock<std::shared_mutex> lock{mMutex};
      return std::move(mArray);
    }

  public:
    vector() : mSize{0}, mCapacity{0}, mCapacityFactor{2}
    {
      mArray = std::make_unique<T[]>(mCapacity);
    }

    vector(const vector &input) : mSize{input.constGetSize()}, mCapacity{input.constGetSize()}, mCapacityFactor{2}
    {
      T *newArray = new T[mSize];
      std::copy_n(input.constGet(), mSize, newArray);
      mArray.reset(newArray);
      std::cout << "Copy constructor: " << mSize << std::endl;
    }

    vector &operator=(vector input)
    {
      mSize = input.getSize();
      mCapacity = mSize;
      std::unique_ptr<T[]> tmpPtr = input.moveUniquePtr();
      mArray.swap(tmpPtr);
      return *this;
    }

    T at(const int n)
    {
      std::shared_lock<std::shared_mutex> lock{mMutex};
      assert(n >= 0 && n < mSize);
      return mArray[n];
    };

    void reserve(const int n)
    {
      std::unique_lock<std::shared_mutex> lock{mMutex};
      reserve_nonlocking(n);
    }

    int getSize()
    {
      std::shared_lock<std::shared_mutex> lock{mMutex};
      return mSize;
    }

    void push_back(const T &input)
    {
      std::unique_lock<std::shared_mutex> lock{mMutex};
      if (mSize == mCapacity)
      {
        reserve_nonlocking(mCapacityFactor * mCapacity + 1);
      }
      mArray[mSize] = input;
      ++mSize;
      return;
    }

    vector extract(const int src_begin_index, const int elements_to_copy)
    {
      std::shared_lock<std::shared_mutex> lock{mMutex};
      assert(elements_to_copy > 0 && elements_to_copy <= mSize && src_begin_index >= 0 && src_begin_index < mSize);
      T *newArray = new T[elements_to_copy];
      std::copy_n(mArray.get() + src_begin_index, elements_to_copy, newArray);
      vector output{newArray, elements_to_copy};
      std::cout << "Extracted vector size: " << output.getSize() << std::endl;
      return output;
    }

    // Pass by value -> appendage has to be copied, otherwise can be changed externally
    void append(vector appendage)
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
