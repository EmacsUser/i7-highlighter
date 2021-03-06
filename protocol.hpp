#ifndef PROTOCOL_HEADER
#define PROTOCOL_HEADER

/* The client (the source text editor) and the server (this program) exchange
 * information about the ongoing edit process through sequences of messages over
 * a codepoint stream.  Each message begins with a codepoint indicating the
 * message's purpose; these purpose codepoints are defined by the macros in this
 * file.  The format for the remaining codepoints in a message depends on the
 * purpose, and is not encoded or enforced here, just documented.
 *
 * In the following,
 *
 *  [lowercase] indicates a codepoint-width word,
 *
 * and
 *
 *  [UPPERCASE] indicates a null-terminated sequence of Unicode codepoints.
 *
 * (Null termination was chosen over a length field because a receiver can
 * always count length as a message is read in, whereas the sender might have
 * situations where precomputing length is a hassle.)
 *
 * A ``buffer'' is a piece of source text, usually a file.  However, some
 * buffers, notably those that have never been saved, may not exist on the file
 * system.
 *
 * A ``view'' is a portion of a buffer displayed.  The highlighter will only
 * update highlighting for codepoints that appear in one or more views.
 */

#define PROTOCOL_VERSION_NUMBER		0x00000000

/* Messages sent by the client (the editor) */

// Sent to request the server to terminate.
#define CLIENT_END_SESSION		0x00000000
// Sent to signal the end of support messages and the beginning of highlighting.
#define CLIENT_BEGIN_SESSION		0x00000001

// Sent before CLIENT_BEGIN_SESSION to indicate client support for a highlight
// code.  The server will only send highlights that both it and the client
// support.
// See below for the possibled highlight codes.
#define CLIENT_SUPPORT_HIGHLIGHT_CODE	0x00000101 // [highlight code]

// Sent when a buffer is no longer relevant to the highlighting process.
#define CLIENT_DISCARD_BUFFER		0x00010000 // [buffer number]
// Sent when a buffer is determined relevant to the highlighting process.
// Usually sent for any buffers being edited as well as included extensions.
#define CLIENT_INTRODUCE_BUFFER		0x00010001 // [buffer number]

// Sent when the author's intended use of a buffer, as story source text or as
// an extension, is no longer clear.  Buffers default to undecided, so this
// message is rarely used.
#define CLIENT_MARK_BUFFER_UNDECIDED	0x00010100 // [buffer number]
// Sent to indicate that a buffer is to hold story source text, not the contents
// of an extension.
#define CLIENT_MARK_BUFFER_AS_STORY	0x00010101 // [buffer number]
// Sent to indicate that a buffer is to hold the contents of an extension, not
// story source text.  The includable file name is the extension name that this
// extension can be included as or an empty string if there is no way to
// included it.  This message should be resent whenever the includable file name
// changes.
#define CLIENT_MARK_BUFFER_AS_EXTENSION	0x00010103 // [buffer number] [INCLUDABLE FILE NAME WITHOUT FILE EXTENSION]

// Sent to signal an edit that has removed codepoints from a buffer.
#define CLIENT_REMOVE_CODEPOINTS	0x00010200 // [buffer number] [inclusive lower bound] [exclusive upper bound]
// Sent to signal an edit that has added codepoints to a buffer.
#define CLIENT_ADD_CODEPOINTS		0x00010201 // [buffer number] [beginning codepoint index] [INSERTION]

// Sent when a view no longer exists.
#define CLIENT_DISCARD_VIEW		0x00020000 // [view number]
// Sent when a view of the given buffer is created.  It begins empty.
#define CLIENT_INTRODUCE_VIEW		0x00020001 // [view number] [buffer number]
// Sent when a view becomes (effectively) empty but is not destroyed.
// Useful, for instance, to prevent updates in occluded windows.
#define CLIENT_CLEAR_VIEW		0x00020002 // [view number]
// Sent when the range of codepoints visible in a view changes.
#define CLIENT_MOVE_VIEW		0x00020003 // [view number] [inclusive lower bound] [exclusive upper bound]

// Sent when a view no longer has a cursor position on which to base emphasis,
// usually when it loses focus.
#define CLIENT_CLEAR_CURSOR		0x00020100 // [view number]
// Sent when the view has focus and a cursor position on which to base emphasis,
// usually when it gains focus.  The endpoints should be the same codepoint
// index to indicate the location of the cursor when no text is selected,
// different indicies to give the endpoints of the selection if there is one.
// (Note that multiple views may have a cursor set in them at the same time, and
// emphasis is likewise per-view.)
#define CLIENT_SET_CURSOR		0x00020101 // [view number] [inclusive lower bound] [exclusive upper bound]

/* Messages sent by the server (the highlighter) */

