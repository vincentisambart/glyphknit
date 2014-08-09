#!/usr/bin/ruby

# Copyright © 2014  Vincent Isambart
#
#  This file is part of Glyphknit.
#
# Permission is hereby granted, without written agreement and without
# license or royalty fees, to use, copy, modify, and distribute this
# software and its documentation for any purpose, provided that the
# above copyright notice and the following two paragraphs appear in
# all copies of this software.
#
# IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
# DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
# ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
# IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#
# THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
# BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
# FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
# ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
# PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

require "set"
require "strscan"
require "nokogiri"
require "json"
require_relative "lib/helper"

if ARGV.length != 3
  STDERR.puts "Syntax: #{$0} path_to_cldr path_to_icu path_to_lang-ietf-opentype"
  exit 1
end
path_to_cldr = ARGV[0]
path_to_icu = ARGV[1]
path_to_lang_ietf_opentype = ARGV[2]

en_language_path = File.join(path_to_cldr, "common/main/en.xml")
unless File.exist?(en_language_path)
  STDERR.puts "cannot find the CLDR language files in #{path_to_cldr} (for example #{en_language_path})"
  exit 1
end

icu_script_header_path = File.join(path_to_icu, "source/common/unicode/uscript.h")
unless File.exist?(icu_script_header_path)
  STDERR.puts "cannot find the ICU script header file (#{icu_script_header_path}) in #{path_to_icu}"
  exit 1
end

lang_ietf_opentype_map_path = File.join(path_to_lang_ietf_opentype, "lib/map.json")
unless File.exist?(lang_ietf_opentype_map_path)
  STDERR.puts "cannot find the ICU script header file (#{lang_ietf_opentype_map_path}) in #{path_to_lang_ietf_opentype}"
  exit 1
end

def tag_value(tag)
  tag = tag + (" " * (4-tag.length))
  tag.codepoints.to_a.inject(0) {|memo, obj| memo << 8 | obj }
end

scripts = load_script_for_codepoints

# an ugly quickly written parser for the CLDR examplar character sets
# note: upper case letters are also in the set, but that doesn't change the script so no interest to us here
def parse_examplar_characters(examplars)
  raise "unknown format for #{examplar_characters}" unless /\A\[.*\]\z/.match(examplars)
  examplars = examplars.gsub(/\A\[|\]\z/, "").gsub(" ", "").gsub(/\\u([0-9A-F]+)/i) { $1.hex.chr(Encoding::UTF_8) }
  ss = StringScanner.new(examplars)
  characters = []
  was_range = false
  while !ss.eos?
    if ss.scan(/\\/)
      characters << ss.scan(/./)
    elsif ss.scan(/-/)
      was_range = true
      next
    elsif ss.scan(/{/)
      characters << ss.scan(/[^}]+/)
      ss.scan(/}/)
    else
      characters << ss.scan(/./)
    end
    if was_range
      range_end = characters.pop
      range_start = characters.pop
      if range_end.length != 1 or range_start.length != 1
        raise "invalid range from #{range_start} to #{range_end}"
      end
      range_start_codepoint = range_start.codepoints[0]
      range_end_codepoint = range_end.codepoints[0]
      (range_start_codepoint..range_end_codepoint).each do |codepoint|
        characters << codepoint.chr(Encoding::UTF_8)
      end
      was_range = false
    end
  end
  characters
end

# makes a list of scripts used for each language using the examplar character sets
# from the Unicode CLDR and the script attribute of each character
scripts_per_language = {}
languages_per_script = {}
Dir.glob(File.join(path_to_cldr, "common/main/*.xml")) do |path|
  document = Nokogiri::XML(File.read(path))
  language = document.at_xpath("/ldml/identity/language")["type"]
  document.xpath("/ldml/characters/exemplarCharacters").each do |exemplar_characters_node|
    type = exemplar_characters_node["type"]
    # the "index" examplar characters add Latn to Chinese
    # the "auxiliary" examplar characters add Hani to Korean
    # the "punctuation" examplar characters don't add anything
    next if %w{index punctuation}.include?(type)
    characters = parse_examplar_characters(exemplar_characters_node.text)
    characters.each do |c|
      codepoint = c.codepoints[0]
      script = scripts[codepoint]
      next if %w{Zinh Zyyy Zzzz}.include?(script)
      scripts_per_language[language] ||= Set.new
      scripts_per_language[language] << script
      languages_per_script[script] ||= Set.new
      languages_per_script[script] << language
    end
  end
end

