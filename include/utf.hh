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

#ifndef GLYPHKNIT_UTF_H_
#define GLYPHKNIT_UTF_H_

#include <unicode/utf.h>
#include <sys/types.h>

namespace glyphknit {

// based on ICU's U16_GET
inline
UChar32 GetCodepoint(const uint16_t *text, ssize_t length, ssize_t offset) {
  auto c = text[offset];
  if (U16_IS_SURROGATE(c)) {
    if (U16_IS_SURROGATE_LEAD(c)) {
      if (offset + 1 < length) {
        auto c2 = text[offset + 1];
        if (U16_IS_TRAIL(c2)) {
          return U16_GET_SUPPLEMENTARY(c, c2);
        }
      }
    }
    else if (offset > 0) {
      uint16_t c2 = text[offset - 1];
      if (U16_IS_LEAD(c2)) {
        return U16_GET_SUPPLEMENTARY(c2, c);
      }
    }
  }
  return UChar32(c);
}

// based on ICU's U16_NEXT
inline
UChar32 ConsumeCodepoint(const uint16_t *text, ssize_t length, ssize_t &offset) {
  auto c = text[offset];
  ++offset;
  if (U16_IS_LEAD(c) && offset < length) {
    auto c2 = text[offset];
    if (U16_IS_TRAIL(c2)) {
      ++offset;
      return U16_GET_SUPPLEMENTARY(c, c2);
    }
  }
  return UChar32(c);
}

}

#endif  // GLYPHKNIT_UTF_H_
