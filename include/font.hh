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

#include <CoreText/CoreText.h>
#include <memory>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "autorelease.hh"

namespace glyphknit {

class FontFace {
 public:
  ~FontFace();
  AutoReleasedCFRef<CTFontDescriptorRef> font_descriptor() const { return font_descriptor_; }

  AutoReleasedCFRef<CTFontRef> CreateCTFont(float size);
  FT_Face GetFTFace();

  // TODO: way to get variations (bold/thin, italic, ...)

 private:
  friend class FontManager;
  FontFace(AutoReleasedCFRef<CTFontDescriptorRef> &&); // the font descriptor must have been normalized
  AutoReleasedCFRef<CTFontDescriptorRef> font_descriptor_;
  FT_Face ft_face_;
};

class FontManager {
 public:
  FT_Library ft_library() const { return ft_library_; }
  static std::shared_ptr<FontFace> LoadFontFromPostScriptName(const char *name);
  //static std::shared_ptr<FontFace> LoadFontFromLocalFile(const char *path);
  static FontManager *instance();

 private:

  FontManager();
  ~FontManager();
  FT_Library ft_library_;
};

}

#endif  // GLYPHKNIT_FONT_H_
