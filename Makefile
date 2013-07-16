CFLAGS=-Wall -Wextra -Isrc -DDEBUG $(OPTFLAGS) -g -Iinclude -I/usr/include/libxml2 -I/usr/local/include/freetype2 -D__ASSERTMACROS__
LINKER_FLAGS=`sdl-config --cflags --libs --static-libs` -lSDL_Image -lSDL_TTF -lSDL_Mixer -Llib -llcthw -lz -lxml2 -lchipmunk -llua -lopenal -lvorbis -lvorbisfile -logg -lfreetype
LIBS=-ldl $(OPTLIBS)
PREFIX?=/usr/local

SOURCES=$(wildcard src/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))

TEST_SRC=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

TARGET=game
SO_TARGET=$(pathsubst %.a,%.so,$(TARGET))

.PHONY: submodules
submodules:
	git submodule update --init
	rm -rf lib/liblcthw.a include/lcthw
	mkdir -p include lib
	cd submodules/liblcthw && $(MAKE) static
	cp submodules/liblcthw/build/liblcthw.a lib/
	mkdir include/lcthw
	cp submodules/liblcthw/build/include/*.h include/lcthw/

# The target build
all: submodules $(TARGET) tests

dev: CFLAGS=-g -Wall -Isrc -Wall -Wextra $(OPTFLAGS)
dev: all

$(TARGET): CFLAGS += -fPIC
$(TARGET): build $(OBJECTS)
	$(CC) $(CFLAGS) $(LINKER_FLAGS) $(OBJECTS) -o $(TARGET)

$(SO_TARGET): $(TARGET) $(OBJECTS)
	$(CC) -shared -o $@ $(OBJECTS)

build:
	@mkdir -p build
	@mkdir -p bin

# The Unit Tests
.PHONY: tests
tests: CFLAGS += $(TARGET)
tests: $(TESTS)
	   sh ./tests/runtests.sh

valgrind:
	mkdir -p log/
	VALGRIND="valgrind --log-file=log/valgrind-%p.log" $(MAKE)

# The Cleaner
clean:
	rm -rf build $(OBJECTS) $(TESTS)
	rm -f tests/tests.log
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`

# The Install
install: all
	install -d $(DESTDIR)/$(PREFIX)/lib/
	install $(TARGET) $(DESTDIR)/$(PREFIX)/lib/

# The Checker
BADFUNCS='[^_.>a-zA-Z0-9](str(n?cpy|n?cat|xfrm|n?dup|str|pbrk|tok|_)|stpn?cpy|a?sn?printf|byte_)'
check:
	@echo Files with potentially dangerous functions.
	@egrep $(BADFUNCS) $(SOURCES) || true
