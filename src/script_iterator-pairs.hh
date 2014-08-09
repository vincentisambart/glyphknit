// this file should only be included by script_iterator.cc
// file automatically generated by scripts/generate_pairs_list.rb, do not edit
static const uint16_t kPairStarts[] = {  // codepoints sorted for binary search
  0x0028,  // LEFT PARENTHESIS
  0x003c,  // LESS-THAN SIGN
  0x005b,  // LEFT SQUARE BRACKET
  0x007b,  // LEFT CURLY BRACKET
  0x00ab,  // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
  0x00bb,  // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
  0x2018,  // LEFT SINGLE QUOTATION MARK
  0x2019,  // RIGHT SINGLE QUOTATION MARK
  0x201a,  // SINGLE LOW-9 QUOTATION MARK
  0x201b,  // SINGLE HIGH-REVERSED-9 QUOTATION MARK
  0x201c,  // LEFT DOUBLE QUOTATION MARK
  0x201d,  // RIGHT DOUBLE QUOTATION MARK
  0x201e,  // DOUBLE LOW-9 QUOTATION MARK
  0x201f,  // DOUBLE HIGH-REVERSED-9 QUOTATION MARK
  0x2039,  // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
  0x203a,  // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
  0x2045,  // LEFT SQUARE BRACKET WITH QUILL
  0x207d,  // SUPERSCRIPT LEFT PARENTHESIS
  0x208d,  // SUBSCRIPT LEFT PARENTHESIS
  0x2308,  // LEFT CEILING
  0x230a,  // LEFT FLOOR
  0x2329,  // LEFT-POINTING ANGLE BRACKET
  0x2768,  // MEDIUM LEFT PARENTHESIS ORNAMENT
  0x276a,  // MEDIUM FLATTENED LEFT PARENTHESIS ORNAMENT
  0x276c,  // MEDIUM LEFT-POINTING ANGLE BRACKET ORNAMENT
  0x276e,  // HEAVY LEFT-POINTING ANGLE QUOTATION MARK ORNAMENT
  0x2770,  // HEAVY LEFT-POINTING ANGLE BRACKET ORNAMENT
  0x2772,  // LIGHT LEFT TORTOISE SHELL BRACKET ORNAMENT
  0x2774,  // MEDIUM LEFT CURLY BRACKET ORNAMENT
  0x27c5,  // LEFT S-SHAPED BAG DELIMITER
  0x27e6,  // MATHEMATICAL LEFT WHITE SQUARE BRACKET
  0x27e8,  // MATHEMATICAL LEFT ANGLE BRACKET
  0x27ea,  // MATHEMATICAL LEFT DOUBLE ANGLE BRACKET
  0x27ec,  // MATHEMATICAL LEFT WHITE TORTOISE SHELL BRACKET
  0x27ee,  // MATHEMATICAL LEFT FLATTENED PARENTHESIS
  0x2983,  // LEFT WHITE CURLY BRACKET
  0x2985,  // LEFT WHITE PARENTHESIS
  0x2987,  // Z NOTATION LEFT IMAGE BRACKET
  0x2989,  // Z NOTATION LEFT BINDING BRACKET
  0x298b,  // LEFT SQUARE BRACKET WITH UNDERBAR
  0x298d,  // LEFT SQUARE BRACKET WITH TICK IN TOP CORNER
  0x298f,  // LEFT SQUARE BRACKET WITH TICK IN BOTTOM CORNER
  0x2991,  // LEFT ANGLE BRACKET WITH DOT
  0x2993,  // LEFT ARC LESS-THAN BRACKET
  0x2995,  // DOUBLE LEFT ARC GREATER-THAN BRACKET
  0x2997,  // LEFT BLACK TORTOISE SHELL BRACKET
  0x29d8,  // LEFT WIGGLY FENCE
  0x29da,  // LEFT DOUBLE WIGGLY FENCE
  0x29fc,  // LEFT-POINTING CURVED ANGLE BRACKET
  0x2e02,  // LEFT SUBSTITUTION BRACKET
  0x2e03,  // RIGHT SUBSTITUTION BRACKET
  0x2e04,  // LEFT DOTTED SUBSTITUTION BRACKET
  0x2e05,  // RIGHT DOTTED SUBSTITUTION BRACKET
  0x2e09,  // LEFT TRANSPOSITION BRACKET
  0x2e0a,  // RIGHT TRANSPOSITION BRACKET
  0x2e0c,  // LEFT RAISED OMISSION BRACKET
  0x2e0d,  // RIGHT RAISED OMISSION BRACKET
  0x2e1c,  // LEFT LOW PARAPHRASE BRACKET
  0x2e1d,  // RIGHT LOW PARAPHRASE BRACKET
  0x2e20,  // LEFT VERTICAL BAR WITH QUILL
  0x2e21,  // RIGHT VERTICAL BAR WITH QUILL
  0x2e22,  // TOP LEFT HALF BRACKET
  0x2e24,  // BOTTOM LEFT HALF BRACKET
  0x2e26,  // LEFT SIDEWAYS U BRACKET
  0x2e28,  // LEFT DOUBLE PARENTHESIS
  0x2e42,  // DOUBLE LOW-REVERSED-9 QUOTATION MARK
  0x3008,  // LEFT ANGLE BRACKET
  0x300a,  // LEFT DOUBLE ANGLE BRACKET
  0x300c,  // LEFT CORNER BRACKET
  0x300e,  // LEFT WHITE CORNER BRACKET
  0x3010,  // LEFT BLACK LENTICULAR BRACKET
  0x3014,  // LEFT TORTOISE SHELL BRACKET
  0x3016,  // LEFT WHITE LENTICULAR BRACKET
  0x3018,  // LEFT WHITE TORTOISE SHELL BRACKET
  0x301a,  // LEFT WHITE SQUARE BRACKET
  0x301d,  // REVERSED DOUBLE PRIME QUOTATION MARK
  0xfd3f,  // ORNATE RIGHT PARENTHESIS
  0xfe17,  // PRESENTATION FORM FOR VERTICAL LEFT WHITE LENTICULAR BRACKET
  0xfe35,  // PRESENTATION FORM FOR VERTICAL LEFT PARENTHESIS
  0xfe37,  // PRESENTATION FORM FOR VERTICAL LEFT CURLY BRACKET
  0xfe39,  // PRESENTATION FORM FOR VERTICAL LEFT TORTOISE SHELL BRACKET
  0xfe3b,  // PRESENTATION FORM FOR VERTICAL LEFT BLACK LENTICULAR BRACKET
  0xfe3d,  // PRESENTATION FORM FOR VERTICAL LEFT DOUBLE ANGLE BRACKET
  0xfe3f,  // PRESENTATION FORM FOR VERTICAL LEFT ANGLE BRACKET
  0xfe41,  // PRESENTATION FORM FOR VERTICAL LEFT CORNER BRACKET
  0xfe43,  // PRESENTATION FORM FOR VERTICAL LEFT WHITE CORNER BRACKET
  0xfe47,  // PRESENTATION FORM FOR VERTICAL LEFT SQUARE BRACKET
  0xfe59,  // SMALL LEFT PARENTHESIS
  0xfe5b,  // SMALL LEFT CURLY BRACKET
  0xfe5d,  // SMALL LEFT TORTOISE SHELL BRACKET
  0xff08,  // FULLWIDTH LEFT PARENTHESIS
  0xff3b,  // FULLWIDTH LEFT SQUARE BRACKET
  0xff5b,  // FULLWIDTH LEFT CURLY BRACKET
  0xff5f,  // FULLWIDTH LEFT WHITE PARENTHESIS
  0xff62,  // HALFWIDTH LEFT CORNER BRACKET
};
struct PairStartsForEnd {
  uint16_t end_codepoint;
  uint16_t start_codepoint;
};
// Codepoints are sorted for binary search.
static PairStartsForEnd kPairEnds[] = {
  { 0x0029, 0x0028 },  // RIGHT PARENTHESIS - LEFT PARENTHESIS
  { 0x003e, 0x003c },  // GREATER-THAN SIGN - LESS-THAN SIGN
  { 0x005d, 0x005b },  // RIGHT SQUARE BRACKET - LEFT SQUARE BRACKET
  { 0x007d, 0x007b },  // RIGHT CURLY BRACKET - LEFT CURLY BRACKET
  { 0x00ab, 0x00bb },  // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK - RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
  { 0x00bb, 0x00ab },  // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK - LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
  { 0x2018, 0x2019 },  // LEFT SINGLE QUOTATION MARK - RIGHT SINGLE QUOTATION MARK
  { 0x2018, 0x201a },  // LEFT SINGLE QUOTATION MARK - SINGLE LOW-9 QUOTATION MARK
  { 0x2019, 0x2018 },  // RIGHT SINGLE QUOTATION MARK - LEFT SINGLE QUOTATION MARK
  { 0x2019, 0x201a },  // RIGHT SINGLE QUOTATION MARK - SINGLE LOW-9 QUOTATION MARK
  { 0x2019, 0x201b },  // RIGHT SINGLE QUOTATION MARK - SINGLE HIGH-REVERSED-9 QUOTATION MARK
  { 0x201b, 0x2019 },  // SINGLE HIGH-REVERSED-9 QUOTATION MARK - RIGHT SINGLE QUOTATION MARK
  { 0x201b, 0x201a },  // SINGLE HIGH-REVERSED-9 QUOTATION MARK - SINGLE LOW-9 QUOTATION MARK
  { 0x201c, 0x201d },  // LEFT DOUBLE QUOTATION MARK - RIGHT DOUBLE QUOTATION MARK
  { 0x201c, 0x201e },  // LEFT DOUBLE QUOTATION MARK - DOUBLE LOW-9 QUOTATION MARK
  { 0x201c, 0x2e42 },  // LEFT DOUBLE QUOTATION MARK - DOUBLE LOW-REVERSED-9 QUOTATION MARK
  { 0x201d, 0x201c },  // RIGHT DOUBLE QUOTATION MARK - LEFT DOUBLE QUOTATION MARK
  { 0x201d, 0x201e },  // RIGHT DOUBLE QUOTATION MARK - DOUBLE LOW-9 QUOTATION MARK
  { 0x201d, 0x201f },  // RIGHT DOUBLE QUOTATION MARK - DOUBLE HIGH-REVERSED-9 QUOTATION MARK
  { 0x201d, 0x2e42 },  // RIGHT DOUBLE QUOTATION MARK - DOUBLE LOW-REVERSED-9 QUOTATION MARK
  { 0x201f, 0x201d },  // DOUBLE HIGH-REVERSED-9 QUOTATION MARK - RIGHT DOUBLE QUOTATION MARK
  { 0x201f, 0x201e },  // DOUBLE HIGH-REVERSED-9 QUOTATION MARK - DOUBLE LOW-9 QUOTATION MARK
  { 0x201f, 0x2e42 },  // DOUBLE HIGH-REVERSED-9 QUOTATION MARK - DOUBLE LOW-REVERSED-9 QUOTATION MARK
  { 0x2039, 0x203a },  // SINGLE LEFT-POINTING ANGLE QUOTATION MARK - SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
  { 0x203a, 0x2039 },  // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK - SINGLE LEFT-POINTING ANGLE QUOTATION MARK
  { 0x2046, 0x2045 },  // RIGHT SQUARE BRACKET WITH QUILL - LEFT SQUARE BRACKET WITH QUILL
  { 0x207e, 0x207d },  // SUPERSCRIPT RIGHT PARENTHESIS - SUPERSCRIPT LEFT PARENTHESIS
  { 0x208e, 0x208d },  // SUBSCRIPT RIGHT PARENTHESIS - SUBSCRIPT LEFT PARENTHESIS
  { 0x2309, 0x2308 },  // RIGHT CEILING - LEFT CEILING
  { 0x230b, 0x230a },  // RIGHT FLOOR - LEFT FLOOR
  { 0x232a, 0x2329 },  // RIGHT-POINTING ANGLE BRACKET - LEFT-POINTING ANGLE BRACKET
  { 0x2769, 0x2768 },  // MEDIUM RIGHT PARENTHESIS ORNAMENT - MEDIUM LEFT PARENTHESIS ORNAMENT
  { 0x276b, 0x276a },  // MEDIUM FLATTENED RIGHT PARENTHESIS ORNAMENT - MEDIUM FLATTENED LEFT PARENTHESIS ORNAMENT
  { 0x276d, 0x276c },  // MEDIUM RIGHT-POINTING ANGLE BRACKET ORNAMENT - MEDIUM LEFT-POINTING ANGLE BRACKET ORNAMENT
  { 0x276f, 0x276e },  // HEAVY RIGHT-POINTING ANGLE QUOTATION MARK ORNAMENT - HEAVY LEFT-POINTING ANGLE QUOTATION MARK ORNAMENT
  { 0x2771, 0x2770 },  // HEAVY RIGHT-POINTING ANGLE BRACKET ORNAMENT - HEAVY LEFT-POINTING ANGLE BRACKET ORNAMENT
  { 0x2773, 0x2772 },  // LIGHT RIGHT TORTOISE SHELL BRACKET ORNAMENT - LIGHT LEFT TORTOISE SHELL BRACKET ORNAMENT
  { 0x2775, 0x2774 },  // MEDIUM RIGHT CURLY BRACKET ORNAMENT - MEDIUM LEFT CURLY BRACKET ORNAMENT
  { 0x27c6, 0x27c5 },  // RIGHT S-SHAPED BAG DELIMITER - LEFT S-SHAPED BAG DELIMITER
  { 0x27e7, 0x27e6 },  // MATHEMATICAL RIGHT WHITE SQUARE BRACKET - MATHEMATICAL LEFT WHITE SQUARE BRACKET
  { 0x27e9, 0x27e8 },  // MATHEMATICAL RIGHT ANGLE BRACKET - MATHEMATICAL LEFT ANGLE BRACKET
  { 0x27eb, 0x27ea },  // MATHEMATICAL RIGHT DOUBLE ANGLE BRACKET - MATHEMATICAL LEFT DOUBLE ANGLE BRACKET
  { 0x27ed, 0x27ec },  // MATHEMATICAL RIGHT WHITE TORTOISE SHELL BRACKET - MATHEMATICAL LEFT WHITE TORTOISE SHELL BRACKET
  { 0x27ef, 0x27ee },  // MATHEMATICAL RIGHT FLATTENED PARENTHESIS - MATHEMATICAL LEFT FLATTENED PARENTHESIS
  { 0x2984, 0x2983 },  // RIGHT WHITE CURLY BRACKET - LEFT WHITE CURLY BRACKET
  { 0x2986, 0x2985 },  // RIGHT WHITE PARENTHESIS - LEFT WHITE PARENTHESIS
  { 0x2988, 0x2987 },  // Z NOTATION RIGHT IMAGE BRACKET - Z NOTATION LEFT IMAGE BRACKET
  { 0x298a, 0x2989 },  // Z NOTATION RIGHT BINDING BRACKET - Z NOTATION LEFT BINDING BRACKET
  { 0x298c, 0x298b },  // RIGHT SQUARE BRACKET WITH UNDERBAR - LEFT SQUARE BRACKET WITH UNDERBAR
  { 0x298e, 0x298d },  // RIGHT SQUARE BRACKET WITH TICK IN BOTTOM CORNER - LEFT SQUARE BRACKET WITH TICK IN TOP CORNER
  { 0x298e, 0x298f },  // RIGHT SQUARE BRACKET WITH TICK IN BOTTOM CORNER - LEFT SQUARE BRACKET WITH TICK IN BOTTOM CORNER
  { 0x2990, 0x298d },  // RIGHT SQUARE BRACKET WITH TICK IN TOP CORNER - LEFT SQUARE BRACKET WITH TICK IN TOP CORNER
  { 0x2990, 0x298f },  // RIGHT SQUARE BRACKET WITH TICK IN TOP CORNER - LEFT SQUARE BRACKET WITH TICK IN BOTTOM CORNER
  { 0x2992, 0x2991 },  // RIGHT ANGLE BRACKET WITH DOT - LEFT ANGLE BRACKET WITH DOT
  { 0x2994, 0x2993 },  // RIGHT ARC GREATER-THAN BRACKET - LEFT ARC LESS-THAN BRACKET
  { 0x2996, 0x2995 },  // DOUBLE RIGHT ARC LESS-THAN BRACKET - DOUBLE LEFT ARC GREATER-THAN BRACKET
  { 0x2998, 0x2997 },  // RIGHT BLACK TORTOISE SHELL BRACKET - LEFT BLACK TORTOISE SHELL BRACKET
  { 0x29d9, 0x29d8 },  // RIGHT WIGGLY FENCE - LEFT WIGGLY FENCE
  { 0x29db, 0x29da },  // RIGHT DOUBLE WIGGLY FENCE - LEFT DOUBLE WIGGLY FENCE
  { 0x29fd, 0x29fc },  // RIGHT-POINTING CURVED ANGLE BRACKET - LEFT-POINTING CURVED ANGLE BRACKET
  { 0x2e02, 0x2e03 },  // LEFT SUBSTITUTION BRACKET - RIGHT SUBSTITUTION BRACKET
  { 0x2e03, 0x2e02 },  // RIGHT SUBSTITUTION BRACKET - LEFT SUBSTITUTION BRACKET
  { 0x2e04, 0x2e05 },  // LEFT DOTTED SUBSTITUTION BRACKET - RIGHT DOTTED SUBSTITUTION BRACKET
  { 0x2e05, 0x2e04 },  // RIGHT DOTTED SUBSTITUTION BRACKET - LEFT DOTTED SUBSTITUTION BRACKET
  { 0x2e09, 0x2e0a },  // LEFT TRANSPOSITION BRACKET - RIGHT TRANSPOSITION BRACKET
  { 0x2e0a, 0x2e09 },  // RIGHT TRANSPOSITION BRACKET - LEFT TRANSPOSITION BRACKET
  { 0x2e0c, 0x2e0d },  // LEFT RAISED OMISSION BRACKET - RIGHT RAISED OMISSION BRACKET
  { 0x2e0d, 0x2e0c },  // RIGHT RAISED OMISSION BRACKET - LEFT RAISED OMISSION BRACKET
  { 0x2e1c, 0x2e1d },  // LEFT LOW PARAPHRASE BRACKET - RIGHT LOW PARAPHRASE BRACKET
  { 0x2e1d, 0x2e1c },  // RIGHT LOW PARAPHRASE BRACKET - LEFT LOW PARAPHRASE BRACKET
  { 0x2e20, 0x2e21 },  // LEFT VERTICAL BAR WITH QUILL - RIGHT VERTICAL BAR WITH QUILL
  { 0x2e21, 0x2e20 },  // RIGHT VERTICAL BAR WITH QUILL - LEFT VERTICAL BAR WITH QUILL
  { 0x2e23, 0x2e22 },  // TOP RIGHT HALF BRACKET - TOP LEFT HALF BRACKET
  { 0x2e23, 0x2e24 },  // TOP RIGHT HALF BRACKET - BOTTOM LEFT HALF BRACKET
  { 0x2e25, 0x2e22 },  // BOTTOM RIGHT HALF BRACKET - TOP LEFT HALF BRACKET
  { 0x2e25, 0x2e24 },  // BOTTOM RIGHT HALF BRACKET - BOTTOM LEFT HALF BRACKET
  { 0x2e27, 0x2e26 },  // RIGHT SIDEWAYS U BRACKET - LEFT SIDEWAYS U BRACKET
  { 0x2e29, 0x2e28 },  // RIGHT DOUBLE PARENTHESIS - LEFT DOUBLE PARENTHESIS
  { 0x3009, 0x3008 },  // RIGHT ANGLE BRACKET - LEFT ANGLE BRACKET
  { 0x300b, 0x300a },  // RIGHT DOUBLE ANGLE BRACKET - LEFT DOUBLE ANGLE BRACKET
  { 0x300d, 0x300c },  // RIGHT CORNER BRACKET - LEFT CORNER BRACKET
  { 0x300f, 0x300e },  // RIGHT WHITE CORNER BRACKET - LEFT WHITE CORNER BRACKET
  { 0x3011, 0x3010 },  // RIGHT BLACK LENTICULAR BRACKET - LEFT BLACK LENTICULAR BRACKET
  { 0x3015, 0x3014 },  // RIGHT TORTOISE SHELL BRACKET - LEFT TORTOISE SHELL BRACKET
  { 0x3017, 0x3016 },  // RIGHT WHITE LENTICULAR BRACKET - LEFT WHITE LENTICULAR BRACKET
  { 0x3019, 0x3018 },  // RIGHT WHITE TORTOISE SHELL BRACKET - LEFT WHITE TORTOISE SHELL BRACKET
  { 0x301b, 0x301a },  // RIGHT WHITE SQUARE BRACKET - LEFT WHITE SQUARE BRACKET
  { 0x301e, 0x301d },  // DOUBLE PRIME QUOTATION MARK - REVERSED DOUBLE PRIME QUOTATION MARK
  { 0x301f, 0x301d },  // LOW DOUBLE PRIME QUOTATION MARK - REVERSED DOUBLE PRIME QUOTATION MARK
  { 0xfd3e, 0xfd3f },  // ORNATE LEFT PARENTHESIS - ORNATE RIGHT PARENTHESIS
  { 0xfe18, 0xfe17 },  // PRESENTATION FORM FOR VERTICAL RIGHT WHITE LENTICULAR BRAKCET - PRESENTATION FORM FOR VERTICAL LEFT WHITE LENTICULAR BRACKET
  { 0xfe36, 0xfe35 },  // PRESENTATION FORM FOR VERTICAL RIGHT PARENTHESIS - PRESENTATION FORM FOR VERTICAL LEFT PARENTHESIS
  { 0xfe38, 0xfe37 },  // PRESENTATION FORM FOR VERTICAL RIGHT CURLY BRACKET - PRESENTATION FORM FOR VERTICAL LEFT CURLY BRACKET
  { 0xfe3a, 0xfe39 },  // PRESENTATION FORM FOR VERTICAL RIGHT TORTOISE SHELL BRACKET - PRESENTATION FORM FOR VERTICAL LEFT TORTOISE SHELL BRACKET
  { 0xfe3c, 0xfe3b },  // PRESENTATION FORM FOR VERTICAL RIGHT BLACK LENTICULAR BRACKET - PRESENTATION FORM FOR VERTICAL LEFT BLACK LENTICULAR BRACKET
  { 0xfe3e, 0xfe3d },  // PRESENTATION FORM FOR VERTICAL RIGHT DOUBLE ANGLE BRACKET - PRESENTATION FORM FOR VERTICAL LEFT DOUBLE ANGLE BRACKET
  { 0xfe40, 0xfe3f },  // PRESENTATION FORM FOR VERTICAL RIGHT ANGLE BRACKET - PRESENTATION FORM FOR VERTICAL LEFT ANGLE BRACKET
  { 0xfe42, 0xfe41 },  // PRESENTATION FORM FOR VERTICAL RIGHT CORNER BRACKET - PRESENTATION FORM FOR VERTICAL LEFT CORNER BRACKET
  { 0xfe44, 0xfe43 },  // PRESENTATION FORM FOR VERTICAL RIGHT WHITE CORNER BRACKET - PRESENTATION FORM FOR VERTICAL LEFT WHITE CORNER BRACKET
  { 0xfe48, 0xfe47 },  // PRESENTATION FORM FOR VERTICAL RIGHT SQUARE BRACKET - PRESENTATION FORM FOR VERTICAL LEFT SQUARE BRACKET
  { 0xfe5a, 0xfe59 },  // SMALL RIGHT PARENTHESIS - SMALL LEFT PARENTHESIS
  { 0xfe5c, 0xfe5b },  // SMALL RIGHT CURLY BRACKET - SMALL LEFT CURLY BRACKET
  { 0xfe5e, 0xfe5d },  // SMALL RIGHT TORTOISE SHELL BRACKET - SMALL LEFT TORTOISE SHELL BRACKET
  { 0xff09, 0xff08 },  // FULLWIDTH RIGHT PARENTHESIS - FULLWIDTH LEFT PARENTHESIS
  { 0xff3d, 0xff3b },  // FULLWIDTH RIGHT SQUARE BRACKET - FULLWIDTH LEFT SQUARE BRACKET
  { 0xff5d, 0xff5b },  // FULLWIDTH RIGHT CURLY BRACKET - FULLWIDTH LEFT CURLY BRACKET
  { 0xff60, 0xff5f },  // FULLWIDTH RIGHT WHITE PARENTHESIS - FULLWIDTH LEFT WHITE PARENTHESIS
  { 0xff63, 0xff62 },  // HALFWIDTH RIGHT CORNER BRACKET - HALFWIDTH LEFT CORNER BRACKET
};
