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

#include <CoreFoundation/CoreFoundation.h>

#include <cstdio>
#include <iostream>  // for debugging
#include <vector>

#include "autorelease.hh"
#include "language.hh"

namespace glyphknit {

static
bool IsASCIILetter(char c) {
  return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

// Must only be called on ASCII letters
static
char ToLower(char c) {
  return c | 0x20;
}

static
bool AreStringEqualCaseInsensitive(const char *str1, const char *str2, int length) {
  for (int i = 0; i < length; ++i) {
    if (ToLower(str1[i]) != ToLower(str2[i])) {
      return false;
    }
  }
  return true;
}

#include "language-data.hh"

static
bool IsScriptValid(UScriptCode script) {
  return (script >= 0 && script < USCRIPT_CODE_LIMIT);
}

Language GetPredominantLanguageForScript(UScriptCode script) {
  if (!IsScriptValid(script)) {
    return kLanguageUnknown;
  }
  return kLikelyLanguageForScripts[script];
}

Language FindLanguageCodeAndOpenTypeLanguageTag(const char *language, ssize_t length) {
  if (length == -1) {
    length = std::strlen(language);
  }
  ssize_t index = 0;
  while (index < length && IsASCIILetter(language[index])) {
    ++index;
  }
  Tag language_tag;
  if (index == 2) {
    language_tag = MakeTag(ToLower(language[0]), ToLower(language[1]));
  }
  else if (index == 3) {
    language_tag = MakeTag(ToLower(language[0]), ToLower(language[1]), ToLower(language[2]));
  }
  else {
    // country codes must be 2 or 3 letters long
    return kLanguageUnknown;
  }

  auto end = std::end(kOpenTypeTagPerLanguage);
  auto default_opentype_tag = std::lower_bound(std::begin(kOpenTypeTagPerLanguage), end, language_tag, [](const auto &element, const auto &value) {
    return element.language < value;
  });

  // fast path
  if (default_opentype_tag != end && default_opentype_tag->language == language_tag && length == index) {
    return {.language_code = language_tag, .opentype_tag = default_opentype_tag->opentype_tag};
  }

  uint32_t condition_flags = OPENTYPE_CONDITION_FLAG_DEFAULT;
  while (index < length) {
    while (index < length && !IsASCIILetter(language[index])) {
      ++index;
    }
    if (index == length) {
      break;
    }
    auto subtag_start_offset = index;
    while (index < length && IsASCIILetter(language[index])) {
      ++index;
    }
    auto subtag_end_offset = index;
    auto subtag_length = subtag_end_offset - subtag_start_offset;
    if (subtag_length == 1) {
      break;
    }
    else if (subtag_length == 6) {
      if (AreStringEqualCaseInsensitive(&language[subtag_start_offset], "fonipa", 6)) {
        return {.language_code = language_tag, .opentype_tag = kOpenTypeTagPhoneticTranscription};
      }
    }
    else {
      condition_flags |= GetOpenTypeTagConditionFlag(&language[subtag_start_offset], subtag_length);
    }
  }

  // the check has to be done after the previous look to allow unknown languages with -fonipa
  if (default_opentype_tag == end || default_opentype_tag->language != language_tag) {
    return {.language_code = language_tag, .opentype_tag = kOpenTypeTagDefaultLanguage};
  }

  for (auto opentype_condition = default_opentype_tag + 1; opentype_condition != end && opentype_condition->language == language_tag; ++opentype_condition) {
    if (opentype_condition->condition & condition_flags) {
      return {.language_code = language_tag, .opentype_tag = opentype_condition->opentype_tag};
    }
  }

  return {.language_code = language_tag, .opentype_tag = default_opentype_tag->opentype_tag};
}

bool IsScriptUsedForLanguage(UScriptCode script, Language language) {
  if (!IsScriptValid(script)) {
    return false;
  }
  if (script == USCRIPT_LATIN && language.opentype_tag == kOpenTypeTagPhoneticTranscription) {
    // phonetic transcription is using IPA, in Latin script
    return true;
  }
  const auto &languages = kLanguagesUsingScript[script];
  if (languages.count == 0) {
    return false;
  }
  auto start = &kLanguagesUsing[languages.start_index];
  auto end = start + languages.count;
  // TODO: maybe limit binary search to scripts used in many languages (like Latin)
  return std::binary_search(start, end, language.language_code);
}

typedef std::vector<Language> PreferredLanguages;

static
const PreferredLanguages &GetPreferredLanguages() {
  static PreferredLanguages *preferred_languages = nullptr;

  if (preferred_languages != nullptr) {
    return *preferred_languages;
  }

  auto new_preferred_languages = new PreferredLanguages{};

  auto system_preferred_languages = MakeAutoReleasedCFRef(CFLocaleCopyPreferredLanguages());
  assert(system_preferred_languages.get() != nullptr);
  auto count = CFArrayGetCount(system_preferred_languages.get());
  char buffer[200];
  for (ssize_t i = 0; i < count; ++i) {
    auto language = static_cast<CFStringRef>(CFArrayGetValueAtIndex(system_preferred_languages.get(), i));
    auto could_convert = CFStringGetCString(language, buffer, sizeof(buffer), kCFStringEncodingUTF8);
    // the buffer should not be too small for a language string so just ignore the entry
    if (could_convert) {
      new_preferred_languages->push_back(FindLanguageCodeAndOpenTypeLanguageTag(buffer));
    }
  }

  preferred_languages = new_preferred_languages;
  return *preferred_languages;
}

Language GuessLanguageFromScript(UScriptCode script) {
  const auto &preferred_languages = GetPreferredLanguages();
  for (const auto &language : preferred_languages) {
    if (IsScriptUsedForLanguage(script, language)) {
      return language;
    }
  }
  return GetPredominantLanguageForScript(script);
}

}
