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

#include "test.h"

TEST(Font, FontFamilyClass) {
  using glyphknit::FontDescriptor;
  using glyphknit::FontManager;
  using glyphknit::FontFamilyClass;
  FontDescriptor descriptor;
  descriptor = FontManager::CreateDescriptorFromPostScriptName("SourceSansPro-Regular");
  ASSERT_EQ(FontFamilyClass::kSansSerif, descriptor.font_family_class());
  descriptor = FontManager::CreateDescriptorFromPostScriptName("Helvetica");
  ASSERT_EQ(FontFamilyClass::kSansSerif, descriptor.font_family_class());
  descriptor = FontManager::CreateDescriptorFromPostScriptName("LucidaGrande");
  ASSERT_EQ(FontFamilyClass::kSansSerif, descriptor.font_family_class());
  descriptor = FontManager::CreateDescriptorFromPostScriptName("HiraKakuProN-W3");
  ASSERT_EQ(FontFamilyClass::kSansSerif, descriptor.font_family_class());
  descriptor = FontManager::CreateDescriptorFromPostScriptName("BrushScriptMT");
  ASSERT_EQ(FontFamilyClass::kCursive, descriptor.font_family_class());
  descriptor = FontManager::CreateDescriptorFromPostScriptName("Webdings");
  ASSERT_EQ(FontFamilyClass::kFantasy, descriptor.font_family_class());
  descriptor = FontManager::CreateDescriptorFromPostScriptName("Courier");
  ASSERT_EQ(FontFamilyClass::kMonospace, descriptor.font_family_class());
  descriptor = FontManager::CreateDescriptorFromPostScriptName("Monaco");
  ASSERT_EQ(FontFamilyClass::kMonospace, descriptor.font_family_class());
  descriptor = FontManager::CreateDescriptorFromPostScriptName("Menlo-Regular");
  ASSERT_EQ(FontFamilyClass::kMonospace, descriptor.font_family_class());
  descriptor = FontManager::CreateDescriptorFromPostScriptName("PTSerif-Regular");
  ASSERT_EQ(FontFamilyClass::kSerif, descriptor.font_family_class());
  descriptor = FontManager::CreateDescriptorFromPostScriptName("Times-Roman");
  ASSERT_EQ(FontFamilyClass::kSerif, descriptor.font_family_class());
}
