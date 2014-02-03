Volume "Built-Ins"

Book "Sentences"

[Inform 7 ``sentences'' are not sentences in the grammatical sense, though they
come close.  A ``sentence'' might be better thought of as being either a
statement in the traditional programming language sense, or else a definition,
phrase, or rule preamble.]

Book "Sentence Boundaries"

[Inform divides sentences on five delimiters: paragraph breaks, full stops,
semicolons, colons, and sometimes commas, depending on context.  Here we take
any of the first four, when in unquoted, uncommented, unnested I7, to constitute
a sentence boundary; sentence-dividing commas are handled separately, as
described in the chapter "A Sentence".

Boundaries cannot be used directly in the parser rules, only implicitly by
judiciously choosing appropriate kinds for nonterminals.  See the following
book.]

Book "Built-in Kinds of Nonterminals"

Chapter "A Subsentence"

[``A subsentence'' matches any token sequence that does not cross a sentence
boundary.  The kind is used for the nonterminal ``a within-sentence token
sequence'', which in turn helps define the nonterminal ``words''.  Hopefully it
can be avoided by everything else.]

Chapter "A Wording"

[``A wording'' matches any token sequence that begins in plain I7 (possibly
within an extract) or documentation, ends in the same lexical state, and does
not cross a sentence boundary.]

Chapter "A Sentence"

[``A sentence'' matches any token sequence that begins in plain I7 (possibly
within an extract) or documentation, ends in the same lexical state just before
a sentence boundary, and does not cross a sentence boundary.  Sentences
automatically absorb any sentence-ending punctuation; it is not necessary to
include the possibilities in a sentence's productions.

Ordinarily a sentence must also begin just after a sentence boundary, but this
requirement is waived when matching part of a larger sentence or passage, a
concession made for the sake of sentence-splitting commas.  For instance, given

	# A statement is a sentence.
	# do something => a statement.
	# A one-line rule is a sentence.
	# When play begins, (S - a statement) => a one-line rule.

``a one-line rule'' can match the sentence ``When play begins, do something.''
Allowing overlapping sentences like this (1) saves the highlighter's parser from
having potentially complicated, hard-coded rules for distinguishing ordinary and
sentence-separating commas, (2) cuts out some CYK prefiltering checks, making
the parser internals simpler, and (3) works just as well in practice as matching
the preamble and the body as disjoint sentences.

Note that sentences using this waiver will always share an ending with their
ancestors.  An attempt to write rules expecting otherwise, such as

	# A foo is a sentence.
	# When play begins, => a preamble.
	# A one-line rule is a sentence.
	# (P - a preamble) do something => a one-line rule.

will result in ``a one-line rule'' never matching, because it would have to
cross the sentence boundary after ``P'' to reach the words ``do something''.]

Chapter "A Passage"

[``A passage'' matches any token sequence that begins in plain I7 (possibly
within an extract) or documentation, usually just after a sentence boundary, and
ends in the same lexical state just before a sentence boundary.  It is
ordinarily used to agglomerate several sentences that are part of a fixed
pattern, like the preamble-body form in definitions, phrases, and rules.

Like sentences, passages may begin in places other than after a sentence
boundary if they are part of a larger passage.  This makes the parser a little
simpler to write, but it's not clear that this caveat is actually useful for
anything.]

Book "Built-in Terminals"

Chapter "Parentheses"

[``Left parenthesis'' and ``right parenthesis'' match the tokens ``('' and
``)'', respectively.]

Chapter "Digits"

[``Digits'' is a terminal that matches a single token consisting of only the
digits 0–9.]

Chapter "A Word"

[``A word'' is a terminal that matches a single token.]

Chapter "A Name Word"

[``A name word'' is a terminal that matches a single token that does not contain
a comma, a double quote, or a parenthesis, those characters being forbidden in
I7 names.]

Volume "Automatics"

Book "Automatic Nonterminals for Kinds"

[....]

Volume "Common Wordings"

Book "Words"

# A within-sentence token sequence is a subsentence.
# (W - a word) => a within-sentence token sequence.
# (L - a within-sentence token sequence) (W - a word) => a within-sentence token sequence.

# Words is a wording.
# (W - a within-sentence token sequence) => words.

Book "Names"

# A name is a wording.
# (W - a name word) => a name.
# (N - a name) (W - a name word) => a name.

Book "Numbers"

# zero => a literal number: decide on 0.
# one => a literal number: decide on 1.
# two => a literal number: decide on 2.
# three => a literal number: decide on 3.
# four => a literal number: decide on 4.
# five => a literal number: decide on 5.
# six => a literal number: decide on 6.
# seven => a literal number: decide on 7.
# eight => a literal number: decide on 8.
# nine => a literal number: decide on 9.
# ten => a literal number: decide on 10.
# eleven => a literal number: decide on 11.
# twelve => a literal number: decide on 12.

Book "Lists"

# A conjunctive list delimiter is a wording.
# , => a list delimiter.
# and => a list delimiter.
# , and => a list delimiter.

