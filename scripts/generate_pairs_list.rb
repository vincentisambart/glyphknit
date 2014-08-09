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
require_relative "lib/helper"

CATEGORY_OVERRIDES = {
  "<" => "Ps",
  ">" => "Pe",
}

cleaned_category_overrides = {}
CATEGORY_OVERRIDES.each do |k, v|
  k = k.codepoints[0] if k.respond_to?(:to_str)
  cleaned_category_overrides[k] = v
end

scripts = load_script_for_codepoints

pairs = {}
categories = {}
names = {}
File.open(in_ucd_dir("UnicodeData.txt")) do |f|
  f.each_line do |line|
    line = line.strip
    next if line.empty?
    codepoint, name, general_category, = line.split(";")
    codepoint = codepoint.hex
    general_category = cleaned_category_overrides[codepoint] if cleaned_category_overrides[codepoint]
    next if scripts[codepoint] and !%w{Zinh Zyyy Zzzz}.include?(scripts[codepoint])
    if %w{Ps Pe Pi Pf}.include?(general_category)
      simplified_name = name.sub("BRAKCET", "BRACKET").gsub(/\s*\b(TOP|BOTTOM|LEFT|RIGHT|LESS|GREATER|LOW-REVERSED-9|HIGH-REVERSED-9|LOW-9|LOW|REVERSED)\b[\-\s]*/, " ").strip
      pairs[simplified_name] ||= []
      pairs[simplified_name] << codepoint
      categories[codepoint] = general_category
      names[codepoint] = name
    end
  end
end

$categories = categories

def cp_for_display(cp)
  'U+%04x "%s"(%s)' % [cp, cp.chr(Encoding::UTF_8), $categories[cp]]
end

start_codepoints = []
end_codepoints = []
start_or_end_codepoints = []
pairs.each do |simplified_name, codepoints|
  codepoints.each do |cp|
    category = categories[cp]
    if category == "Ps"
      start_codepoints << cp
    elsif category == "Pe"
      end_codepoints << cp
    elsif %w{Pi Pf}.include?(category)
      start_or_end_codepoints << cp
    end
  end
end

# currently we don't need possible_ends afterwards but we have to make sure any codepoint has at least one possible end
possible_ends = {}
pairs.each do |simplified_name, codepoints|
  codepoints.each do |start_cp|
    start_category = categories[start_cp]

    if start_category == "Ps"
      end_categories = %w{Pe Pi Pf}
    elsif start_category == "Pi"
      end_categories = %w{Pe Pf}
    elsif start_category == "Pf"
      end_categories = %w{Pe Pi}
    else
      next
    end
    possible_ends[start_cp] = codepoints.select {|end_cp| end_categories.include?(categories[end_cp]) }
    raise "can't find pair for #{cp_for_display(start_cp)}" if possible_ends[start_cp].empty?
  end
end

possible_starts = {}
pairs.each do |simplified_name, codepoints|
  codepoints.each do |end_cp|
    end_category = categories[end_cp]

    if end_category == "Pe"
      start_categories = %w{Ps Pi Pf}
    elsif end_category == "Pi"
      start_categories = %w{Ps Pf}
    elsif end_category == "Pf"
      start_categories = %w{Ps Pi}
    else
      next
    end
    possible_starts[end_cp] = codepoints.select {|start_cp| start_categories.include?(categories[start_cp]) }
    raise "can't find pair for #{cp_for_display(end_cp)} (#{simplified_name})" if possible_starts[end_cp].empty?
  end
end

max_codepoint = (start_codepoints + end_codepoints + start_or_end_codepoints).max
if max_codepoint.to_s(2).length <= 16
  codepoint_storage_type = "uint16_t"
else
  codepoint_storage_type = "uint32_t"
end

output_file_path = in_src_dir("script_iterator-pairs.hh")
File.open(output_file_path, "w") do |output_file|
  output_file.puts "// this file should only be included by script_iterator.cc"
  output_file.puts "// file automatically generated by scripts/#{File.basename(__FILE__)}, do not edit"
  output_file.puts "static const #{codepoint_storage_type} kPairStarts[] = {  // codepoints sorted for binary search"
  all_start_codepoints = start_codepoints + start_or_end_codepoints
  all_start_codepoints.sort.each_with_index do |cp, i|
    last = (i == all_start_codepoints.length - 1)
    output_file.puts "  0x%04x,  // %s" % [cp, names[cp]]
  end
  output_file.puts "};"

  output_file.puts <<-ENDSTR
struct PairStartsForEnd {
  #{codepoint_storage_type} end_codepoint;
  #{codepoint_storage_type} start_codepoint;
};
  ENDSTR

  def index_of_sub_array(ary, sub_ary)
    (ary.length-sub_ary.length+1).times do |i|
      return i if sub_ary == ary[i..(i+sub_ary.length-1)]
    end
    nil
  end

  output_file.puts "// Codepoints are sorted for binary search."
  output_file.puts "static PairStartsForEnd kPairEnds[] = {"
  all_end_codepoints = end_codepoints + start_or_end_codepoints
  all_end_codepoints.sort.each_with_index do |cp, i|
    last = (i == all_end_codepoints.length - 1)
    possible_starts[cp].sort.each do |start|
      output_file.puts "  { 0x%04x, 0x%04x },  // %s - %s" % [cp, start, names[cp], names[start]]
    end
  end
  output_file.puts "};"
end
puts "generated #{output_file_path}"
