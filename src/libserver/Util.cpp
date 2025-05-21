/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2024 Story Of Alicia
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **/

#include "libserver/Util.hpp"

#define Int32x32To64(a, b) ((uint16_t)(((uint64_t)((long)(a))) * ((long)(b))))

namespace alicia
{

// Sources:
//  https://learn.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-int32x32to64
//  https://gist.github.com/JamesMenetrey/d3f494262bcab48af1d617c3d39f34cf#file-winnt-h-L944
WinFileTime UnixTimeToFileTime(const std::chrono::system_clock::time_point& timePoint)
{
  const uint64_t unixTime = timePoint.time_since_epoch().count();
  const uint64_t convertedUnixTime = Int32x32To64(unixTime, 10000000) + 116444736000000000;
  return WinFileTime{
    .dwLowDateTime = static_cast<uint32_t>(convertedUnixTime),
    .dwHighDateTime = static_cast<uint32_t>(convertedUnixTime >> 32)};
}

asio::ip::address_v4 ResolveHostName(const std::string& host)
{
  const auto address = asio::ip::make_address(host);
  // If host is an IP address, parse that directly.
  if (not address.is_unspecified() && address.is_v4())
    return address.to_v4();

  asio::io_context ioContext;
  asio::ip::tcp::resolver resolver(ioContext);
  const auto endpoints = resolver.resolve(host, "");

  for (const auto& endpoint : endpoints)
  {
    const auto& addr = endpoint.endpoint().address();
    if (addr.is_v4())
      return addr.to_v4();
  }

  return {};
}

} // namespace alicia
