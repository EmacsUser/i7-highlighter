TARGET = i7-highlighter
SOURCES = \
    annotation \
    annotation_fact \
    base_class \
    buffer \
    codepoints \
    deduction \
    delimiters \
    io \
    lexer \
    lexer_monoid \
    lexical_highlights \
    main \
    parser \
    relexer \
    session \
    token \
    type_indices

CC = g++
CFLAGS = -std=c++11 -Wall -Wno-switch -Werror -g
LFLAGS =
DFLAGS = -MM

all:	$(TARGET) TAGS

$(TARGET):	$(SOURCES:%=%.o)
	$(CC) -o $@ $(filter %.o,$^) $(LFLAGS)

$(SOURCES:%=%.o):	Makefile
	$(CC) -c -o $@ $(@:%.o=%.cpp) $(CFLAGS)

$(SOURCES:%=%.d):	%.d:%.cpp Makefile
	$(CC) $(DFLAGS) $(@:%.d=%.cpp) $(CFLAGS) | sed 's,.*\.o:,$(@:%.d=%.o) $@:	,g' > $@
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
-include $(SOURCES:%=%.d)
endif
endif

Dependencies:	$(SOURCES:%=%.d)
	sed -e 's/://g' -e 's/[^ ][^ ]*\.d//g' -e 's/[^ ][^ ]*\.o//g' -e 's/[ 	\\][ 	\\]*/ /g' $(SOURCES:%=%.d) | tr ' ' "\n" | sort | uniq | tr "\n" ' ' | sed 's/^/ALL_INPUTS =/' > $@
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
-include Dependencies
endif
endif

TAGS:	$(ALL_INPUTS)
	etags $^

clean:
	-$(RM) $(TARGET) $(SOURCES:%=%.o)

distclean:	clean
	-$(RM) $(SOURCES:%=%.d) Dependencies TAGS

.PHONY:	all clean distclean
