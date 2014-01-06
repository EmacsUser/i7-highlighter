#include <cstdint>
#include <cstdio>

#include "io.hpp"
#include "protocol.hpp"

static bool endianness_undetermined = true;
static bool flip_endianness = false;
static union {
  struct { uint8_t a, b, c, d; } bytes;
  uint32_t codepoint;
} codepoint_union;

static bool stdin_closed = false;
static inline uint8_t read_byte() {
  int result = fgetc(stdin);
  stdin_closed |= (result == EOF);
  return static_cast<uint8_t>(result);
}

static inline uint32_t read_codepoint() {
  if (flip_endianness) {
    codepoint_union.bytes.a = static_cast<uint8_t>(read_byte());
    codepoint_union.bytes.b = static_cast<uint8_t>(read_byte());
    codepoint_union.bytes.c = static_cast<uint8_t>(read_byte());
    codepoint_union.bytes.d = static_cast<uint8_t>(read_byte());
  } else {
    codepoint_union.bytes.d = static_cast<uint8_t>(read_byte());
    codepoint_union.bytes.c = static_cast<uint8_t>(read_byte());
    codepoint_union.bytes.b = static_cast<uint8_t>(read_byte());
    codepoint_union.bytes.a = static_cast<uint8_t>(read_byte());
  }
  if (stdin_closed) {
    return CLIENT_END_SESSION;
  }
  if (endianness_undetermined) {
    endianness_undetermined = false;
    flip_endianness = (codepoint_union.codepoint > 0xFFFF);
    if (flip_endianness) {
      uint8_t swap = codepoint_union.bytes.a;
      codepoint_union.bytes.a = codepoint_union.bytes.d;
      codepoint_union.bytes.d = swap;
      swap = codepoint_union.bytes.b;
      codepoint_union.bytes.b = codepoint_union.bytes.c;
      codepoint_union.bytes.c = swap;
    }
  }
  return codepoint_union.codepoint;
}

static inline i7_string read_string() {
  i7_string_stream stream;
  for (uint32_t codepoint; (codepoint = read_codepoint());) {
    stream << static_cast<i7_codepoint>(codepoint);
  }
  return stream.str();
}

static inline void write_codepoint(uint32_t codepoint) {
  codepoint_union.codepoint = codepoint;
  if (flip_endianness) {
    fputc(codepoint_union.bytes.a, stdout);
    fputc(codepoint_union.bytes.b, stdout);
    fputc(codepoint_union.bytes.c, stdout);
    fputc(codepoint_union.bytes.d, stdout);
  } else {
    fputc(codepoint_union.bytes.d, stdout);
    fputc(codepoint_union.bytes.c, stdout);
    fputc(codepoint_union.bytes.b, stdout);
    fputc(codepoint_union.bytes.a, stdout);
  }
}

static inline void write_string(const i7_string&string) {
  for (i7_codepoint codepoint : string) {
    write_codepoint(static_cast<uint32_t>(codepoint));
  }
}

void startup_io() {
  freopen(NULL, "rb", stdin);
  freopen(NULL, "wb", stdout);
  uint32_t command;
  unsigned buffer_number;
  unsigned view_number;
  unsigned beginning, end;
  ::highlight_code highlight_code;
  i7_string text;
  for (;;) {
    command = read_codepoint();
    switch (command) {
    case CLIENT_END_SESSION:
      return;
    case CLIENT_BEGIN_SESSION:
      break;
    case CLIENT_SUPPORT_HIGHLIGHT_CODE:
      highlight_code = static_cast< ::highlight_code>(read_codepoint());
      support_highlight_code(highlight_code);
      break;
    case CLIENT_DISCARD_BUFFER:
      buffer_number = static_cast<unsigned>(read_codepoint());
      discard_buffer(buffer_number);
      break;
    case CLIENT_INTRODUCE_BUFFER:
      buffer_number = static_cast<unsigned>(read_codepoint());
      introduce_buffer(buffer_number);
      break;
    case CLIENT_MARK_BUFFER_UNDECIDED:
      buffer_number = static_cast<unsigned>(read_codepoint());
      mark_buffer_undecided(buffer_number);
      break;
    case CLIENT_MARK_BUFFER_AS_STORY:
      buffer_number = static_cast<unsigned>(read_codepoint());
      mark_buffer_as_story(buffer_number);
      break;
    case CLIENT_MARK_BUFFER_AS_EXTENSION:
      buffer_number = static_cast<unsigned>(read_codepoint());
      text = read_string();
      mark_buffer_as_extension(buffer_number, text);
      break;
    case CLIENT_REMOVE_CODEPOINTS:
      buffer_number = static_cast<unsigned>(read_codepoint());
      beginning = static_cast<unsigned>(read_codepoint());
      end = static_cast<unsigned>(read_codepoint());
      remove_codepoints(buffer_number, beginning, end);
      break;
    case CLIENT_ADD_CODEPOINTS:
      buffer_number = static_cast<unsigned>(read_codepoint());
      beginning = static_cast<unsigned>(read_codepoint());
      text = read_string();
      add_codepoints(buffer_number, beginning, text);
      break;
    case CLIENT_DISCARD_VIEW:
      view_number = static_cast<unsigned>(read_codepoint());
      discard_view(view_number);
      break;
    case CLIENT_INTRODUCE_VIEW:
      view_number = static_cast<unsigned>(read_codepoint());
      buffer_number = static_cast<unsigned>(read_codepoint());
      introduce_view(view_number, buffer_number);
      break;
    case CLIENT_CLEAR_VIEW:
      view_number = static_cast<unsigned>(read_codepoint());
      move_view(view_number, 0, 0);
      break;
    case CLIENT_MOVE_VIEW:
      view_number = static_cast<unsigned>(read_codepoint());
      beginning = static_cast<unsigned>(read_codepoint());
      end = static_cast<unsigned>(read_codepoint());
      move_view(view_number, beginning, end);
      break;
    case CLIENT_CLEAR_CURSOR:
      view_number = static_cast<unsigned>(read_codepoint());
      clear_cursor(view_number);
      break;
    case CLIENT_SET_CURSOR:
      view_number = static_cast<unsigned>(read_codepoint());
      beginning = static_cast<unsigned>(read_codepoint());
      end = static_cast<unsigned>(read_codepoint());
      set_cursor(view_number, beginning, end);
      break;
    default:
      printf("Unrecognized command from editor: %08x.", command);
      exit(1);
    }
    fflush(stdout);
  }
}

