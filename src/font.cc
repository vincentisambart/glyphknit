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

#include "font.hh"
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_TAGS_H
#include "autorelease.hh"

#include <CoreFoundation/CoreFoundation.h>
#include <sys/param.h>

namespace glyphknit {

class FontDescriptor::Data {
 public:
  AutoReleasedCFRef<CTFontDescriptorRef> &native_font_descriptor() { return native_font_descriptor_; }
  Data(AutoReleasedCFRef<CTFontDescriptorRef> &&);
  ~Data();
  FT_Face GetFTFace() const;
  hb_font_t *GetHBFont() const;
  AutoReleasedCFRef<CTFontRef> CreateNativeFont(float size) const;
  bool operator ==(const Data &) const;

 private:
  AutoReleasedCFRef<CTFontDescriptorRef> native_font_descriptor_;
  mutable FT_Face ft_face_;
  mutable hb_font_t *hb_font_;
};

static bool FaceContainsTable(FT_Face face, FT_ULong tag) {
  FT_ULong table_length = 0;
  auto error = FT_Load_Sfnt_Table(face, tag, 0, nullptr, &table_length);
  return (error == 0);
}

FontDescriptor::Data::~Data() {
  if (ft_face_ != nullptr) {
    FT_Done_Face(ft_face_);
  }
  if (hb_font_ != nullptr) {
    hb_font_destroy(hb_font_);
  }
}

FontDescriptor::Data::Data(AutoReleasedCFRef<CTFontDescriptorRef> &&native_font_descriptor) : native_font_descriptor_{native_font_descriptor}, ft_face_{nullptr}, hb_font_{nullptr} {
}

AutoReleasedCFRef<CTFontRef> FontDescriptor::Data::CreateNativeFont(float size) const {
  return {CTFontCreateWithFontDescriptor(native_font_descriptor_.get(), size, nullptr)};
}

hb_font_t *FontDescriptor::Data::GetHBFont() const {
  if (hb_font_ == nullptr) {
    hb_font_ = hb_ft_font_create(GetFTFace(), nullptr);
  }
  return hb_font_;
}

FT_Face FontDescriptor::Data::GetFTFace() const {
  if (ft_face_ != nullptr) {
    return ft_face_;
  }
  auto url = MakeAutoReleasedCFRef<CFURLRef>(CTFontDescriptorCopyAttribute(native_font_descriptor_.get(), kCTFontURLAttribute));
  assert(url.get() != nullptr);

  char path[MAXPATHLEN];
  auto could_get_representation = CFURLGetFileSystemRepresentation(url.get(), true, reinterpret_cast<uint8_t *>(path), sizeof(path));
  assert(could_get_representation);

  auto font_name = MakeAutoReleasedCFRef<CFStringRef>(CTFontDescriptorCopyAttribute(native_font_descriptor_.get(), kCTFontNameAttribute));

  // TODO: save PostScript name in instance variable?

  // the OpenType spec says that the Postscript name of a font should be no longer than 63 characters
  // http://www.microsoft.com/typography/otspec/name.htm
  char searched_postscript_name[64];
  auto could_get_cstring = CFStringGetCString(font_name.get(), searched_postscript_name, sizeof(searched_postscript_name), kCFStringEncodingUTF8);
  assert(could_get_cstring);

  auto ft_library = FontManager::instance()->ft_library();

  FT_Error error;
  FT_Face ft_face;
  error = FT_New_Face(ft_library, path, 0, &ft_face);
  assert(!error);

  if (strcmp(searched_postscript_name, FT_Get_Postscript_Name(ft_face)) != 0) {
    auto num_faces = ft_face->num_faces;
    FT_Done_Face(ft_face);
    ft_face = nullptr;
    for (int face_index = 1; face_index < num_faces; ++face_index) {
      error = FT_New_Face(ft_library, path, face_index, &ft_face);
      assert(!error);

      if (strcmp(searched_postscript_name, FT_Get_Postscript_Name(ft_face)) == 0) {
        break;
      }
      FT_Done_Face(ft_face);
      ft_face = nullptr;
    }
  }
  assert(ft_face != nullptr);

  // get all the measurements in font points, we'll handle scaling by ourselves
  error = FT_Set_Char_Size(ft_face, 0, ft_face->units_per_EM, 0, 0);
  assert(!error);

  assert(!FaceContainsTable(ft_face, TTAG_morx));

  ft_face_ = ft_face;

  return ft_face;
}

bool FontDescriptor::Data::operator ==(const FontDescriptor::Data &compared_to) const {
  if (this == &compared_to || CFEqual(native_font_descriptor_.get(), compared_to.native_font_descriptor_.get())) {
    return true;
  }
  auto font_name = MakeAutoReleasedCFRef<CFStringRef>(CTFontDescriptorCopyAttribute(native_font_descriptor_.get(), kCTFontNameAttribute));
  auto font_name_to_compare_to = MakeAutoReleasedCFRef<CFStringRef>(CTFontDescriptorCopyAttribute(compared_to.native_font_descriptor_.get(), kCTFontNameAttribute));
  return CFEqual(font_name.get(), font_name_to_compare_to.get());
}


bool FontDescriptor::operator ==(const FontDescriptor &compared_to) const {
  return *data_ == *compared_to.data_;
}

FT_Face FontDescriptor::GetFTFace() const {
  assert(is_valid());
  return data_->GetFTFace();
}
hb_font_t *FontDescriptor::GetHBFont() const {
  assert(is_valid());
  return data_->GetHBFont();
}
AutoReleasedCFRef<CTFontRef> FontDescriptor::CreateNativeFont(float size) const {
  assert(is_valid());
  return data_->CreateNativeFont(size);
}
FontDescriptor::FontDescriptor(AutoReleasedCFRef<CTFontDescriptorRef> &&native_font_descriptor) : data_(std::make_shared<FontDescriptor::Data>(std::move(native_font_descriptor))) {
}


FontDescriptor FontManager::CreateDescriptorFromPostScriptName(const char *name) {
  auto cf_name = MakeAutoReleasedCFRef(CFStringCreateWithCString(kCFAllocatorDefault, name, kCFStringEncodingUTF8));
  auto basic_descriptor = MakeAutoReleasedCFRef(CTFontDescriptorCreateWithNameAndSize(cf_name.get(), 0.0));

  static const void *mandatory_attributes_values[] = { kCTFontNameAttribute };
  auto mandatory_attributes = MakeAutoReleasedCFRef(CFSetCreate(kCFAllocatorDefault, mandatory_attributes_values, sizeof(mandatory_attributes_values) / sizeof(mandatory_attributes_values[0]), nullptr));

  auto font_descriptor = MakeAutoReleasedCFRef(CTFontDescriptorCreateMatchingFontDescriptor(basic_descriptor.get(), mandatory_attributes.get()));
  if (font_descriptor.get() == nullptr) {
    return {};
  }

  return {std::move(font_descriptor)};
}

FontDescriptor FontManager::CreateDescriptorFromNativeFont(CTFontRef ct_font) {
  auto font_descriptor = MakeAutoReleasedCFRef(CTFontCopyFontDescriptor(ct_font));
  return {std::move(font_descriptor)};
}

FontManager *FontManager::instance() {
  static FontManager *instance = nullptr;
  if (instance == nullptr) {
    instance = new FontManager();
  }
  return instance;
}

FontManager::FontManager() {
  auto error = FT_Init_FreeType(&ft_library_);
  assert(!error);
}

FontManager::~FontManager() {
  FT_Done_FreeType(ft_library_);
}

}
