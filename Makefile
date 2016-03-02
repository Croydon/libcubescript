LIBCUBESCRIPT_CXXFLAGS = -std=c++14 -Wall -Wextra -I. -fPIC
LIBCUBESCRIPT_LDFLAGS = -shared

LIBCUBESCRIPT_OBJ = cubescript.o
LIBCUBESCRIPT_LIB = libcubescript.so

OCTASTD_PATH = ../octastd

all: library

library: $(LIBCUBESCRIPT_LIB)

$(LIBCUBESCRIPT_LIB): $(LIBCUBESCRIPT_OBJ)
	$(CXX) $(CXXFLAGS) $(LIBCUBESCRIPT_CXXFLAGS) -I$(OCTASTD_PATH) \
	$(LDFLAGS) $(LIBCUBESCRIPT_LDFLAGS) -o $@ $(LIBCUBESCRIPT_OBJ)

.cc.o:
	$(CXX) $(CXXFLAGS) $(LIBCUBESCRIPT_CXXFLAGS) -I$(OCTASTD_PATH) -c -o $@ $<

clean:
	rm -f $(LIBCUBESCRIPT_OBJ) $(LIBCUBESCRIPT_LIB)
