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

#include "language.hh"

#include "test.h"

static glyphknit::Language L(const char *language_code = nullptr, const char *opentype_tag = nullptr) {
  return glyphknit::Language{
    .language_code = language_code == nullptr ? glyphknit::kTagUnknown : glyphknit::MakeTag(language_code),
    .opentype_tag = opentype_tag == nullptr ? glyphknit::kOpenTypeTagDefaultLanguage : glyphknit::MakeTag(opentype_tag),
  };
}

TEST(IsScriptUsedForLanguage, BasicTest) {
  using glyphknit::MakeTag;
  using glyphknit::IsScriptUsedForLanguage;
  using glyphknit::Language;

  ASSERT_TRUE(IsScriptUsedForLanguage(USCRIPT_HIRAGANA, L("ja", "JAN")));
  ASSERT_FALSE(IsScriptUsedForLanguage(USCRIPT_LATIN, L("ja", "JAN")));
  ASSERT_TRUE(IsScriptUsedForLanguage(USCRIPT_LATIN, L("ja", "IPPH")));
}

TEST(GetPredominantLanguageForScript, BasicTest) {
  using glyphknit::GetPredominantLanguageForScript;

  ASSERT_EQ(L(), GetPredominantLanguageForScript(USCRIPT_LATIN));
  ASSERT_EQ(L(), GetPredominantLanguageForScript(UScriptCode(-9)));
  ASSERT_EQ(L(), GetPredominantLanguageForScript(UScriptCode(9999)));
  ASSERT_EQ(L("ja", "JAN"), GetPredominantLanguageForScript(USCRIPT_HIRAGANA));
}