# A disjunctive list delimiter is a wording.
# , => a list delimiter.
# or => a list delimiter.
# , or => a list delimiter.

# A list delimiter is a wording.
# (D - a conjunctive list delimiter) => a list delimiter.
# (D - a disjunctive list delimiter) => a list delimiter.

Volume "Source Code Organization"

Book "Headings"

# A heading is a sentence.

Chapter "General Heading Annotations"

# A Z-machine version is a wording.
# 5/6/8/five/six/eight => a Z-machine version.

# A list of Z-machine versions is a wording.
# (V - a Z-machine version) => a list of Z-machine versions.
# (L - a list of Z-machine versions) (D - a list delimiter) (V - a Z-machine version) => a list of Z-machine versions.

# A format condition is a wording.
# for Z-machine only => a format condition.
# for Z-machine version/versions (L - a list of Z-machine versions) only => a format condition.
# for Glulx only => a format condition.
# for Z-machine version/versions (L - a list of Z-machine versions) and/or Glulx only => a format condition.
# for Glulx and/or Z-machine version/versions (L - a list of Z-machine versions) only => a format condition.

Section "Internal Heading Annotations"

# A heading annotation is a wording.
# - not for release => a heading annotation.
# - unindexed => a heading annotation.
# (L - a left parenthesis) for use with (X - an extension name) => a heading annotation.
# (L - a left parenthesis) for use without (X - an extension name) => a heading annotation.
# (L - a left parenthesis) in place of (T - a title) in (X - an extension name) (R - a right parenthesis) => a heading annotation.
# (L - a left parenthesis) (C - a format condition) (R - a right parenthesis) => a heading annotation.

# A heading annotation list is a wording.
# (A - a heading annotation) => a heading annotation list.
# (L - a heading annotation list) (A - a heading annotation) => a heading annotation list.

Chapter "Headings for Entire Source Texts"

Section "Titles and Authors"

# A title is a wording.
# (T - some words) => a title.

# An author is a wording.
# (A - some words) => an author.

Section "Story Source Text Headings"

# A story source text heading is a sentence.
# (B - the beginning of a story) (T - a title) => a story source text heading.
# (B - the beginning of a story) (T - a title) by (A - an author) => a story source text heading.
# (H - a story source text heading) => a heading.

Section "Extension Source Text Headings"

# An extension title is a wording.
# (T - a title) => an extension title.
# version (N - a literal number) of (T - a title) => an extension title.
# version (N - a literal number)/(D - a date) of (T - a title) => an extension title.

# An extension beginning line is a sentence.
# (B - the beginning of an extension) (T - an extension title) by (A - an author) begins here => an extension beginning.
# (B - the beginning of an extension) (T - an extension title) (L - a left parenthesis) (A - a format condition) (R - a right parenthesis) by (A - an author) begins here => an extension beginning.

# An extension beginning is a passage.
# (L - an extension beginning line) => an extension beginning.
# (L - an extension beginning line) (R - some literal text) => an extension beginning.
# (L - an extension beginning line) (R - some literal text) (C - some literal text) => an extension beginning.

# An extension end line is a sentence.
# (T - a title) by (A - an author) ends here => an extension end line.

Chapter "Internal Headings"

Section "Levels of Internal Headings"

# A section heading is a sentence.
# Section (T - some words) => a section heading.
# Section (T - some words) (A - a heading annotation list) => a section heading.
# (H - a section heading) => a heading.

# A chapter heading is a sentence.
# Chapter (T - some words) => a chapter heading.
# Chapter (T - some words) (A - a heading annotation list) => a chapter heading.
# (H - a chapter heading) => a heading.

# A part heading is a sentence.
# Part (T - some words) => a part heading.
# Part (T - some words) (A - a heading annotation list) => a part heading.
# (H - a part heading) => a heading.

# A book heading is a sentence.
# Book (T - some words) => a book heading.
# Book (T - some words) (A - a heading annotation list) => a book heading.
# (H - a book heading) => a heading.

# A volume heading is a sentence.
# Volume (T - some words) => a volume heading.
# Volume (T - some words) (A - a heading annotation list) => a volume heading.
# (H - a volume heading) => a heading.

Chapter "Documentation Headings"

Section "Levels of Documentation Headings"

# A documentation section heading is a passage.
# Section: (T - some words) => a documentation section heading.
# (H - a documentation section heading) => a heading.

# A documentation chapter heading is a passage.
# Chapter: (T - some words) => a documentation chapter heading.
# (H - a documentation chapter heading) => a heading.

Section "Example Headings"

# Example stars are a kind of wording.
# */**/***/**** -> example stars.

# A documentation example heading is a passage.
# Example: (S - some example stars) (T - a title) - (D - some words) => a documentation example heading.
# (H - a documentation example heading) => a heading.

Book "Inclusions"

# An extension title list is a wording.
# (T - an extension title) => an extension title list.
# (L - an extension title list) (D - a conjunctive list delimiter) (T - a title) => an extension title list.

# An inclusion is a sentence.
# Include (E - an extension title list) by (A - an author) => an inclusion.

