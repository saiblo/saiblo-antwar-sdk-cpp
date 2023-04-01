# Compiler
CXX = g++
# Compiler flags
CXXFLAGS := -std=c++11 -O2

# Include directories
INCLUDEDIRS := .
# Include files
INCLUDES := $(wildcard *.hpp)

# Source directories
SOURCEDIRS := example
# Source files
SOURCES := $(wildcard $(patsubst %, %/*.cpp, $(SOURCEDIRS)))

# Target files
TARGETS := $(patsubst %.cpp, %, $(SOURCES))


all: $(TARGETS)

$(TARGETS): %: %.cpp $(INCLUDES)
	$(CXX) $(CXXFLAGS) -I$(INCLUDEDIRS) -o $@ $<

docs: Doxyfile $(INCLUDES)
	doxygen

html: docs

latex: docs
ifeq ($(OS), Windows_NT) # Windows
	cd docs\latex && .\make.bat
else # Linux
	$(MAKE) -C docs/latex
endif

.PHONY: clean
clean:
	rm -f $(TARGETS)
