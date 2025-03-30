#pragma once

#include <stdio.h>
#include <math.h>


template <typename T, int N>
class CyclycBuffer
{
public:
  CyclycBuffer(T initValue = {})
  {
    data = new T[N];
    for (size_t i = 0; i < N; ++i)
    {
      data[i] = initValue;
    }
    
    sz = N;
    top = 0;
  }

  size_t size()
  {
    return sz;
  }

  void push(T value)
  {
    top = (top + 1) % sz;
    data[top] = value;
  }

  T &operator[](size_t n)
  {
    if (n < sz)
    {
      return data[(top - n) % sz];
    }
    throw;
  }

private:
  T* data;
  size_t sz;
  size_t top;
};
