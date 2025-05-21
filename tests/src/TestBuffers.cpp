#include "libserver/Stream.hpp"

#include <boost/asio/streambuf.hpp>

#include <cassert>

namespace {

struct Datum
{
  uint32_t val0 = 0;
  uint32_t val1 = 0;

  static void Write(
    const Datum& value,
    alicia::SinkStream& stream)
  {
    stream.Write(value.val0)
      .Write(value.val1);
  }

  static void Read(
    Datum& value,
    alicia::SourceStream& stream)
  {
    stream.Read(value.val0)
      .Read(value.val1);
  }
};

//! Perform test of magic encoding/decoding.
void TestBuffers()
{
  boost::asio::streambuf buf;
  auto mutableBuffer = buf.prepare(4092);
  alicia::SinkStream sink(std::span(
    static_cast<std::byte*>(mutableBuffer.data()),
    mutableBuffer.size()));

  sink.Write(0xCAFE);
  sink.Write(0xBABE);

  Datum structToWrite{
    .val0 = 0xBAAD,
    .val1 = 0xF00D};
  sink.Write(structToWrite);

  assert(sink.GetCursor() == 4 * sizeof(uint32_t));
  buf.commit(sink.GetCursor());

  auto constBuffer = buf.data();
  alicia::SourceStream source(std::span(
    static_cast<const std::byte*>(constBuffer.data()),
    constBuffer.size()));

  uint32_t cafe{};
  uint32_t babe{};
  Datum structToRead{};

  source.Read(cafe)
    .Read(babe)
    .Read(structToRead);

  assert(cafe == 0xCAFE && babe == 0xBABE);
  assert(structToRead.val0 == structToWrite.val0);
  assert(structToRead.val1 == structToWrite.val1);
  assert(source.GetCursor() == 4 * sizeof(uint32_t));
}

} // namespace anon

int main() {
  TestBuffers();
}

