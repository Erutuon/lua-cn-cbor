CBOR_DIR = cn-cbor
CBOR_SRC_DIR = $(CBOR_DIR)/src
CBOR_INC_DIR = $(CBOR_DIR)/include

CFLAGS = -shared -fPIC -Wall -Wextra

ifdef debug
CFLAGS += -g -fsanitize=address -fno-omit-frame-pointer
else
CFLAGS += -O3
endif

CBOR_SRC_FILES = cn-cbor.c cn-error.c cn-get.c
CBOR_SRC = $(foreach file,$(CBOR_SRC_FILES),$(CBOR_SRC_DIR)/$(file))

LIBS = -llua -lm -ldl

lib: lua-cn-cbor.c $(CBOR_SRC)
	gcc $(CFLAGS) -I$(CBOR_INC_DIR) $(LIBS) $(CBOR_SRC) lua-cn-cbor.c -o cn-cbor.so
