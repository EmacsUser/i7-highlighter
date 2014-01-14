Volume "Common Wordings"

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

# A conjunctive list delimiter is a kind of wording.
# , => a list delimiter.
# and => a list delimiter.
# , and => a list delimiter.

# A disjunctive list delimiter is a kind of wording.
# , => a list delimiter.
# or => a list delimiter.
# , or => a list delimiter.

# A list delimiter is a kind of wording.
# (D - a conjunctive list delimiter) => a list delimiter.
# (D - a disjunctive list delimiter) => a list delimiter.

Volume "Source Code Organization"

Book "Headings"

# A heading is a kind of sentence.

Chapter "General Heading Annotations"

# A Z-machine version is a kind of wording.
# 5/6/8/five/six/eight => a Z-machine version.

# A list of Z-machine versions is a kind of wording.
# (V - a Z-machine version) => a list of Z-machine versions.
# (L - a list of Z-machine versions) (D - a list delimiter) (V - a Z-machine version) => a list of Z-machine versions.

# A format condition is a kind of wording.
# for Z-machine only => a format condition.
# for Z-machine version/versions (L - a list of Z-machine versions) only => a format condition.
# for Glulx only => a format condition.
# for Z-machine version/versions (L - a list of Z-machine versions) and/or Glulx only => a format condition.
# for Glulx and/or Z-machine version/versions (L - a list of Z-machine versions) only => a format condition.

Section "Internal Heading Annotations"

# A heading annotation is a kind of wording.
# - not for release => a heading annotation.
# - unindexed => a heading annotation.
# (L - a left parenthesis) for use with (X - an extension name) => a heading annotation.
# (L - a left parenthesis) for use without (X - an extension name) => a heading annotation.
# (L - a left parenthesis) in place of (T - a title) in (X - an extension name) (R - a right parenthesis) => a heading annotation.
# (L - a left parenthesis) (C - a format condition) (R - a right parenthesis) => a heading annotation.

# A heading annotation list is a kind of wording.
# (A - a heading annotation) => a heading annotation list.
# (L - a heading annotation list) (A - a heading annotation) => a heading annotation list.

Chapter "Headings for Entire Source Texts"

Section "Titles and Authors"

# A title is a kind of wording.
# (T - some label words) => a title.

# An author is a kind of wording.
# (A - some label words) => an author.

Section "Story Source Text Headings"

# A story source text heading is a kind of sentence.
# (B - the beginning of a story) (T - a title) => a story source text heading.
# (B - the beginning of a story) (T - a title) by (A - an author) => a story source text heading.
# (H - a story source text heading) => a heading.

Section "Extension Source Text Headings"

# An extension title is a kind of wording.
# (T - a title) => an extension title.
# version (N - a literal number) of (T - a title) => an extension title.
# version (N - a literal number)/(D - a date) of (T - a title) => an extension title.

# An extension beginning line is a kind of sentence.
# (B - the beginning of an extension) (T - an extension title) by (A - an author) begins here => an extension beginning.
# (B - the beginning of an extension) (T - an extension title) (L - a left parenthesis) (A - a format condition) (R - a right parenthesis) by (A - an author) begins here => an extension beginning.

# An extension beginning is a kind of passage.
# (L - an extension beginning line) => an extension beginning.
# (L - an extension beginning line) (R - some literal text) => an extension beginning.
# (L - an extension beginning line) (R - some literal text) (C - some literal text) => an extension beginning.

# An extension end line is a kind of sentence.
# (T - a title) by (A - an author) ends here => an extension end line.

Chapter "Internal Headings"

Section "Levels of Internal Headings"

# A section heading is a kind of sentence.
# Section (T - some label words) => a section heading.
# Section (T - some label words) (A - a heading annotation list) => a section heading.
# (H - a section heading) => a heading.

# A chapter heading is a kind of sentence.
# Chapter (T - some label words) => a chapter heading.
# Chapter (T - some label words) (A - a heading annotation list) => a chapter heading.
# (H - a chapter heading) => a heading.

# A part heading is a kind of sentence.
# Part (T - some label words) => a part heading.
# Part (T - some label words) (A - a heading annotation list) => a part heading.
# (H - a part heading) => a heading.

# A book heading is a kind of sentence.
# Book (T - some label words) => a book heading.
# Book (T - some label words) (A - a heading annotation list) => a book heading.
# (H - a book heading) => a heading.

# A volume heading is a kind of sentence.
# Volume (T - some label words) => a volume heading.
# Volume (T - some label words) (A - a heading annotation list) => a volume heading.
# (H - a volume heading) => a heading.

Chapter "Documentation Headings"

Section "Levels of Documentation Headings"

# A documentation section heading is a kind of sentence.
# Section: (T - some label words) => a documentation section heading.
# (H - a documentation section heading) => a heading.

# A documentation chapter heading is a kind of sentence.
# Chapter: (T - some label words) => a documentation chapter heading.
# (H - a documentation chapter heading) => a heading.

Section "Example Headings"

# Example stars are a kind of wording.
# */**/***/**** -> example stars.

# A documentation example heading is a kind of sentence.
# Example: (S - some example stars) (T - a title) - (D - some label words) => a documentation example heading.
# (H - a documentation example heading) => a heading.

Book "Inclusions"

# An extension title list is a kind of wording.
# (T - an extension title) => an extension title list.
# (L - an extension title list) (D - a conjunctive list delimiter) (T - a title) => an extension title list.

# An inclusion is a kind of sentence.
# Include (E - an extension title list) by (A - an author) => an inclusion.

Book "Use Options"

# A use option use is a kind of sentence.
# Use (O - a use option) => a use option use.
# Use (O - a use option) of (N - a literal number) => a use option use.

Book "Test Scripts"

# A test script is a kind of sentence.
# Test (T - some name words) with (S - some literal text) => a test script.

Book "Index Options"

Chapter "Index Settings"

# An index setting is a kind of wording.
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

# An index setting value is a kind of wording.
# on/off => an index setting value.
# (N - a literal number) => an index setting value.
# (X - a literal number) & (Y - a literal number) => an index setting value.
# (T - some literal text) => an index setting value.

Chapter "Index Options Proper"

# An optional index map rubric size is a kind of wording.
# (N - nothing) => an optional index map rubric size.
# size (S - a literal number) => an optional index map rubric size.

# An optional index map rubric font is a kind of wording.
# (N - nothing) => an optional index map rubric font.
# font (F - some literal text) => an optional index map rubric font.

# An optional index map rubric colour/color is a kind of wording.
# (N - nothing) => an optional index map rubric colour/color.
# color/colour (C - some literal text) => an optional index map rubric colour/color.

# An optional index map rubric reference point is a kind of wording.
# (N - nothing) => an optional index map rubric reference point.
# from (R - a literal room) => an optional index map rubric reference point.

# An index option is a kind of wording.
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

# An index option list is a kind of wording.
# (O - an index option) => an index option list.
# (L - an index option list) (D - a conjunctive list delimiter) (O - an index option) => an index option list.

# An index command is a kind of sentence.
# Index map with (O - an index option list) => an index command.

Book "Release Instructions"

# A release option is a kind of wording.
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

# A release option list is a kind of wording.
# (O - a release option) => a release option list.
# (L - a release option list) (O - a release option) => a release option list.

# A release instruction is a kind of sentence.
# Release along with (L - a release option list) => a release instruction.

Book "Telemetry Notes"

# A telemetry note is a kind of sentence.
# * (N - some literal text) => a telemetry note.

Volume "Assertions"

# An assertion is a kind of sentence.

Volume "Rules"

# A rule is a kind of sentence.
