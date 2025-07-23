//
// Created by rgnter on 13/07/2025.
//

#include "libserver/util/Locale.hpp"

#ifndef WIN32
#include <iconv.h>
#include <errno.h>
#elifdef WIN32
#include <icu.h>
#endif

#include <format>
#include <stdexcept>

namespace
{

#ifndef WIN32
//! Invalid iconv object.
const iconv_t InvalidIconv = reinterpret_cast<iconv_t>(-1);
#endif

} // anon namespace

namespace server
{

namespace locale
{

const std::string ToUtf8(const std::string& input)
{
  std::string output;

  #ifndef WIN32
  output.resize(input.length() * 2);

  const thread_local auto icd = iconv_open("EUC-KR", "UTF-8");
  if (icd == InvalidIconv)
    throw std::runtime_error("Conversion from EUC-KR to UTF-8 not supported");

  char const* source = input.data();
  size_t sourceBytesAvailable = input.length();
  char* destination = output.data();
  size_t destinationBytesAvailable = output.size();

  iconv(icd,
    const_cast<char**>(&source), // incorrect API specification, underlying data are const
    &sourceBytesAvailable,
    &destination,
    &destinationBytesAvailable);

  iconv_close(icd);
  #else

  std::u16string unicodeOutput;
  unicodeOutput.resize(input.length() * 2);

  thread_local UErrorCode error;

  thread_local UConverter* koreanConv = ucnv_open("EUC-KR", &error);
  ucnv_toUChars(
    koreanConv,
    unicodeOutput.data(),
    unicodeOutput.size(),
    input.data(),
    input.length(),
    &error);
  if (error > U_ZERO_ERROR)
  {
    throw std::runtime_error(
      std::format(
        "Failed to perform locale conversion from EUC-KR to Unicode: 0x{:x}",
        static_cast<uint32_t>(error)));
  }

  thread_local UConverter* utfConv = ucnv_open("UTF-8", &error);
  output.resize(UCNV_GET_MAX_BYTES_FOR_STRING(
    unicodeOutput.length(),
    ucnv_getMaxCharSize(utfConv)));

  ucnv_fromUChars(
    utfConv,
    output.data(),
    output.size(),
    unicodeOutput.data(),
    unicodeOutput.length(),
    &error);
  if (error > U_ZERO_ERROR)
  {
    throw std::runtime_error(
      std::format(
        "Failed to perform locale conversion from Unicode to UTF-8: 0x{:x}",
        static_cast<uint32_t>(error)));
  }

  #endif

  return std::string(output.data());
}

const std::string FromUtf8(const std::string& input)
{
  std::string output;

  #ifndef WIN32
  output.resize(input.length() * 2);

  const thread_local auto icd = iconv_open("UTF-8", "EUC-KR");
  if (icd == InvalidIconv)
    throw std::runtime_error("Conversion from UTF-8 to EUC-KR not supported");

  char const* source = input.data();
  size_t sourceBytesAvailable = input.length();
  char* destination = output.data();
  size_t destinationBytesAvailable = output.size();

  iconv(icd,
    const_cast<char**>(&source), // incorrect API specification, underlying data are const
    &sourceBytesAvailable,
    &destination,
    &destinationBytesAvailable);

  iconv_close(icd);
  #else

  std::u16string unicodeOutput;
  unicodeOutput.resize(input.length() * 2);

  thread_local UErrorCode error;

  thread_local UConverter* utfConv = ucnv_open("UTF-8", &error);
  ucnv_toUChars(
    utfConv,
    unicodeOutput.data(),
    unicodeOutput.size(),
    input.data(),
    input.length(),
    &error);
  if (error > U_ZERO_ERROR)
  {
    throw std::runtime_error(
      std::format(
        "Failed to perform locale conversion from UTF-8 to Unicode: 0x{:x}",
        static_cast<uint32_t>(error)));
  }

  thread_local UConverter* koreanConv = ucnv_open("EUC-KR", &error);
  output.resize(UCNV_GET_MAX_BYTES_FOR_STRING(
    unicodeOutput.length(),
    ucnv_getMaxCharSize(koreanConv)));

  ucnv_fromUChars(
    koreanConv,
    output.data(),
    output.size(),
    unicodeOutput.data(),
    unicodeOutput.length(),
    &error);
  if (error > U_ZERO_ERROR)
  {
    throw std::runtime_error(
      std::format(
        "Failed to perform locale conversion from Unicode to EUC-KR: 0x{:x}",
        static_cast<uint32_t>(error)));
  }

  #endif

  return std::string(output.data());
}

} // namespace locale

} // namespace server