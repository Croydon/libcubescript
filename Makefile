LIBCUBESCRIPT_CXXFLAGS = -std=c++14 -Wall -Wextra -Wshadow -Wold-style-cast -I. -fPIC -fvisibility=hidden
LIBCUBESCRIPT_LDFLAGS = -shared

LIBCUBESCRIPT_SRC = cubescript.cc
LIBCUBESCRIPT_LIB = libcubescript.so

OCTASTD_PATH = ../octastd

all: library

library: $(LIBCUBESCRIPT_LIB)

$(LIBCUBESCRIPT_LIB):
	$(CXX) $(CXXFLAGS) $(LIBCUBESCRIPT_CXXFLAGS) -I$(OCTASTD_PATH) \
	$(LDFLAGS) $(LIBCUBESCRIPT_LDFLAGS) -o $@ $(LIBCUBESCRIPT_SRC)

clean:
	rm -f $(LIBCUBESCRIPT_LIB)