// The test suite below comes fully from lang-ietf-opentype
TEST(FindLanguageCodeAndOpenTypeLanguageTag, TestsFromLangIetfOpenType) {
  using glyphknit::FindLanguageCodeAndOpenTypeLanguageTag;

  ASSERT_EQ(L("en", "ENG"), FindLanguageCodeAndOpenTypeLanguageTag("en"));
  ASSERT_EQ(L("en", "ENG"), FindLanguageCodeAndOpenTypeLanguageTag("EN"));
  ASSERT_EQ(L("en", "ENG"), FindLanguageCodeAndOpenTypeLanguageTag("en-US"));
  // Arabic
  ASSERT_EQ(L("ar", "ARA"), FindLanguageCodeAndOpenTypeLanguageTag("ar"));
  ASSERT_EQ(L("arb", "ARA"), FindLanguageCodeAndOpenTypeLanguageTag("arb"));
  ASSERT_EQ(L("ar", "GAR"), FindLanguageCodeAndOpenTypeLanguageTag("ar-Syrc"));
  ASSERT_EQ(L("ar", "GAR"), FindLanguageCodeAndOpenTypeLanguageTag("ar-arb-Syrc"));
  // North Levantine spoken Arabic (comes from macrolanguage expansion)
  ASSERT_EQ(L("apc", "ARA"), FindLanguageCodeAndOpenTypeLanguageTag("apc"));
  // Don"t recognize as GAR
  ASSERT_EQ(L("apc", "ARA"), FindLanguageCodeAndOpenTypeLanguageTag("apc-Syrc"));
  ASSERT_EQ(L("ary", "MOR"), FindLanguageCodeAndOpenTypeLanguageTag("ary"));
  ASSERT_EQ(L("ar", "MOR"), FindLanguageCodeAndOpenTypeLanguageTag("ar-ary"));
  // Chinese
  ASSERT_EQ(L("zh", "ZHS"), FindLanguageCodeAndOpenTypeLanguageTag("zh-CN"));
  ASSERT_EQ(L("zh", "ZHH"), FindLanguageCodeAndOpenTypeLanguageTag("zh-HK"));
  ASSERT_EQ(L("zh", "ZHH"), FindLanguageCodeAndOpenTypeLanguageTag("ZH-hk"));
  ASSERT_EQ(L("zh", "ZHT"), FindLanguageCodeAndOpenTypeLanguageTag("zh-Hant-x-HK"));
  ASSERT_EQ(L("zh", "ZHT"), FindLanguageCodeAndOpenTypeLanguageTag("zh-MO"));
  ASSERT_EQ(L("zh", "ZHS"), FindLanguageCodeAndOpenTypeLanguageTag("zh-SG"));
  ASSERT_EQ(L("zh", "ZHT"), FindLanguageCodeAndOpenTypeLanguageTag("zh-TW"));
  ASSERT_EQ(L("zh", "ZHS"), FindLanguageCodeAndOpenTypeLanguageTag("zh-Hans"));
  ASSERT_EQ(L("zh", "ZHT"), FindLanguageCodeAndOpenTypeLanguageTag("zh-Hant"));
  ASSERT_EQ(L("zh", "ZHH"), FindLanguageCodeAndOpenTypeLanguageTag("zh-Hant-HK"));
  ASSERT_EQ(L("zh", "ZHS"), FindLanguageCodeAndOpenTypeLanguageTag("zh-Hans-HK"));
  ASSERT_EQ(L("zh", "ZHH"), FindLanguageCodeAndOpenTypeLanguageTag("zh-yue-Hant-HK"));
  // Yue (of which Cantonese is a dialect) is a component of the Chinese macrolanguage
  ASSERT_EQ(L("yue", "ZHH"), FindLanguageCodeAndOpenTypeLanguageTag("yue-HK"));
  ASSERT_EQ(L("yue", "ZHH"), FindLanguageCodeAndOpenTypeLanguageTag("yue-Hant-HK"));
  ASSERT_EQ(L("yue", "ZHT"), FindLanguageCodeAndOpenTypeLanguageTag("yue-Hant"));
  ASSERT_EQ(L("yue", "ZHS"), FindLanguageCodeAndOpenTypeLanguageTag("yue"));
  // Greek
  ASSERT_EQ(L("el", "ELL"), FindLanguageCodeAndOpenTypeLanguageTag("el"));
  ASSERT_EQ(L("el", "PGR"), FindLanguageCodeAndOpenTypeLanguageTag("el-polyton"));
  ASSERT_EQ(L("el", "PGR"), FindLanguageCodeAndOpenTypeLanguageTag("el-Grek-GR-polyton-x-wow"));
  // Cree
  ASSERT_EQ(L("cwd", "DCR"), FindLanguageCodeAndOpenTypeLanguageTag("cwd"));
  // Others
  ASSERT_EQ(L("ijc", "IJO"), FindLanguageCodeAndOpenTypeLanguageTag("ijc"));
  // IPA
  ASSERT_EQ(L("und", "IPPH"), FindLanguageCodeAndOpenTypeLanguageTag("und-fonipa"));
  ASSERT_EQ(L("en", "IPPH"), FindLanguageCodeAndOpenTypeLanguageTag("en-fonipa"));
  ASSERT_EQ(L("zh", "IPPH"), FindLanguageCodeAndOpenTypeLanguageTag("zh-fonipa"));
  ASSERT_EQ(L("ary", "IPPH"), FindLanguageCodeAndOpenTypeLanguageTag("ary-fonipa"));
  ASSERT_EQ(L("en", "IPPH"), FindLanguageCodeAndOpenTypeLanguageTag("en-US-fonipa"));
  ASSERT_EQ(L("ijc", "IPPH"), FindLanguageCodeAndOpenTypeLanguageTag("ijc-fonipa"));

  ASSERT_EQ(L("und"), FindLanguageCodeAndOpenTypeLanguageTag("und"));
}

TEST(FindLanguageCodeAndOpenTypeLanguageTag, Other) {
  using glyphknit::FindLanguageCodeAndOpenTypeLanguageTag;
  // separator not limited to "-"
  ASSERT_EQ(L("zh", "ZHH"), FindLanguageCodeAndOpenTypeLanguageTag("zh_HK"));
  ASSERT_EQ(L("zh", "ZHS"), FindLanguageCodeAndOpenTypeLanguageTag("zh_Hans_TW"));
  ASSERT_EQ(L("zh", "ZHS"), FindLanguageCodeAndOpenTypeLanguageTag("zh_ANYTHING"));
  ASSERT_EQ(L("fr", "FRA"), FindLanguageCodeAndOpenTypeLanguageTag("fr_FR"));
  ASSERT_EQ(L("ja", "JAN"), FindLanguageCodeAndOpenTypeLanguageTag("ja"));
}
