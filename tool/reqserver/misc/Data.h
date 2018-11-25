//===- Data.h ---------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Data class handling owned bytes of raw memory.
//===----------------------------------------------------------------------===//

namespace reqserver
{

class Data {
  std::unique_ptr<std::uint8_t[]> raw_dynamic_mem;

public:
  Data(std::nullptr_t)
      : raw_dynamic_mem(nullptr) {}

  Data(std::size_t size)
      : raw_dynamic_mem(std::make_unique<std::uint8_t[]>(size)) {}

  template <class T>
  operator std::unique_ptr<T>&& ()
  { return std::unique_ptr<T>(std::move(raw_dynamic_mem)); }

  auto pointer()
  { return raw_dynamic_mem.get(); }

  bool operator==(std::nullptr_t)
  { return raw_dynamic_mem == nullptr; }
};

} // reqserver
