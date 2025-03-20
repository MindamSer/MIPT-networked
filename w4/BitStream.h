#pragma once

#include <cstdint>
#include <cstring>

class BitStream
{
public:
  BitStream() = delete;

  BitStream(uint64_t n)
  {
    if (n < 1)
      throw;
  
    dataPtr = new char[n];
    storageCapacity = n;
  }

  ~BitStream()
  {
    delete[] dataPtr;
  }

  template <typename T>
  BitStream(T *data, uint64_t n) : BitStream(n)
  {
    memcpy(dataPtr, data, n);
    storedSize = n;
  }

  char *data() const
  {
    return dataPtr + readStart;
  }

  uint64_t size() const
  {
    return storedSize - readStart;
  }

  void skip(uint64_t n)
  {
    readStart += n;
    if (readStart >= storedSize)
    {
      storedSize = 0;
      readStart = 0;
    }
  }

  template<typename T>
  void flush(T *dst)
  {
    memcpy(dst, dataPtr + readStart, size());
    storedSize = 0;
    readStart = 0;
  }

  template<typename T>
  BitStream &operator<<(const T &val)
  {
    uint64_t valSize = sizeof(T);
  
    if (storedSize + valSize > storageCapacity)
    {
      char *newPtr = new char[storageCapacity*2];
      memcpy(newPtr, dataPtr, storedSize);
      delete[] dataPtr;
  
      dataPtr = newPtr;
      storageCapacity *= 2;
    }
  
    memcpy(dataPtr + storedSize, &val, valSize);
    storedSize += valSize;
  
    return *this;
  }

  template<typename T>
  BitStream &operator>>(T &val)
  {
    uint64_t valSize = sizeof(T);
  
    if (size() < valSize)
      throw;
  
    memcpy(&val, dataPtr + readStart, valSize);
    readStart += valSize;
  
    return *this;
  }

private:
  char *dataPtr;
  uint64_t storedSize = 0;
  uint64_t storageCapacity = 0;
  uint64_t readStart = 0;
};
