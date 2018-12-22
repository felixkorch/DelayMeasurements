//===- StrideIterator.h -----------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Stride sequence.
//===----------------------------------------------------------------------===//

#ifndef REQSERVER_STRIDEITERATOR_H
#define REQSERVER_STRIDEITERATOR_H

template<class Position, class Distance>
class StrideIterator
{
  Position position;
  Distance stride;

public:
  StrideIterator(const Position& position, const Distance& stride)
      : position(position), stride(stride)
  {}

  StrideIterator& operator++()
  {
    position += stride;
    return *this;
  }

  Position operator*() const
  {
    return position;
  }

  bool operator!=(const Position& other_position) const
  {
    return position != other_position;
  }

  template<class DistanceB>
  bool operator!=(const StrideIterator<Position, DistanceB>& other) const
  {
    return position != *other;
  }

  Distance get_stride() const
  {
    return stride;
  }
};

#endif // REQSERVER_STRIDEITERATOR_H
