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

LUA_VERSION ?= 5.3

LUA_LIB ?= lua$(LUA_VERSION)
LUA_LIBS = -l$(LUA_LIB)
LIBS = $(LUA_LIBS) -lm -ldl

LUA_LIB_DIR ?= /usr/lib

LUA_INC_DIR ?= /usr/include/lua5.3
LUA_INCLUDES = -I$(LUA_INC_DIR)

lib: lua-cn-cbor.c $(CBOR_SRC)
	gcc $(CFLAGS) -I$(CBOR_INC_DIR) -L$(LUA_LIB_DIR) $(LUA_INCLUDES) -Ilua-compat-5.3 $(LIBS) $(CBOR_SRC) lua-cn-cbor.c -o cncbor.so
