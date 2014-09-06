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
#include "autorelease.hh"

#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_TAGS_H
#include <CoreFoundation/CoreFoundation.h>
#include <sys/param.h>

namespace glyphknit {

struct FontFamilyClassByName {
  const char *name;
  FontFamilyClass font_family_class;
};

static FontFamilyClass ResolveFontFamilyClass(FT_Face ft_face) {
  // in fact a font could be have multiple family classes (Cursive + Serif), but we will only return the "major" one

  TT_OS2 *os2_table = static_cast<TT_OS2 *>(FT_Get_Sfnt_Table(ft_face, ft_sfnt_os2));
  if (os2_table != nullptr) {
    FontFamilyClass resolved_family_class = FontFamilyClass::kUnknown;

    // http://www.monotypeimaging.com/ProductsServices/pan1.aspx
    auto &panose = os2_table->panose;
    auto panose_family_kind = panose[0];
    if ((panose_family_kind == 2 && panose[3] == 9) || (panose_family_kind == 3 && panose[3] == 3) || (panose_family_kind == 4 && panose[3] == 9) || (panose_family_kind == 5 && panose[3] == 3)) {
      return FontFamilyClass::kMonospace;
    }
    switch (panose_family_kind) {
      case 2: { // Latin Text
        auto panose_serif_style = panose[1];
        switch (panose_serif_style) {
          case 2:  // Cove
          case 3:  // Obtuse Cove
          case 4:  // Square Cove
          case 5:  // Obtuse Square Cove
          case 6:  // Square
          case 7:  // Thin
          case 8:  // Oval
          case 9:  // Exaggerated
          case 10:  // Triangle
            resolved_family_class = FontFamilyClass::kSerif;
            break;
          case 11:  // Normal Sans
          case 12:  // Obtuse Sans
          case 13:  // Perpendicular Sans
          case 14:  // Flared
          case 15:  // Rounded
            resolved_family_class = FontFamilyClass::kSansSerif;
            break;
          default:
            break;
        }
        break;
      }
      case 3:  // Latin Hand Written
        resolved_family_class = FontFamilyClass::kCursive;
        break;
      case 4:  // Latin Decoratives
      case 5:  // Latin Symbol
        resolved_family_class = FontFamilyClass::kFantasy;
        break;
      default:
        break;
    }

    // http://www.microsoft.com/typography/otspec/os2.htm#fc
    // http://www.microsoft.com/typography/otspec/ibmfc.htm
    uint8_t tt_font_class = (os2_table->sFamilyClass >> 8) & 0xff;
    uint8_t tt_font_subclass = os2_table->sFamilyClass & 0xff;

    switch (tt_font_class) {
      case 1:  // Oldstyle Serifs
        if (tt_font_subclass == 8) {  // Calligraphic
          resolved_family_class = FontFamilyClass::kCursive;
        }
        else {
          resolved_family_class = FontFamilyClass::kSerif;
        }
        break;
      case 2:  // Transitional Serifs
      case 3:  // Modern Serifs
        if (tt_font_subclass == 2) {  // Script
          resolved_family_class = FontFamilyClass::kCursive;
        }
        else {
          resolved_family_class = FontFamilyClass::kSerif;
        }
        break;
      case 4:  // Clarendon Serifs
      case 5:  // Slab Serifs
      case 7:  // Freeform Serifs
        resolved_family_class = FontFamilyClass::kSerif;
        break;
      case 8:  // Sans Serif
        resolved_family_class = FontFamilyClass::kSansSerif;
        break;
      case 9:  // Ornamentals
      case 12:  // Symbolic
        resolved_family_class = FontFamilyClass::kFantasy;
        break;
      case 10:  // Scripts
        resolved_family_class = FontFamilyClass::kCursive;
        break;
      default:
        break;
    }

    if (resolved_family_class != FontFamilyClass::kUnknown) {
      return resolved_family_class;
    }
  }

  // we couldn't find the class just using the OS2 table, or there was no OS2 table, so we have to try with the font name
  static const FontFamilyClassByName kDefaultFontFamilyClassByName[] = {
    {"Courier",   FontFamilyClass::kMonospace},
    {"Helvetica", FontFamilyClass::kSansSerif},
    {"Times",     FontFamilyClass::kSerif},
    {"Monaco",    FontFamilyClass::kMonospace},
  };

  const char *postscript_name = FT_Get_Postscript_Name(ft_face);
  for (auto &&default_class : kDefaultFontFamilyClassByName) {
    if (strstr(postscript_name, default_class.name) != nullptr) {
      return default_class.font_family_class;
    }
  }
  return FontFamilyClass::kUnknown;
}

class FontDescriptor::Data {
 public:
  AutoReleasedCFRef<CTFontDescriptorRef> &native_font_descriptor() { return native_font_descriptor_; }
  Data(AutoReleasedCFRef<CTFontDescriptorRef> &&);
  ~Data();
  FT_Face GetFTFace() const;
  hb_font_t *GetHBFont() const;
  AutoReleasedCFRef<CTFontRef> CreateNativeFont(float size) const;
  bool operator ==(const Data &) const;
  FontFamilyClass font_family_class() const {
    GetFTFace();  // needed so that the family class is resolved
    return font_family_class_;
  }

