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

#include "text_block.hh"
#include <unicode/ustring.h>

namespace glyphknit {

void TextBlock::SetText(const uint16_t *utf16_text, const size_t utf16_length) {
  text_.resize(utf16_length);
  std::copy(utf16_text, utf16_text+utf16_length, text_.begin());
}

const uint32_t kReplacementCharacter = 0xfffd;
void TextBlock::SetText(const char *utf8_text, const size_t utf8_length) {
  text_.resize(utf8_length);  // a UTF-16 string is always smaller than its UTF-8 equivalent (in code units of course, not necessarily in bytes)
  int32_t utf16_length;
  UErrorCode errorCode = U_ZERO_ERROR;
  u_strFromUTF8WithSub(text_.data(), int32_t(text_.capacity()), &utf16_length, utf8_text, int32_t(utf8_length), kReplacementCharacter, NULL, &errorCode);
  text_.resize(utf16_length);
}

void TextBlock::SetText(const char *utf8_text) {
  SetText(utf8_text, std::strlen(utf8_text));
}

TextBlock::~TextBlock() {
}

}
