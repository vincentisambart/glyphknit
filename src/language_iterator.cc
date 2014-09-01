#include "language_iterator.hh"

#include <iostream>

namespace glyphknit {

void LanguageIterator::FindNextScriptRun() {
  script_run_ = script_iterator_.FindNextRun();
  default_language_ = GuessLanguageFromScript(script_run_.script);
  previous_language_ = default_language_;
}

// TODO: always use offsets in the full text (in ScriptIterator), not just local ones
LanguageIterator::Run LanguageIterator::FindNextRun() {
  while (script_run_.start + start_index_ < end_index_) {
    if (run_start_ >= script_run_.end) {
      FindNextScriptRun();
    }
    auto script_run_start = script_run_.start + start_index_;
    auto script_run_end = script_run_.end + start_index_;
    while (attributes_run_it_->end <= script_run_end) {
      auto current_language = attributes_run_it_->attributes.language;
      if (current_language.is_undefined() || !IsScriptUsedForLanguage(script_run_.script, current_language)) {
        current_language = default_language_;
      }
      if (current_language != previous_language_) {
        auto run_end = std::max(attributes_run_it_->start, script_run_start);
        if (run_start_ < run_end) {
          LanguageIterator::Run language_run = {
            .script = script_run_.script,
            .language = previous_language_,
            .start = run_start_,
            .end = run_end,
          };
          previous_language_ = current_language;
          run_start_ = run_end;
          return language_run;
        }
        previous_language_ = current_language;
      }
      ++attributes_run_it_;
    }
    if (run_start_ < script_run_end) {
      Language language;
      if (attributes_run_it_ == text_block_.attributes_runs().end()) {
        language = previous_language_;
      }
      else {
        language = attributes_run_it_->attributes.language;
        if (language.is_undefined() || !IsScriptUsedForLanguage(script_run_.script, language)) {
          language = default_language_;
        }
      }
      LanguageIterator::Run language_run = {
        .script = script_run_.script,
        .language = language,
        .start = run_start_,
        .end = script_run_end,
      };
      run_start_ = script_run_end;
      return language_run;
    }
  }

  LanguageIterator::Run language_run = {
    .script = USCRIPT_INVALID_CODE,
    .language = Language{},
    .start = end_index_,
    .end = end_index_,
  };

  return language_run;
}


}
