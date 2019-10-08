#ifndef SQL_TYPE_INET_H
#define SQL_TYPE_INET_H
/* Copyright (c) 2011, 2013, Oracle and/or its affiliates. All rights reserved.
   Copyright (c) 2014 MariaDB Foundation
   Copyright (c) 2019 MariaDB Corporation

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1335  USA */


static const size_t IN_ADDR_SIZE= 4;
static const size_t IN_ADDR_MAX_CHAR_LENGTH= 15;

static const size_t IN6_ADDR_SIZE= 16;
static const size_t IN6_ADDR_NUM_WORDS= IN6_ADDR_SIZE / 2;

/**
  Non-abbreviated syntax is 8 groups, up to 4 digits each,
  plus 7 delimiters between the groups.
  Abbreviated syntax is even shorter.
*/
static const uint IN6_ADDR_MAX_CHAR_LENGTH= 8 * 4 + 7;


class NativeBufferInet6: public NativeBuffer<IN6_ADDR_SIZE+1>
{
};

class StringBufferInet6: public StringBuffer<IN6_ADDR_MAX_CHAR_LENGTH+1>
{
};

/***********************************************************************/

class Inet4
{
  char m_buffer[IN_ADDR_SIZE];
protected:
  bool ascii_to_ipv4(const char *str, size_t length);
  bool character_string_to_ipv4(const char *str, size_t str_length,
                                CHARSET_INFO *cs)
  {
    if (cs->state & MY_CS_NONASCII)
    {
      char tmp[IN_ADDR_MAX_CHAR_LENGTH];
      String_copier copier;
      uint length= copier.well_formed_copy(&my_charset_latin1, tmp, sizeof(tmp),
                                           cs, str, str_length);
      return ascii_to_ipv4(tmp, length);
    }
    return ascii_to_ipv4(str, str_length);
  }
  bool binary_to_ipv4(const char *str, size_t length)
  {
    if (length != sizeof(m_buffer))
      return true;
    memcpy(m_buffer, str, length);
    return false;
  }
  // Non-initializing constructor
  Inet4() { }
public:
  void to_binary(char *dst, size_t dstsize) const
  {
    DBUG_ASSERT(dstsize >= sizeof(m_buffer));
    memcpy(dst, m_buffer, sizeof(m_buffer));
  }
  bool to_binary(String *to) const
  {
    return to->copy(m_buffer, sizeof(m_buffer), &my_charset_bin);
  }
  size_t to_string(char *dst, size_t dstsize) const;
  bool to_string(String *to) const
  {
    to->set_charset(&my_charset_latin1);
    if (to->alloc(INET_ADDRSTRLEN))
      return true;
    to->length((uint32) to_string((char*) to->ptr(), INET_ADDRSTRLEN));
    return false;
  }
};


class Inet4_null: public Inet4, public Null_flag
{
public:
  // Initialize from a text representation
  Inet4_null(const char *str, size_t length, CHARSET_INFO *cs)
   :Null_flag(character_string_to_ipv4(str, length, cs))
  { }
  Inet4_null(const String &str)
   :Inet4_null(str.ptr(), str.length(), str.charset())
  { }
  // Initialize from a binary representation
  Inet4_null(const char *str, size_t length)
   :Null_flag(binary_to_ipv4(str, length))
  { }
  Inet4_null(const Binary_string &str)
   :Inet4_null(str.ptr(), str.length())
  { }
public:
  const Inet4& to_inet4() const
  {
    DBUG_ASSERT(!is_null());
    return *this;
  }
  void to_binary(char *dst, size_t dstsize) const
  {
    to_inet4().to_binary(dst, dstsize);
  }
  bool to_binary(String *to) const
  {
    return to_inet4().to_binary(to);
  }
  size_t to_string(char *dst, size_t dstsize) const
  {
    return to_inet4().to_string(dst, dstsize);
  }
  bool to_string(String *to) const
  {
    return to_inet4().to_string(to);
  }
};


class Inet6
{
protected:
  char m_buffer[IN6_ADDR_SIZE];
  bool make_from_item(Item *item);
  bool ascii_to_ipv6(const char *str, size_t str_length);
  bool character_string_to_ipv6(const char *str, size_t str_length,
                                CHARSET_INFO *cs)
  {
    if (cs->state & MY_CS_NONASCII)
    {
      char tmp[IN6_ADDR_MAX_CHAR_LENGTH];
      String_copier copier;
      uint length= copier.well_formed_copy(&my_charset_latin1, tmp, sizeof(tmp),
                                           cs, str, str_length);
      return ascii_to_ipv6(tmp, length);
    }
    return ascii_to_ipv6(str, str_length);
  }
  bool make_from_character_or_binary_string(const String *str);
  bool binary_to_ipv6(const char *str, size_t length)
  {
    if (length != sizeof(m_buffer))
      return true;
    memcpy(m_buffer, str, length);
    return false;
  }