icu_scripts = []
File.open(icu_script_header_path) do |f|
  started = false
  limit = nil
  icu_scripts = []
  f.each_line do |line|
    line = line.strip
    next if line.empty?
    if line == "typedef enum UScriptCode {"
      started = true
      next
    end
    next unless started
    break if line == "} UScriptCode;"

    if md = %r{\AUSCRIPT_CODE_LIMIT\s+=\s+([0-9]+)}.match(line)
      limit = md[1].to_i
    elsif md = %r{\A(USCRIPT_[A-Z_]+)\s+=\s+([0-9]+),\s*/\*\s*([A-Z][a-z]{3})\s*\*/}.match(line)
      constant_name, value, script_code = md[1], md[2].to_i, md[3]
      icu_scripts[value] = {constant: constant_name, short_name: script_code}
    end
  end
  raise "could not find the UScriptCode enum in #{icu_script_header_path}" unless started
  limit.times do |value|
    raise "no script for index #{value}" unless icu_scripts[value]
  end
end

script_unused_in_properties = Set.new
scripts_used = scripts.values.uniq
load_all_scripts.each do |short_name, long_name|
  script_unused_in_properties << short_name unless scripts_used.include?(short_name)
end

GROUPING_SCRIPT_DECOMPOSITIONS = {
  "Kore" => %w{Hani Hang},
  "Jpan" => %w{Hani Hira Kana},
  "Hrkt" => %w{Hira Kana},
}

SCRIPT_VARIANTS = {
  "Hant" => "Hani",
  "Hans" => "Hani",
  "Cyrs" => "Cyrl",
  "Latf" => "Latn",
  "Latg" => "Latn",
  "Syre" => "Syrc",
  "Syrj" => "Syrc",
  "Syrn" => "Syrc",
}

likely_languages = {}
path = File.join(path_to_cldr, "common/supplemental/likelySubtags.xml")
document = Nokogiri::XML(File.read(path))
document.xpath("/supplementalData/likelySubtags/likelySubtag").each do |subtag|
  from, to = subtag["from"], subtag["to"]

  from_language, from_script, from_country = from.split("_")
  to_language, to_script, to_country = to.split("_")

  if from_language == "und" and from_script != nil and from_country == nil and /\A[A-Z][a-z]{3}\z/.match(from_script)
    likely_languages[from_script] = to_language unless script_unused_in_properties.include?(from_script)
  end

  if GROUPING_SCRIPT_DECOMPOSITIONS[to_script]
    scripts_used_as_properties = GROUPING_SCRIPT_DECOMPOSITIONS[to_script]
  elsif SCRIPT_VARIANTS[to_script]
    scripts_used_as_properties = [ SCRIPT_VARIANTS[to_script] ]
  else
    scripts_used_as_properties = [ to_script ]
  end

  scripts_used_as_properties.each do |script|
    scripts_per_language[to_language] ||= Set.new
    scripts_per_language[to_language] << script
    languages_per_script[script] ||= Set.new
    languages_per_script[script] << to_language
  end
end

lang_ietf_opentype_map = JSON.parse(File.read(lang_ietf_opentype_map_path))
opentype_condition_types = Set.new
lang_ietf_opentype_map.values.each do |conditions|
  conditions = [ conditions ].flatten
  default = conditions.shift
  conditions.each_slice(2) do |value, ot_tag|
    opentype_condition_types << value
  end
end

OPENTYPE_CONDITION_FLAG_PREFIX = "OPENTYPE_CONDITION_FLAG_"
OPENTYPE_CONDITION_FLAG_MAX_LENGTH = (opentype_condition_types.to_a + [ "DEFAULT" ]).map{|x| OPENTYPE_CONDITION_FLAG_PREFIX.length + x.length }.max
def aligned_opentype_condition_flag(name)
  long_name = OPENTYPE_CONDITION_FLAG_PREFIX + name.upcase
  long_name + (" " * (OPENTYPE_CONDITION_FLAG_MAX_LENGTH - long_name.length))
end

