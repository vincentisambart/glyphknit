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

#ifndef GLYPHKNIT_LANGUAGE_H_
#define GLYPHKNIT_LANGUAGE_H_

#include <unicode/uscript.h>
#include "tag.hh"

namespace glyphknit {

struct Language {
  Tag language_code;
  Tag opentype_tag;

  bool operator ==(const Language &language) const {
    return language.language_code == this->language_code && language.opentype_tag == this->opentype_tag;
  }
};

static const Language kLanguageUnknown = {.language_code = kTagUnknown, .opentype_tag = kOpenTypeTagDefaultLanguage};

bool IsScriptUsedForLanguage(UScriptCode script, Language language);
Language GetPredominantLanguageForScript(UScriptCode script);
Language FindLanguageCodeAndOpenTypeLanguageTag(const char *language, ssize_t length = -1);
Language GuessLanguageFromScript(UScriptCode script);

}

#endif  // GLYPHKNIT_LANGUAGE_H_