  Inet6() { }

public:
  static uint binary_length() { return IN6_ADDR_SIZE; }
  /**
    Non-abbreviated syntax is 8 groups, up to 4 digits each,
    plus 7 delimiters between the groups.
    Abbreviated syntax is even shorter.
  */
  static uint max_char_length() { return IN6_ADDR_MAX_CHAR_LENGTH; }

  static bool only_zero_bytes(const char *ptr, uint length)
  {
    for (uint i= 0 ; i < length; i++)
    {
      if (ptr[i] != 0)
        return false;
    }
    return true;
  }

public:

  Inet6(Item *item, bool *error)
  {
    *error= make_from_item(item);
  }
  void to_binary(char *str, size_t str_size) const
  {
    DBUG_ASSERT(str_size >= sizeof(m_buffer));
    memcpy(str, m_buffer, sizeof(m_buffer));
  }
  bool to_binary(String *to) const
  {
    return to->copy(m_buffer, sizeof(m_buffer), &my_charset_bin);
  }
  bool to_native(Native *to) const
  {
    return to->copy(m_buffer, sizeof(m_buffer));
  }
  size_t to_string(char *dst, size_t dstsize) const;
  bool to_string(String *to) const
  {
    to->set_charset(&my_charset_latin1);
    if (to->alloc(INET6_ADDRSTRLEN))
      return true;
    to->length((uint32) to_string((char*) to->ptr(), INET6_ADDRSTRLEN));
    return false;
  }
  bool is_v4compat() const
  {
    static_assert(sizeof(in6_addr) == IN6_ADDR_SIZE, "unexpected in6_addr size");
    return IN6_IS_ADDR_V4COMPAT((struct in6_addr *) m_buffer);
  }
  bool is_v4mapped() const
  {
    static_assert(sizeof(in6_addr) == IN6_ADDR_SIZE, "unexpected in6_addr size");
    return IN6_IS_ADDR_V4MAPPED((struct in6_addr *) m_buffer);
  }
  int cmp(const char *str, size_t length) const
  {
    DBUG_ASSERT(length == sizeof(m_buffer));
    return memcmp(m_buffer, str, length);
  }
  int cmp(const Binary_string &other) const
  {
    return cmp(other.ptr(), other.length());
  }
  int cmp(const Inet6 &other) const
  {
    return memcmp(m_buffer, other.m_buffer, sizeof(m_buffer));
  }
};


class Inet6_zero: public Inet6
{
public:
  Inet6_zero()
  {
    bzero(&m_buffer, sizeof(m_buffer));
  }
};


class Inet6_null: public Inet6, public Null_flag
{
public:
  // Initialize from a text representation
  Inet6_null(const char *str, size_t length, CHARSET_INFO *cs)
   :Null_flag(character_string_to_ipv6(str, length, cs))
  { }
  Inet6_null(const String &str)
   :Inet6_null(str.ptr(), str.length(), str.charset())
  { }
  // Initialize from a binary representation
  Inet6_null(const char *str, size_t length)
   :Null_flag(binary_to_ipv6(str, length))
  { }
  Inet6_null(const Binary_string &str)
   :Inet6_null(str.ptr(), str.length())
  { }
  // Initialize from an Item
  Inet6_null(Item *item)
   :Null_flag(make_from_item(item))
  { }
public:
  const Inet6& to_inet6() const
  {
    DBUG_ASSERT(!is_null());
    return *this;
  }
  void to_binary(char *str, size_t str_size) const
  {
    to_inet6().to_binary(str, str_size);
  }
  bool to_binary(String *to) const
  {
    return to_inet6().to_binary(to);
  }
  size_t to_string(char *dst, size_t dstsize) const
  {
    return to_inet6().to_string(dst, dstsize);
  }
  bool to_string(String *to) const
  {
    return to_inet6().to_string(to);
  }
  bool is_v4compat() const
  {
    return to_inet6().is_v4compat();
  }
  bool is_v4mapped() const
  {
    return to_inet6().is_v4mapped();
  }
};


#endif /* SQL_TYPE_INET_H */
