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

#ifndef GLYPHKNIT_AUTORELEASE_H_
#define GLYPHKNIT_AUTORELEASE_H_

#include <CoreFoundation/CoreFoundation.h>

namespace glyphknit {

template <typename RefType>
class AutoReleasedCFRef {
 public:
  AutoReleasedCFRef(RefType ref) : ref_(ref) {}
  AutoReleasedCFRef(AutoReleasedCFRef<RefType> &&source) : ref_(source.ref_) {
    source.ref_ = nullptr;
  }
  AutoReleasedCFRef(const AutoReleasedCFRef<RefType> &source) : ref_(source.ref_) {
    if (ref_ != nullptr) {
      CFRetain(ref_);
    }
  }
  AutoReleasedCFRef<RefType> &operator=(AutoReleasedCFRef<RefType> &&source) {
    Release();
    ref_ = source.ref_;
    source.ref_ = nullptr;
    return *this;
  }
  AutoReleasedCFRef<RefType> &operator=(const AutoReleasedCFRef<RefType> &source) {
    if (&source != this) {
      Release();
      ref_ = source.ref_;
      CFRetain(ref_);
    }
    return *this;
  }

  RefType get() const { return ref_; }

  explicit operator RefType() { return ref_; }
  // I tried making the conversion to RefType implicit, but it was easy to do by mistake
  //   RefType x = MakeAutoReleasedCFRef(xxxx);
  // and get a reference to an object released before you could use it.

  ~AutoReleasedCFRef() { Release(); }
  void Release() {
    if (ref_ != nullptr) {
      CFRelease(ref_);
      ref_ = nullptr;
    }
  }

 private:
  RefType ref_;
};

template <typename RefType>
AutoReleasedCFRef<RefType> MakeAutoReleasedCFRef(RefType ref) {
  return {ref};
}

template <typename RefType>
AutoReleasedCFRef<RefType> MakeAutoReleasedCFRef(CFTypeRef ref) {
  return {static_cast<RefType>(ref)};
}

}

#endif  // GLYPHKNIT_AUTORELEASE_H_
