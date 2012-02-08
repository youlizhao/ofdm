/* -*- c++ -*- */
/*
 * Copyright 2005 Free Software Foundation, Inc.
 * 
 * This file is part of GNU Radio
 * 
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

/*
 * See also ISO 3309 [ISO-3309] or ITU-T V.42 [ITU-V42] for a formal specification.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <ftw_crc32.h>
#include <iostream>

unsigned int
ftw_update_crc32(unsigned int crc, const unsigned char *data, size_t len)
{
  int j;
  unsigned int byte, mask;
  static unsigned int table[256];
  /* Set up the table if necesary */
  if (table[1] == 0) {
    for(byte = 0; byte <= 255; byte++) {
      crc = byte;
      for (j = 7; j >= 0; j--) {
        mask = -(crc & 1);
	crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
	table[byte] = crc;
    }
  }

  /* Calculate the CRC32*/
  size_t i = 0;
  crc = 0xFFFFFFFF;
  for (i = 0; i < len; i++) {
    byte = data[i];    //Get next byte
    crc = (crc >> 8) ^ table[(crc ^ byte) & 0xFF];
  }
  unsigned int crc_reversed;
  crc_reversed = 0x00000000;
  for (j=31; j >= 0; j--) {
    crc_reversed |= ((crc >> j) & 1) << (31 - j);  		
  }
  return crc_reversed;
}

unsigned int
ftw_update_crc32(unsigned int crc, const std::string s)
{
  return ftw_update_crc32(crc, (const unsigned char *) s.data(), s.size());
}
    
unsigned int
ftw_crc32(const unsigned char *buf, size_t len)
{
  return ftw_update_crc32(0xffffffff, buf, len) ^ 0xffffffff;
}

unsigned int
ftw_crc32(const std::string s)
{
  return ftw_crc32((const unsigned char *) s.data(), s.size());
}

