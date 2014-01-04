#ifndef IO_HEADER
#define IO_HEADER

#include "codepoints.hpp"
#include "lexical_highlights.hpp"

// See also protocol.hpp.

void startup_io();

// To be implemented elsewhere:

void support_highlight_code(::highlight_code highlight_code);

void discard_buffer(unsigned buffer_number);
void introduce_buffer(unsigned buffer_number);

void mark_buffer_undecided(unsigned buffer_number);
void mark_buffer_as_story(unsigned buffer_number);
void mark_buffer_as_extension(unsigned buffer_number, const i7_string&includable_file_name);

void remove_codepoints(unsigned buffer_number, unsigned beginning, unsigned end);
void add_codepoints(unsigned buffer_number, unsigned beginning, const i7_string&insertion);

void discard_view(unsigned view_number);
void introduce_view(unsigned view_number, unsigned buffer_number);

void move_view(unsigned view_number, unsigned beginning, unsigned end);

void clear_cursor(unsigned view_number);
void set_cursor(unsigned view_number, unsigned beginning, unsigned end);

// Implemented in io.cpp:

void remove_highlights(unsigned buffer_number, unsigned beginning, unsigned end);
void add_highlight(unsigned buffer_number, unsigned beginning, unsigned end, ::highlight_code highlight_code);
void remove_warnings(unsigned buffer_number, unsigned beginning, unsigned end);
void add_warning(unsigned buffer_number, unsigned beginning, unsigned end);
void remove_errors(unsigned buffer_number, unsigned beginning, unsigned end);
void add_error(unsigned buffer_number, unsigned beginning, unsigned end);
void remove_hovertexts(unsigned buffer_number, unsigned beginning, unsigned end);
void add_hovertext(unsigned buffer_number, unsigned beginning, unsigned end, const i7_string&hovertext);

void remove_emphasis(unsigned view_number, unsigned beginning, unsigned end);
void add_emphasis(unsigned view_number, unsigned beginning, unsigned end);
void clear_suggestions(unsigned view_number);
void make_suggestions(unsigned view_number, const i7_string&suggestion);

#endif