// Sent to instruct the client to remove all highlighting in the given range.
#define SERVER_REMOVE_HIGHLIGHTS	0x00010000 // [buffer number] [inclusive lower bound] [exclusive upper bound]
// Sent to instruct the client to highlight the given range per the given code.
// See below for the possibled highlight codes.
#define SERVER_ADD_HIGHLIGHT		0x00010001 // [buffer number] [inclusive lower bound] [exclusive upper bound] [highlight code]
// Sent to instruct the client to remove all warnings in the given range.
#define SERVER_REMOVE_WARNINGS		0x00010002 // [buffer number] [inclusive lower bound] [exclusive upper bound]
// Sent to instruct the client to add warning formatting to the given range.
// Warning formatting is usually effected by underlining the text orange in
// addition to any formatting prescribed by its highlights.
#define SERVER_ADD_WARNING		0x00010003 // [buffer number] [inclusive lower bound] [exclusive upper bound]
// Sent to instruct the client to remove all errors in the given range.
#define SERVER_REMOVE_ERRORS		0x00010004 // [buffer number] [inclusive lower bound] [exclusive upper bound]
// Sent to instruct the client to add error formatting to the given range.
// Error formatting is usually effected by underlining the text red in addition
// to any formatting prescribed by its highlights.  If an error and warning
// overlap and conflict in formatting, the error should trump the warning.
#define SERVER_ADD_ERROR		0x00010005 // [buffer number] [inclusive lower bound] [exclusive upper bound]
// Sent to instruct the client to remove all hovertexts in the given range.
#define SERVER_REMOVE_HOVERTEXTS	0x00010100 // [buffer number] [inclusive lower bound] [exclusive upper bound]
// Sent to instruct the client to add hovertext to the given range.  Hovertext
// is displayed in a popup or ``tooltip'' when the mouse hovers over the
// relevant range; there should also be a keyboard command to make it visible.
#define SERVER_ADD_HOVERTEXT		0x00010101 // [buffer number] [inclusive lower bound] [exclusive upper bound] [HOVERTEXT]

// Sent to instruct the client to remove all emphasis in the given range.
#define SERVER_REMOVE_EMPHASIS		0x00020000 // [view number] [inclusive lower bound] [exclusive upper bound]
// Sent to instruct the client to emphasize the given range.  Emphasis is
// usually effected by bolding the text in addition to any formatting prescribed
// by its highlights.
#define SERVER_ADD_EMPHASIS		0x00020001 // [view number] [inclusive lower bound] [exclusive upper bound]
// Sent when suggestions sent earlier no longer apply.
#define SERVER_CLEAR_SUGGESTIONS	0x00020100 // [view number]
// Sent to suggest text to insert after the cursor.  Will be sent multiple times
// if there are multiple suggestions.
#define SERVER_MAKE_SUGGESTION		0x00020101 // [view number] [SUGGESTION]

/* Highlight codes (partly based on the Inform Technical Manual) */

#define HIGHLIGHT_ORDINARY_I7		0x00000000
#define HIGHLIGHT_I7_DELIMITER		0x00000001 // (+ and +)

#define HIGHLIGHT_ORDINARY_I6		0x00010000
#define HIGHLIGHT_I6_DELIMITER		0x00010001 // (- and -)
#define HIGHLIGHT_I6_DIRECTIVE		0x00010002
#define HIGHLIGHT_I6_KEYWORD		0x00010004
#define HIGHLIGHT_I6_FUNCTION		0x00010100
#define HIGHLIGHT_I6_FUNCTION_DELIMITER	0x00010101 // [ and ]
#define HIGHLIGHT_I6_PROPERTY_LIKE	0x00010102
#define HIGHLIGHT_I6_PROPERTY_LIKE_DELIMITER \
					0x00010103

#define HIGHLIGHT_VM_ASSEMBLY		0x00020000
#define HIGHLIGHT_VM_ASSEMBLY_DELIMITER	0x00020001 // @

#define HIGHLIGHT_COMMENT		0x00030000
#define HIGHLIGHT_COMMENT_DELIMITER	0x00030001 // !, [, and ]
#define HIGHLIGHT_DOCUMENTATION		0x00030100
#define HIGHLIGHT_DOCUMENTATION_DELIMITER \
					0x00030101 // ---- DOCUMENTATION ----
#define HIGHLIGHT_PASTE_MARKER		0x00030103 // *:

#define HIGHLIGHT_CHARACTER_LITERAL	0x00040000
#define HIGHLIGHT_CHARACTER_LITERAL_DELIMITER \
					0x00040001 // '
#define HIGHLIGHT_STRING_LITERAL	0x00040100
#define HIGHLIGHT_STRING_LITERAL_DELIMITER \
					0x00040101 // "
#define HIGHLIGHT_CHARACTER_ESCAPE	0x00040200
#define HIGHLIGHT_CHARACTER_ESCAPE_DELIMITER \
					0x00040201 // @ and @@
#define HIGHLIGHT_SUBSTITUTION		0x00040300
#define HIGHLIGHT_SUBSTITUTION_DELIMITER \
					0x00040301 // [ and ]
#define HIGHLIGHT_SEGMENTED_SUBSTITUTION \
					0x00040302
#define HIGHLIGHT_SEGMENTED_SUBSTITUTION_DELIMITER \
					0x00040303 // [ and ]

#endif