void remove_highlights(unsigned buffer_number, unsigned beginning, unsigned end) {
  write_codepoint(SERVER_REMOVE_HIGHLIGHTS);
  write_codepoint(buffer_number);
  write_codepoint(beginning);
  write_codepoint(end);
}
void add_highlight(unsigned buffer_number, unsigned beginning, unsigned end, ::highlight_code highlight_code) {
  write_codepoint(SERVER_ADD_HIGHLIGHT);
  write_codepoint(buffer_number);
  write_codepoint(beginning);
  write_codepoint(end);
  write_codepoint(static_cast<i7_codepoint>(highlight_code));
}
void remove_warnings(unsigned buffer_number, unsigned beginning, unsigned end) {
  write_codepoint(SERVER_REMOVE_WARNINGS);
  write_codepoint(buffer_number);
  write_codepoint(beginning);
  write_codepoint(end);
}
void add_warning(unsigned buffer_number, unsigned beginning, unsigned end) {
  write_codepoint(SERVER_ADD_WARNING);
  write_codepoint(buffer_number);
  write_codepoint(beginning);
  write_codepoint(end);
}
void remove_errors(unsigned buffer_number, unsigned beginning, unsigned end) {
  write_codepoint(SERVER_REMOVE_ERRORS);
  write_codepoint(buffer_number);
  write_codepoint(beginning);
  write_codepoint(end);
}
void add_error(unsigned buffer_number, unsigned beginning, unsigned end) {
  write_codepoint(SERVER_ADD_ERROR);
  write_codepoint(buffer_number);
  write_codepoint(beginning);
  write_codepoint(end);
}
void remove_hovertexts(unsigned buffer_number, unsigned beginning, unsigned end) {
  write_codepoint(SERVER_REMOVE_HOVERTEXTS);
  write_codepoint(buffer_number);
  write_codepoint(beginning);
  write_codepoint(end);
}
void add_hovertext(unsigned buffer_number, unsigned beginning, unsigned end, const i7_string&hovertext) {
  write_codepoint(SERVER_ADD_HOVERTEXT);
  write_codepoint(buffer_number);
  write_codepoint(beginning);
  write_codepoint(end);
  write_string(hovertext);
}

void remove_emphasis(unsigned view_number, unsigned beginning, unsigned end) {
  write_codepoint(SERVER_REMOVE_EMPHASIS);
  write_codepoint(view_number);
  write_codepoint(beginning);
  write_codepoint(end);
}
void add_emphasis(unsigned view_number, unsigned beginning, unsigned end) {
  write_codepoint(SERVER_ADD_EMPHASIS);
  write_codepoint(view_number);
  write_codepoint(beginning);
  write_codepoint(end);
}
void clear_suggestions(unsigned view_number) {
  write_codepoint(SERVER_CLEAR_SUGGESTIONS);
  write_codepoint(view_number);
}
void make_suggestions(unsigned view_number, const i7_string&suggestion) {
  write_codepoint(SERVER_MAKE_SUGGESTION);
  write_codepoint(view_number);
  write_string(suggestion);
}