Book "Use Options"

# A use option use is a sentence.
# Use (O - a use option) => a use option use.
# Use (O - a use option) of (N - a literal number) => a use option use.

Book "Test Scripts"

# A test script is a sentence.
# Test (T - a name word) with (S - some literal text) => a test script.

Book "Index Options"

Chapter "Index Settings"

# An index setting is a wording.
# font => an index setting.
# minimum-map-width => an index setting.
# title => an index setting.
# title-size => an index setting.
# title-font => an index setting.
# title-colour/title-color => an index setting.
# map-outline => an index setting.
# border-size => an index setting.
# vertical-spacing => an index setting.
# monochrome => an index setting.
# annotation-size => an index setting.
# annotation-length => an index setting.
# annotation-font => an index setting.
# subtitle => an index setting.
# subtitle-size => an index setting.
# subtitle-font => an index setting.
# subtitle-colour/subtitle-color => an index setting.
# grid-size => an index setting.
# route-stiffness => an index setting.
# route-thickness => an index setting.
# route-colour/route-color => an index setting.
# room-offset => an index setting.
# room-size => an index setting.
# room-colour/room-color => an index setting.
# room-name => an index setting.
# room-name-size => an index setting.
# room-name-font => an index setting.
# room-name-colour/room-name-color => an index setting.
# room-name-length => an index setting.
# room-name-offset => an index setting.
# room-outline => an index setting.
# room-outline-colour/room-outline-color => an index setting.
# room-outline-thickness => an index setting.
# room-shape => an index setting.

Chapter "Index Setting Values"

# An index setting value is a wording.
# on/off => an index setting value.
# (N - a literal number) => an index setting value.
# (X - a literal number) & (Y - a literal number) => an index setting value.
# (T - some literal text) => an index setting value.

Chapter "Index Options Proper"

# An optional index map rubric size is a wording.
# (N - nothing) => an optional index map rubric size.
# size (S - a literal number) => an optional index map rubric size.

# An optional index map rubric font is a wording.
# (N - nothing) => an optional index map rubric font.
# font (F - some literal text) => an optional index map rubric font.

# An optional index map rubric colour/color is a wording.
# (N - nothing) => an optional index map rubric colour/color.
# color/colour (C - some literal text) => an optional index map rubric colour/color.

# An optional index map rubric reference point is a wording.
# (N - nothing) => an optional index map rubric reference point.
# from (R - a literal room) => an optional index map rubric reference point.

# An index option is a wording.
# (R - a literal room) mapped (D - a literal direction) of (S - a literal room) => an index option.
# an/-- EPS file => an index option.
# title set to (T - some literal text) => an index option.
# (S - an index setting) set to (V - an index setting value) => an index option.
# (S - an index setting) of level (L - a literal number) set to (V - an index setting value) => an index option.
# subtitle of level (L - a literal number) set to (S - some literal text) => an index option.
# (S - an index setting) of (R - a literal region) set to (V - an index setting value) => an index option.
# (S - an index setting) of (R - a description by kind of rooms) set to (V - an index setting value) => an index option.
# (S - an index setting) of (R - a literal room) set to (V - an index setting value) => an index option.
# name of (R - a literal room) set to (N - some literal text) => an index option.
# (S - an index setting) of the first room set to (V - an index setting value) => an index option.
# rubric (R - some literal text) (S - an optional index map rubric size) (F - an optional index map rubric font) (C - an optional index map rubric color) at (X - a literal number) & (Y - a literal number) (R - an optional index map rubric reference point) => an index option.

Chapter "Index Commands"

# An index option list is a wording.
# (O - an index option) => an index option list.
# (L - an index option list) (D - a conjunctive list delimiter) (O - an index option) => an index option list.

# An index command is a sentence.
# Index map with (O - an index option list) => an index command.

Book "Release Instructions"

# A release option is a wording.
# a file of (F - some literal text) called (N - some literal text) => a release option.
# cover art => a release option.
# the introductory booklet => a release option.
# a website => a release option.
# a (T - some literal text) website => a release option.
# an interpreter => a release option.
# the (I - some literal text) interpreter => a release option.
# an existing story file => a release option.
# a public/private/-- solution => a release option.
# the public/private/-- source text => a release option.
# the library card => a release option.

# A release option list is a wording.
# (O - a release option) => a release option list.
# (L - a release option list) (O - a release option) => a release option list.

# A release instruction is a sentence.
# Release along with (L - a release option list) => a release instruction.

Book "Telemetry Notes"

# A telemetry note is a sentence.
# * (N - some literal text) => a telemetry note.

Book "Secret Syntaxes"

# A secret syntax is a sentence.
# * => a secret syntax.

[ni__crash__1, ni__crash__10, and ni__crash__11 are "secret hieroglyphs of
dreadful power", and trying to use them as object or value names causes ni to
terminate with the corresponding exit code.  I only note them here, as they are
already accommodated by the standard Inform 7 syntax.]

Volume "Assertions"

# An assertion is a sentence.

Volume "Rules"

# A rule is a sentence.
