//
// Created by rgnter on 23/05/2025.
//

#include "libserver/chatter/ChatterServer.hpp"
#include "libserver/Util.hpp"

#include <libserver/Stream.hpp>
#include <spdlog/spdlog.h>

constexpr bool Debug = false;

struct ChatterCommandLogin
{
  uint32_t val0{};
  std::string characterName{};
  uint32_t code{};
  uint32_t val1{};
};

alicia::ChatterServer::ChatterServer()
    : _server(
        [](ClientId clientId) {},
        [](ClientId clientId) {},
        [](ClientId clientId, asio::streambuf& readBuffer)
        {
          const auto buffer = readBuffer.data();

          alicia::SourceStream stream({
            static_cast<const std::byte*>(buffer.data()),
            buffer.size()});

          std::array<std::byte, 4092> dataBuffer;
          SinkStream dataSinkStream(
            {dataBuffer.begin(), dataBuffer.end()});

          constexpr std::array XorCode{
            static_cast<std::byte>(0x2B),
            static_cast<std::byte>(0xFE),
            static_cast<std::byte>(0xB8),
            static_cast<std::byte>(0x02)};
          while (stream.GetCursor() != stream.Size())
          {
            std::byte val;
            stream.Read(val);
            val ^= XorCode[(stream.GetCursor() - 1) % 4];
            dataSinkStream.Write(val);
          }

          spdlog::debug(
            "Chatter message:\n\n"
            "Size: {}\n"
            "Dump: \n\n{}\n\n",
            buffer.size(), GenerateByteDump({dataBuffer.begin(), stream.GetCursor()}));

          readBuffer.consume(buffer.size());
        },
        [](ClientId clientId, asio::streambuf& writeBuffer) {})
{
}

alicia::ChatterServer::~ChatterServer()
{
  if (_serverThread.joinable())
    _serverThread.join();
}

void alicia::ChatterServer::Host()
{
  _serverThread = std::thread([this]()
  {
    _server.Begin(boost::asio::ip::address_v4::any(), 10032);
  });

}
