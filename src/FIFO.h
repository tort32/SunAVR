#pragma once
#include "stdafx.h"

template <class T, uint8_t N = 64>
class FIFO
{
public:
  FIFO()
  {
    begin = end = 0;
  }

  // Add element to the queue
  /*bool push_back(const T& item)
  {
    uint8_t newEnd = (end + 1) % N;
    if(newEnd == begin)
      return false; // No more space left

    mData[end] = item; // store the element

    end = newEnd;
    return true;
  }*/
  T& push_back()
  {
    T& ret = mData[end]; // store the element

    end = (end + 1) % N;

    return ret;
  }

  // Get element from the queue
  const T& pop_front()
  {
    const T& element = mData[begin];
    if(begin != end)
    {
      begin = (begin + 1) % N;
    }
    return element;
  }

  uint8_t size() const
  {
    if(end >= begin)
      return end - begin;
    else
      return end + N - begin;
  }

private:
   T mData[N];
   uint8_t begin;
   uint8_t end;
};
