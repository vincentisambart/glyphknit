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
#include "language.hh"

namespace glyphknit {

class TextBlock {
 public:
  // TODO: have a default font it the user does not specify one
  TextBlock(std::shared_ptr<FontFace> default_font_face, float default_font_size) : default_font_face_{default_font_face}, default_font_size_{default_font_size} {}
  ~TextBlock();

  void SetText(const uint16_t *, size_t length);
  void SetText(const char *, size_t length);
  void SetText(const char *);

  const uint16_t *text_content() { return string_.getBuffer(); }
  ssize_t text_length() { return string_.length(); }

  const std::shared_ptr<FontFace> &default_font_face() { return default_font_face_; }
  float default_font_size() const { return default_font_size_; }
  void set_default_font_face(const std::shared_ptr<FontFace> &font_face) {
    default_font_face_ = font_face;
  }
  void set_default_font_size(float &font_size) {
    default_font_size_ = font_size;
  }
  void set_default_font(const std::shared_ptr<FontFace> &font_face, float font_size) {
    default_font_face_ = font_face;
    default_font_size_ = font_size;
  }

 private:
  icu::UnicodeString string_;  // TODO: use std:vector<uint16_t>
  std::shared_ptr<FontFace> default_font_face_;
  float default_font_size_;
};

}

#endif  // GLYPHKNIT_TEXT_BLOCK_H_
