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

#ifndef GLYPHKNIT_FONT_H_
#define GLYPHKNIT_FONT_H_

#include "autorelease.hh"
#include "language.hh"

#include <memory>
#include <cmath>
#include <CoreText/CoreText.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshift-sign-overflow"
#include <hb-ft.h>
#pragma clang diagnostic pop

namespace glyphknit {

enum class FontFamilyClass {
  kUnknown,
  kSansSerif,
  kSerif,
  kMonospace,
  kCursive,
  kFantasy,
};

class FontDescriptor {
  // In fact FontDescriptor is a glorified shared_ptr to a class that does everything
  // It's mainly to make it easier to use: the type name is simpler and no need to unreference the pointer for example when comparing
 public:
  FontDescriptor() : data_{nullptr} {}
  bool operator ==(const FontDescriptor &) const;
  bool operator !=(const FontDescriptor &compared_to) const { return !(*this == compared_to); }
  bool is_valid() const { return data_.get() != nullptr; }
  FT_Face GetFTFace() const;
  hb_font_t *GetHBFont() const;
  FontDescriptor GetFallback(size_t index, Language);
  AutoReleasedCFRef<CTFontRef> CreateNativeFont(float size) const;
  FontFamilyClass font_family_class() const;

  // TODO: way to get variations (bold/thin, italic, ...)
 private:
  friend class FontManager;
  class Data;
  FontDescriptor(AutoReleasedCFRef<CTFontDescriptorRef> &&);  // the font descriptor must have been normalized
  std::shared_ptr<Data> data_;
};

class FontManager {
 public:
  FT_Library ft_library() const { return ft_library_; }
  static FontDescriptor CreateDescriptorFromPostScriptName(const char *name);
  static FontDescriptor CreateDescriptorFromNativeFont(CTFontRef);
  //static FontDescriptor CreateDescriptorFromLocalFile(const char *path);
  static FontManager *instance();

 private:
  FontManager();
  ~FontManager();
  FT_Library ft_library_;
};

static const float kFontComparisonDelta = 0.015625f;
inline bool IsFontSizeSimilar(float a, float b) {
  return std::abs(a - b) < kFontComparisonDelta;
}

inline ssize_t SizeInFontUnits(double size, FontDescriptor font_descriptor, float font_size) {
  return size_t(double(size) * font_descriptor.GetFTFace()->units_per_EM / font_size);
}

}

#endif  // GLYPHKNIT_FONT_H_
