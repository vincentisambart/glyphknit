/*
 * Copyright © 2014  Vincent Isambart
 *
 *  This file is part of Glyphknit.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef GLYPHKNIT_TAG_H_
#define GLYPHKNIT_TAG_H_

#include <cstring>
#include <cstdlib>

namespace glyphknit {

typedef uint32_t Tag;

const char kEmptyTagCharacter = ' ';

constexpr inline
Tag MakeTag(uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4) {
  return (Tag(c1) << 24) | (Tag(c2) << 16) | (Tag(c3) << 8) | Tag(uint8_t(c4));
}

constexpr inline
Tag MakeTag(char c1, char c2 = kEmptyTagCharacter, char c3 = kEmptyTagCharacter, char c4 = kEmptyTagCharacter) {
  return MakeTag(uint8_t(c1), uint8_t(c2), uint8_t(c3), uint8_t(c4));
}

// The definition of MakeTag below is mostly for using in automatic tests
inline
Tag MakeTag(const char *str) {
  switch (std::strlen(str)) {
    case 1:
      return MakeTag(str[0]);
    case 2:
      return MakeTag(str[0], str[1]);
    case 3:
      return MakeTag(str[0], str[1], str[2]);
    case 4:
      return MakeTag(str[0], str[1], str[2], str[3]);
    default:
      std::abort();
  }
}

static const Tag kTagUnknown = Tag(0);
static const Tag kOpenTypeTagDefaultScript = MakeTag('D','F','L','T');
static const Tag kOpenTypeTagDefaultLanguage = MakeTag('d','f','l','t');
static const Tag kOpenTypeTagPhoneticTranscription = MakeTag('I','P','P','H');

}

#endif  // GLYPHKNIT_TAG_H_