 private:
  AutoReleasedCFRef<CTFontDescriptorRef> native_font_descriptor_;
  mutable FT_Face ft_face_;
  mutable hb_font_t *hb_font_;
  // The font family class is mutable because we need the FT_Face to be able to compute it.
  // TODO: Creating the FT_Face from the constructor might be a better idea.
  mutable FontFamilyClass font_family_class_;
};

/*
static bool FaceContainsTable(FT_Face face, FT_ULong tag) {
  FT_ULong table_length = 0;
  auto error = FT_Load_Sfnt_Table(face, tag, 0, nullptr, &table_length);
  return (error == 0);
}
*/

FontDescriptor::Data::~Data() {
  if (ft_face_ != nullptr) {
    FT_Done_Face(ft_face_);
  }
  if (hb_font_ != nullptr) {
    hb_font_destroy(hb_font_);
  }
}

FontDescriptor::Data::Data(AutoReleasedCFRef<CTFontDescriptorRef> &&native_font_descriptor) : native_font_descriptor_{native_font_descriptor}, ft_face_{nullptr}, hb_font_{nullptr}, font_family_class_(FontFamilyClass::kUnknown) {
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

//  assert(!FaceContainsTable(ft_face, TTAG_morx));

  font_family_class_ = ResolveFontFamilyClass(ft_face);
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
FontFamilyClass FontDescriptor::font_family_class() const {
  assert(is_valid());
  return data_->font_family_class();
}
FontDescriptor::FontDescriptor(AutoReleasedCFRef<CTFontDescriptorRef> &&native_font_descriptor) : data_(std::make_shared<FontDescriptor::Data>(std::move(native_font_descriptor))) {
}

static const char *kSansSerifFallbackFonts[] = {
  "Helvetica",
  //"AppleColorEmoji",
  "AppleSymbolsFB",
  "GeezaPro",
  "Thonburi",
  "Kailasa",
  "BanglaSangamMN",
  "DevanagariSangamMN",
  "GujaratiMT",
  "MonotypeGurmukhi",
  "KannadaSangamMN",
  "KhmerSangamMN",
  "LaoSangamMN",
  "MalayalamSangamMN",
  "MyanmarSangamMN",
  "OriyaSangamMN",
  "SinhalaSangamMN",
  "InaiMathi",
  "TeluguSangamMN",
  "Mshtakan",
  "EuphemiaUCAS",
  "PlantagenetCherokee",
};
static const char *kSerifFallbackFonts[] = {
  "Times-Roman",
  "Helvetica",
  //"AppleColorEmoji",
  "AppleSymbolsFB",
  "GeezaPro",
  "Thonburi",
  "Kokonor",
  "BanglaMN",
  "DevanagariMT",
  "GujaratiMT",
  "MonotypeGurmukhi",
  "KannadaMN",
  "KhmerMN",
  "LaoMN",
  "MalayalamMN",
  "MyanmarMN",
  "OriyaMN",
  "SinhalaMN",
  "InaiMathi",
  "TeluguMN",
  "Mshtakan",
  "EuphemiaUCAS",
  "PlantagenetCherokee",
};
static const char *kCursiveFallbackFonts[] = {
  "Apple-Chancery",
  //"AppleColorEmoji",
  "AppleSymbolsFB",
  "GeezaPro",
  "Thonburi",
  "DevanagariSangamMN",
  "Kokonor",
  "BanglaMN",
  "GujaratiMT",
  "MonotypeGurmukhi",
  "KannadaMN",
  "KhmerMN",
  "LaoMN",
  "MalayalamMN",
  "MyanmarMN",
  "OriyaMN",
  "SinhalaMN",
  "InaiMathi",
  "TeluguMN",
  "Mshtakan",
  "EuphemiaUCAS",
  "PlantagenetCherokee",
};
static const char *kFantasyFallbackFonts[] = {
  "Zapfino",
  //"AppleColorEmoji",
  "AppleSymbolsFB",
  "GeezaPro",
  "Thonburi",
  "DevanagariSangamMN",
  "Kokonor",
  "BanglaMN",
  "GujaratiMT",
  "MonotypeGurmukhi",
  "KannadaMN",
  "KhmerMN",
  "LaoMN",
  "MalayalamMN",
  "MyanmarMN",
  "OriyaMN",
  "SinhalaMN",
  "InaiMathi",
  "TeluguMN",
  "Mshtakan",
  "EuphemiaUCAS",
  "PlantagenetCherokee",
};
static const char *kMonospaceFallbackFonts[] = {
  "Monaco",
  //"AppleColorEmoji",
  "AppleSymbolsFB",
  "Helvetica",
  "Baghdad",
  "Ayuthaya",
  "Kailasa",
  "BanglaSangamMN",
  "DevanagariSangamMN",
  "GujaratiMT",
  "MonotypeGurmukhi",
  "KannadaSangamMN",
  "KhmerSangamMN",
  "LaoSangamMN",
  "MalayalamSangamMN",
  "MyanmarSangamMN",
  "OriyaSangamMN",
  "SinhalaSangamMN",
  "InaiMathi",
  "TeluguSangamMN",
  "Mshtakan",
  "EuphemiaUCAS",
  "PlantagenetCherokee",
};

FontDescriptor FontDescriptor::GetFallback(size_t index, Language language) {
  assert(is_valid());
  if (index == 0) {
    return *this;
  }
  --index;

  auto klass = font_family_class();

  const char *font_name = nullptr;
  if (language.language_code == MakeTag('j','a')) {
    switch (klass) {
      case FontFamilyClass::kSerif:
      case FontFamilyClass::kCursive:
      case FontFamilyClass::kFantasy:
        font_name = "HiraMinProN-W3";
        break;
      default:
        font_name = "HiraKakuProN-W3";
        break;
    }
  }
  else if (language.language_code == MakeTag('z','h')) {
    if (language.opentype_tag == MakeTag('Z','H','S')) {
      switch (klass) {
        case FontFamilyClass::kSerif:
          font_name = "STSongti-SC-Regular";
          break;
        case FontFamilyClass::kCursive:
        case FontFamilyClass::kFantasy:
          font_name = "STKaiti-SC-Regular";
          break;
        default:
          font_name = "STHeitiSC-Light";
          break;
      }
    }
    else if (language.opentype_tag == MakeTag('Z','H','T') || language.opentype_tag == MakeTag('Z','H','H')) {
      switch (klass) {
        case FontFamilyClass::kSerif:
          font_name = "STSongti-TC-Regular";
          break;
        case FontFamilyClass::kCursive:
        case FontFamilyClass::kFantasy:
          font_name = "DFKaiShu-SB-Estd-BF";
          break;
        default:
          font_name = "STHeitiTC-Light";
          break;
      }
    }
  }
  else if (language.language_code == MakeTag('k','o')) {
    switch (klass) {
      case FontFamilyClass::kSerif:
      case FontFamilyClass::kCursive:
      case FontFamilyClass::kFantasy:
        font_name = "AppleMyungjo";
        break;
      default:
        font_name = "AppleSDGothicNeo-Regular";
        break;
    }
  }
  if (font_name) {
    if (index == 0) {
      return FontManager::CreateDescriptorFromPostScriptName(font_name);
    }
    --index;
  }

  size_t fallbacks_count;
  const char **fallbacks;
  switch (klass) {
    case FontFamilyClass::kSerif: {
      fallbacks = kSerifFallbackFonts;
      fallbacks_count = std::end(kSerifFallbackFonts) - std::begin(kSerifFallbackFonts);
      break;
    }
    case FontFamilyClass::kMonospace: {
      fallbacks = kMonospaceFallbackFonts;
      fallbacks_count = std::end(kMonospaceFallbackFonts) - std::begin(kMonospaceFallbackFonts);
      break;
    }
    case FontFamilyClass::kCursive: {
      fallbacks = kCursiveFallbackFonts;
      fallbacks_count = std::end(kCursiveFallbackFonts) - std::begin(kCursiveFallbackFonts);
      break;
    }
    case FontFamilyClass::kFantasy: {
      fallbacks = kFantasyFallbackFonts;
      fallbacks_count = std::end(kFantasyFallbackFonts) - std::begin(kFantasyFallbackFonts);
      break;
    }
    default: {
      fallbacks = kSansSerifFallbackFonts;
      fallbacks_count = std::end(kSansSerifFallbackFonts) - std::begin(kSansSerifFallbackFonts);
      break;
    }
  }
  if (index < fallbacks_count) {
    font_name = fallbacks[index];
  }
  else if (index == fallbacks_count) {
    font_name = ".LastResort";
  }
  else {
    std::abort();
  }
  return FontManager::CreateDescriptorFromPostScriptName(font_name);
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
