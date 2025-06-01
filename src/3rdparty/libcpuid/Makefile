
# Copyright 2015-2025 Rivoreo

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
# IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


CFLAGS += -D VERSION=\"0.4.0\" -Wall -Os

SOURCES := \
    cpuid_main.c \
    asm-bits.c \
    recog_amd.c \
    recog_intel.c \
    libcpuid_util.c

ifdef BUILD_DIR
OBJECTS := $(addsuffix .o,$(basename $(patsubst %,$(BUILD_DIR)/%,$(SOURCES))))
TARGET := $(BUILD_DIR)/libcpuid.a
else
OBJECTS := $(addsuffix .o,$(basename $(SOURCES)))
TARGET := libcpuid.a
endif

$(TARGET):	$(OBJECTS)
	$(AR) -r $@ $^

clean:
	rm -f $(OBJECTS) $(TARGET)

ifdef BUILD_DIR
$(BUILD_DIR)/%.o:	%.c
	$(CC) $(CFLAGS) -c $< -o $@
endif
