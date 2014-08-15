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

#include "font.hh"
#include "language.hh"

#include <vector>
#include <list>
#include <cmath>

namespace glyphknit {

static const float kDefaultFontSize = 12;

struct AllTextAttributes {
  std::shared_ptr<FontFace> font_face;
  float font_size;
  Language language;

  AllTextAttributes(const std::shared_ptr<FontFace> &face, float size = kDefaultFontSize, Language lang = kLanguageUnknown) :
    font_face(face), font_size(size), language(lang) {}
  AllTextAttributes() : font_face(nullptr), font_size(0), language(kLanguageUnknown) {}

  bool operator ==(const AllTextAttributes &compared_to) const {
    return this->language == compared_to.language
      && IsFontSizeSimilar(this->font_size, compared_to.font_size)
      && *this->font_face == *compared_to.font_face;
  }
};

struct TextAttributesRun {
  AllTextAttributes attributes;
  ssize_t start, end;
};

class TextBlock {
 public:
  // TODO: have a default font if the user does not specify one
  TextBlock(std::shared_ptr<FontFace> default_font_face, float default_font_size) {
    AllTextAttributes base_attributes{default_font_face, default_font_size};
    attributes_runs_.push_front(TextAttributesRun{
      .attributes = base_attributes,
      .start = 0,
      .end = 0,
    });
  }
  ~TextBlock();

  void SetText(const uint16_t *, size_t length);
  void SetText(const char *, size_t length);
  void SetText(const char *);

  const uint16_t *text_content() { return text_.data(); }
  ssize_t text_length() { return text_.size(); }
  const std::list<TextAttributesRun> &attributes_runs() const { return attributes_runs_; }

  void SetFontSize(float font_size, ssize_t start = 0, ssize_t end = -1);
  void SetFontFace(std::shared_ptr<FontFace> font_face, ssize_t start = 0, ssize_t end = -1);
  void SetLanguage(Language language, ssize_t start = 0, ssize_t end = -1);

 private:
  void MergeAdjacentRunsWithSameAttributes();

  template <typename T, typename Comparator>
  void SetAttribute(T AllTextAttributes::*attribute, const T &value, Comparator f, ssize_t start, ssize_t end);

  std::vector<uint16_t> text_;
  std::list<TextAttributesRun> attributes_runs_;
};

}

#endif  // GLYPHKNIT_TEXT_BLOCK_H_
