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

UNICODE_VERSION = "7.0.0"

def in_src_dir(filename)
  File.expand_path(File.join(File.dirname(__FILE__), "../../src", filename))
end

def in_data_dir(filename)
  File.expand_path(File.join(File.dirname(__FILE__), "../../data", filename))
end

def in_ucd_dir(filename)
  in_data_dir(File.join("UCD-#{UNICODE_VERSION}", filename))
end

def each_file_of_unicode_data_file(file_path)
  File.open(file_path) do |f|
    f.each_line do |original_line|
      line = original_line.gsub(/#.*/, "").strip
      next if line.empty?
      yield line.split(/\s*;\s*/)
    end
  end
end

def load_all_scripts
  all_scripts = {}
  each_file_of_unicode_data_file(in_data_dir("iso15924-utf8-20131012.txt")) do |fields|
    short_name, _, long_name, = fields
    all_scripts[short_name] = long_name
  end
  all_scripts
end

def load_script_for_codepoints
  # make conversion table from script names in Scripts.txt (Latin, Arabic, Hiragana...)
  # to 4 letters ISO 15924 script codes (Latn, Arab, Hira...)
  script_codes = {}
  each_file_of_unicode_data_file(in_ucd_dir("PropertyValueAliases.txt")) do |fields|
    # only interested in the script property value aliases
    next unless fields.first == "sc"
    code, name = fields[1..-1]
    script_codes[name] = code
  end

  # read the script code for Unicode codepoints
  scripts = {}
  each_file_of_unicode_data_file(in_ucd_dir("Scripts.txt")) do |fields|
    codepoints, script = fields
    script_code = script_codes[script]
    unless script_code
      puts "unknown script #{script}"
      next
    end
    if md = /\A([0-9A-F]+)\.\.([0-9A-F]+)\z/i.match(codepoints) # codepoints range
      first, last = md[1].hex, md[2].hex
    elsif md = /\A([0-9A-F]+)\z/i.match(codepoints) # 1 codepoint
      first = last = md[1].hex
    else
      raise "invalid data #{fields.inspect} in Scripts.txt"
    end
    (first..last).each do |codepoint|
      scripts[codepoint] = script_code
    end
  end
  scripts
end
