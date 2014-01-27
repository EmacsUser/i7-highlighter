#include "session.hpp"
#include "io.hpp"

// Placeholders

void support_highlight_code(::highlight_code highlight_code) {}

void mark_buffer_undecided(unsigned buffer_number) {}
void mark_buffer_as_story(unsigned buffer_number) {}
void mark_buffer_as_extension(unsigned buffer_number, const i7_string&includable_file_name) {}

void discard_view(unsigned view_number) {}
void introduce_view(unsigned view_number, unsigned buffer_number) {}

void move_view(unsigned view_number, unsigned beginning, unsigned end) {}

void clear_cursor(unsigned view_number) {}
void set_cursor(unsigned view_number, unsigned beginning, unsigned end) {}

// End placeholders

int main() {
  session = new typename ::session{};
  startup_io();
  return 0;
}
