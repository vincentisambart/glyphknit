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

#include <cstring>
#include <algorithm>
#include <functional>

#include "utf.hh"
#include "script_iterator.hh"

namespace glyphknit {

#include "script_iterator-pairs.hh"

static
UScriptCode GetSimplifiedScript(UChar32 const codepoint) {
  UErrorCode errorCode = U_ZERO_ERROR; // we don't care about the error code
  auto script = uscript_getScript(codepoint, &errorCode);
  if (script == USCRIPT_HIRAGANA) {
    // Handle hiragana and katakana as one single script, as OpenType also does not differenciate them.
    // Note that we can't use USCRIPT_KATAKANA_OR_HIRAGANA as it would not work with uscript_hasScript.
    return USCRIPT_KATAKANA;
  }
  else if (script == USCRIPT_INVALID_CODE) {
    return USCRIPT_INHERITED;
  }
  return script;
}

template <typename Callable>
void IfIsPossiblePairEnd(UChar32 const codepoint, Callable to_call) {
  auto last = std::end(kPairEnds);
  auto first_possible_pair = std::lower_bound(std::begin(kPairEnds), last, codepoint, [](const auto &element, const auto &value) {
    return element.end_codepoint < value;
  });
  auto last_possible_pair = first_possible_pair;
  while (last_possible_pair != last && last_possible_pair->end_codepoint == codepoint) {
    ++last_possible_pair;
  }
  if (last_possible_pair != first_possible_pair) {
    to_call(first_possible_pair, last_possible_pair);
  }
}

static
bool IsPairStart(UChar32 const codepoint) {
  return std::binary_search(std::begin(kPairStarts), std::end(kPairStarts), codepoint);
}

static
bool IsScriptFixed(UScriptCode const script) {
  return script != USCRIPT_COMMON && script != USCRIPT_INHERITED;
}

UScriptCode ScriptIterator::FindNextFixedScript() const {
  auto index = current_index_; // do not move the main cursor
  while (index < length_) {
    UChar32 const codepoint = ConsumeCodepoint(text_, length_, index);
    UScriptCode const script = GetSimplifiedScript(codepoint);
    if (IsScriptFixed(script)) {
      return script;
    }
  }
  return USCRIPT_COMMON;
}

ScriptIterator::Run ScriptIterator::FindNextRun() {
  if (last_script_ == USCRIPT_COMMON && run_start_ == 0 && length_ > 0) {
    // if the text only contains codepoints of script Common or Inherited, only one run
    current_index_ = run_start_ = length_;
    return ScriptIterator::Run{.script = USCRIPT_COMMON, .start = 0, .end = length_};
  }

  while (current_index_ < length_) {
    auto codepoint_start = current_index_;
    auto codepoint = ConsumeCodepoint(text_, length_, current_index_);
    UScriptCode script = GetSimplifiedScript(codepoint);

    if (script == USCRIPT_INHERITED) {
      script = last_script_;
    }
    else if (script == USCRIPT_COMMON) {
      IfIsPossiblePairEnd(codepoint, [this, &script](auto first_possible_pair, auto last_possible_pair) {
        for (int stack_index = stack_length_ - 1; stack_index >= 0; --stack_index) {
          auto stack_codepoint = pair_starts_[stack_index].codepoint;
          if (std::any_of(first_possible_pair, last_possible_pair, [=](auto pair) { return pair.start_codepoint == stack_codepoint; })) {
            script = pair_starts_[stack_index].script;
            stack_length_ = stack_index;
            break;
          }
        }
      });

      if (script == USCRIPT_COMMON) {
        if (uscript_hasScript(codepoint, last_script_)) {
          script = last_script_;
        }
        else {
          auto next_fixed_script = FindNextFixedScript();
          if (next_fixed_script != USCRIPT_COMMON && uscript_hasScript(codepoint, next_fixed_script)) {
            script = next_fixed_script;
          }
          else {
            script = last_script_;
          }
        }

        if (IsPairStart(codepoint)) {
          if (stack_length_ == kStackSize) {
            std::copy(std::begin(pair_starts_) + 1, std::end(pair_starts_), std::begin(pair_starts_));
          }
          else {
            ++stack_length_;
          }
          auto new_element = stack_length_-1;
          pair_starts_[new_element].script = script;
          pair_starts_[new_element].codepoint = codepoint;
        }
      }
    }
    if (script != last_script_) {
      auto run = ScriptIterator::Run{.script = last_script_, .start = run_start_, .end = codepoint_start};
      run_start_ = codepoint_start;
      last_script_ = script;
      return run;
    }
  }
  if (run_start_ == length_) {
    return ScriptIterator::Run{.script = USCRIPT_INVALID_CODE, .start = length_, .end = length_};
  }
  else {
    auto run = ScriptIterator::Run{.script = last_script_, .start = run_start_, .end = length_};
    run_start_ = length_;
    return run;
  }
}

}
