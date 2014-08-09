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

#ifndef GLYPHKNIT_TEXT_BLOCK_H_
#define GLYPHKNIT_TEXT_BLOCK_H_

#include <unicode/unistr.h>
#include "font.hh"

namespace glyphknit {

class TextBlock {
 public:
  // TODO: have a default font it the user does not specify one
  TextBlock(const SizedFont &default_sized_font) : default_sized_font_{default_sized_font} {}
  ~TextBlock();

  void SetText(const uint16_t *, size_t length);
  void SetText(const char *, size_t length);
  void SetText(const char *);
  void SetText(const icu::UnicodeString &);

  const icu::UnicodeString &text_content() { return string_; }

  const SizedFont &default_sized_font() { return default_sized_font_; }
  void set_default_sized_font(const SizedFont &font) {
    default_sized_font_ = font;
  }

 private:
  icu::UnicodeString string_;
  SizedFont default_sized_font_;
};

}

#endif  // GLYPHKNIT_TEXT_BLOCK_H_
