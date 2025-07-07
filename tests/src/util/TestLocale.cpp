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

#include <cassert>
#include <locale>

#include <iconv.h>
#include <errno.h>

namespace
{

const iconv_t InvalidIconv = reinterpret_cast<iconv_t>(-1);

void TestLocale()
{
  #ifndef WIN32
  const auto icd = iconv_open("EUC-KR", "UTF-8");
  if (icd == InvalidIconv)
  {
    if (errno == EINVAL)
    {
      printf("conversion from EUC-KR to UTF-8 not supported\n");
    }
    return;
  }
  std::string koreanText = "abcdef8";

  std::string defaultText;
  defaultText.resize(koreanText.size() * 2);

  char* source = koreanText.data();
  size_t sourceBytesAvailable = koreanText.size();
  char* destination = defaultText.data();
  size_t destinationBytesAvailable = defaultText.size();

  iconv(icd,
    &source,
    &sourceBytesAvailable,
    &destination,
    &destinationBytesAvailable);

  iconv_close(icd);

  printf("%s", defaultText.c_str());
  #else
  printf("iconv not supported on windows\n");
  #endif
}

} // namespace

int main()
{
  TestLocale();
}
