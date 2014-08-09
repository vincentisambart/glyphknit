Glyphknit
=========

Glyphknit is a library to display text.

I started development around May-June 2014 but it also contains code I had written much earlier.

It is in very early stage so I would not recommend to use it for anything serious.

The code mostly follows the Google C++ Style Guide, but uses freely C++1y features, and doesn't limit the length of lines.


Warnings
--------

- The API might change heavily without any warning.
- It is probably full of bugs.
- I am still a beginner at internationalisation so the code might be full of mistakes.


Goals
-----
Compared to using for example CoreText the differences/goals are:

- *portability*: using as possible portable open-source libraries, it should be usable on multiple platforms with as few changes as possible.
- *high-level*: it should be able to easily display vertical text, ruby (furigana), handle text storage...
- *open-source*: CoreText (or Uniscribe) are black-boxes, so it's hard to know how things work, and you can't fix bugs you might find.

And another important goal for me is to improve my knowledge of Unicode and OpenType, understanding how things work.

It is being written with interactive (editable) text as a goal. (Though in the current state it is not even good for simple static text.)


Platforms
---------

It currenly only works on OS X, but the targeted platforms target are (highest priority first):

- OS X
- iOS
- Android
- Windows

Antialiasing (at least by default) should be as close to the default of the platform as possible, so the platform's default glyph renderer should be used if possible.
The main targeted use is scalable text, so no grid fitting.


Requirements
------------

If you are crazy enough to want to build it you will find below some information to how to do it.

- *[ninja](http://martine.github.io/ninja/)* (the build tool used by example for Chrome). To install it: `brew install ninja`
- *[ICU4C](http://site.icu-project.org/)*, *[FreeType](http://www.freetype.org/)*, *[HarfBuzz](http://www.freedesktop.org/wiki/Software/HarfBuzz/)*: how-to-build-dependencies.txt quickly explains how I built those. I do have a script to build those for iOS but for the time being most of the development is using a one-architecture build for OS X.
- *[Google Test](https://code.google.com/p/googletest/)*: Nothing to compile, just checkout its code (http://googletest.googlecode.com/svn/trunk) somewhere on your hard drive and change the path to it in build.ninja.
- If you want to regenerate src/script_iterator-pairs.hh, you need a recent version of Ruby (at least 1.9). Ruby 2.0 included in the last OS X works fine. Then just run the script. The needed data files are included in the repository (in data/UCD-7.0.0)
- If you want to regenerate src/language-data.hh, you also need a recent version of Ruby, but also the Nokogiri gem. To install it just run `gem install nokogiri`. You also need to have a recent version of the [CLDR](http://cldr.unicode.org/index/downloads), [ICU4C](http://site.icu-project.org/repository), and [lang-ietf-opentype](https://github.com/jclark/lang-ietf-opentype) repositories. I am using the very last trunk of all of them to generate src/language-data.hh so you probably don't need to do it yourself.