def make_tag(tag, align_on = 4)
  spaces_to_add = (align_on - tag.length) * 4
  spaces_to_add = 0 if spaces_to_add < 0
  "MakeTag('" + tag.split(//).join("','") + "'" + (' ' * spaces_to_add) + ")"
end

class Array
  def sort_by_tag_value
    sort_by {|x| tag_value(x) }
  end
end

output_file_short_name = "language-data.hh"
output_file_path = in_src_dir(output_file_short_name)
File.open(output_file_path, "w") do |output_file|
  output_file.puts <<-ENDSTR
// this file should only be included by language.cc
// file automatically generated by scripts/#{File.basename(__FILE__)}, do not edit

static_assert(USCRIPT_CODE_LIMIT <= #{icu_scripts.length}, "USCRIPT_CODE_LIMIT is bigger than expected. You need to regenerate #{output_file_short_name} with the version of ICU you are using.");
// The language text from a script is most likely to be in if you don't have any other information.
// The indices of kLikelyLanguageForScripts are USCRIPT_ values.
// Note that most of the unknowns are for scripts we don't care about here as they are never used as Unicode properties.
static const Language kLikelyLanguageForScripts[] = {
  ENDSTR

  icu_scripts.each do |script|
    likely = likely_languages[script[:short_name]]
    if likely
      # some old scripts have a default language (not used anymore) that has no OpenType tag so use the default language tag instead
      opentype_tag = [ lang_ietf_opentype_map[likely] ].flatten.first || "dflt"
      output_file.puts "  { #{make_tag(likely, 3)}, #{make_tag(opentype_tag)} },  // #{script[:constant]}"
    else
      output_file.puts "  { kTagUnknown         , #{make_tag("dflt")} },  // #{script[:constant]}"
    end
  end

  output_file.puts <<-ENDSTR
};

// The OpenType tag condition code is based on data and code from lang-ietf-opentype
// https://github.com/jclark/lang-ietf-opentype
enum OpenTypeTagCondition : uint32_t {
  ENDSTR

  raise "the conditions type must be made bigger" if opentype_condition_types.length > 32

  output_file.puts "  #{aligned_opentype_condition_flag("DEFAULT")} = 0,"
  opentype_condition_types.to_a.sort.each_with_index do |condition_type, i|
    output_file.puts "  #{aligned_opentype_condition_flag(condition_type)} = #{2 ** i},"
  end

  output_file.puts <<-ENDSTR
};

static
uint32_t GetOpenTypeTagConditionFlag(const char *subtag, ssize_t length) {
  // this could be made a bit faster (especially when length is 4), but probably not worth it
  switch (length) {
  ENDSTR

  opentype_condition_types_by_length = opentype_condition_types.to_a.group_by(&:length)
  opentype_condition_types_by_length.sort_by {|length, types| length }.each do |length, types|
    output_file.puts "    case #{length}:"
    types.sort.each_with_index do |type, i|
      type = type[0..0].upcase + type[1..-1].downcase if type.length == 4
      output_file.puts <<-ENDSTR
      #{i == 0 ? "" : "else "}if (AreStringEqualCaseInsensitive(subtag, "#{type}", #{length})) {
        return #{OPENTYPE_CONDITION_FLAG_PREFIX + type.upcase};
      }
      ENDSTR
    end
    output_file.puts "      break;"
  end

  output_file.puts <<-ENDSTR
  }
  return 0;
}

struct OpenTypeTagPerLanguage {
  Tag language;
  Tag opentype_tag;
  OpenTypeTagCondition condition;
};
static const OpenTypeTagPerLanguage kOpenTypeTagPerLanguage[] = {
  ENDSTR

  lang_ietf_opentype_map.keys.sort_by_tag_value.each do |lang|
    conditions = [ lang_ietf_opentype_map[lang] ].flatten
    default_tag = conditions.shift
    output_file.puts "  { #{make_tag(lang)}, #{make_tag(default_tag)}, #{aligned_opentype_condition_flag("DEFAULT")} },"
    conditions.each_slice(2) do |value, ot_tag|
      output_file.puts "  { #{make_tag(lang)}, #{make_tag(ot_tag)}, #{aligned_opentype_condition_flag(value)} },"
    end
  end

  output_file.puts <<-ENDSTR
};

struct LanguageUsingScript {
  uint16_t start_index;
  uint16_t count;
};
// The list of languages a script can be used for.
// The indices of kLanguagesUsingScript are USCRIPT_ values.
// For each script, the languages are sorted for binary search.
static const LanguageUsingScript kLanguagesUsingScript[] = {
  ENDSTR
  languages_using = []
  icu_scripts.each do |script|
    start_index = languages_using.length
    languages = languages_per_script[script[:short_name]]
    if languages
      languages_using.concat(languages.to_a.sort_by_tag_value.map {|language| [language, script]})
      end_index = languages_using.length
      values = [start_index, end_index-start_index]
    else
      values = [0, 0]
    end
    output_file.puts "  { %3d, %3d },  // #{script[:short_name]} (#{script[:constant]})" % values
  end
  output_file.puts <<-ENDSTR
};
// The languages using each script.
// The index to that array are in kLanguagesUsingScript above.
static const Tag kLanguagesUsing[] = {
  ENDSTR

  languages_using.each do |language, script|
    output_file.puts "  #{make_tag(language, 3)},  // #{script[:short_name]}"
  end

  output_file.puts <<-ENDSTR
};
  ENDSTR
end
puts "generated #{output_file_path}"

