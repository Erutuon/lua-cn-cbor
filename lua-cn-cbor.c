#include <stdlib.h>
#include <stdio.h>
#include "compat-5.3.h"
#include "cn-cbor/cn-cbor.h"

#define EPRINTF(...) fprintf(stderr, __VA_ARGS__)
#define CRASH_WITH_MSG(...) EPRINTF(__VA_ARGS__), exit(EXIT_FAILURE)
#define ARRAY_SIZE(array) (sizeof (array) / sizeof ((array)[0]))

static const char * const type_names[] = {
	"FALSE",
	"TRUE",
	"NULL",
	"UNDEF",
	"UINT",
	"INT",
	"BYTES",
	"TEXT",
	"BYTES_CHUNKED",
	"TEXT_CHUNKED",
	"ARRAY",
	"MAP",
	"TAG",
	"SIMPLE",
	"DOUBLE",
	"FLOAT",
	"INVALID"
};

#ifndef LUA_MAXINTEGER
#define LUA_MAXINTEGER __LONG_LONG_MAX__
#endif

static int lua_push_cn_cbor (lua_State * L, cn_cbor * val) {
	switch (val->type) {
		case CN_CBOR_ARRAY: {
			lua_createtable(L, val->length, 0);
			if (val->length > 0) {
				int count = 0;
				for (cn_cbor * item = val->first_child; item != NULL; item = item->next) {
					lua_push_cn_cbor(L, item);
					lua_seti(L, -2, (lua_Integer) ++count);
				}
			}
			break;
		}
		case CN_CBOR_BYTES:
		case CN_CBOR_TEXT: {
			lua_pushlstring(L, val->v.str, val->length);
			break;
		}
		case CN_CBOR_BYTES_CHUNKED:
		case CN_CBOR_TEXT_CHUNKED: {
			for (cn_cbor * item = val->first_child; item != NULL; item = item->next)
				lua_pushlstring(L, item->v.str, item->length);
			lua_concat(L, val->length);
			break;
		}
		case CN_CBOR_MAP: {
			lua_createtable(L, 0, val->length / 2);
			if (val->length > 0) {
				for (cn_cbor * item = val->first_child; item != NULL; item = item->next) {
					// Push key.
					lua_push_cn_cbor(L, item);
					
					// Push value.
					item = item->next;
					lua_push_cn_cbor(L, item);
					
					lua_settable(L, -3);
				}
			}
			break;
		}
		case CN_CBOR_DOUBLE: {
			lua_pushnumber(L, (lua_Number) val->v.dbl); break;
		}
		case CN_CBOR_FLOAT: {
			lua_pushnumber(L, (lua_Number) val->v.f); break;
		}
		case CN_CBOR_FALSE:
		case CN_CBOR_TRUE: {
			lua_pushboolean(L, val->type == CN_CBOR_TRUE); break;
		}
		case CN_CBOR_INT: {
			if (val->v.sint > (int) LUA_MAXINTEGER)
				return luaL_error(L, "integer overflow");
			lua_pushinteger(L, (lua_Integer) val->v.sint); break;
		}
		case CN_CBOR_UINT: {
			if (val->v.uint > (unsigned) LUA_MAXINTEGER)
				return luaL_error(L, "integer overflow");
			lua_pushinteger(L, (lua_Integer) val->v.uint); break;
		}
		case CN_CBOR_NULL: { // TODO: Fix nil as value in table?
			lua_pushnil(L); break;
		}
		case CN_CBOR_INVALID:
		case CN_CBOR_SIMPLE:
		case CN_CBOR_TAG:
		case CN_CBOR_UNDEF: {
			return luaL_error(L, "conversion for type '%s' unimplemented", type_names[val->type]);
		}
	}
	return 1;
}

static int lua_cn_cbor_decode(lua_State * L) {
	size_t len;
	const uint8_t * cbor = (const uint8_t *) luaL_checklstring(L, 1, &len);
	lua_Integer pos = luaL_optinteger(L, 2, 1) - 1; // one-based indexing
	if (pos < 0 || (size_t) pos >= len) {
		lua_pushnil(L);
		return 1;
	}
	cn_cbor_errback err;
	cn_cbor * val = cn_cbor_decode(cbor + pos, len - pos, &err);
	if (val == NULL)
		return luaL_error(L, "%s", cn_cbor_error_str[err.err]);
	lua_push_cn_cbor(L, val);
	cn_cbor_free(val);
	return 1;
}

static int lua_cn_cbor_decode_next(lua_State * L) {
	size_t len;
	const uint8_t * cbor = (const uint8_t *) luaL_checklstring(L, 1, &len);
	lua_Integer pos = luaL_optinteger(L, 2, 1) - 1; // one-based indexing
	if (pos < 0 || (size_t) pos >= len) {
		lua_pushnil(L);
		return 1;
	}
	cn_cbor_errback err;
	cn_cbor * val = cn_cbor_decode(cbor + pos, len - pos, &err);
	if (val == NULL)
		return luaL_error(L, "%s", cn_cbor_error_str[err.err]);
	lua_pushinteger(L, pos + 1 + err.pos);
	lua_push_cn_cbor(L, val);
	cn_cbor_free(val);
	return 2;
}

static int lua_cn_cbor_iter(lua_State * L) {
	luaL_checkstring(L, 1);
	lua_settop(L, 1);
	lua_pushcfunction(L, &lua_cn_cbor_decode_next);
	lua_insert(L, -2);
	lua_pushinteger(L, 1);
	return 3;
}

static const luaL_Reg funcs[] = {
	{ "decode", lua_cn_cbor_decode },
	{ "decode_next", lua_cn_cbor_decode_next },
	{ "iter", lua_cn_cbor_iter },
	{ NULL, NULL }
};

int luaopen_cncbor(lua_State * L) {
	luaL_newlib(L, funcs);
	
	lua_createtable(L, ARRAY_SIZE(type_names), 0);
	for (size_t i = 0; i < ARRAY_SIZE(type_names); ++i) {
		lua_pushstring(L, type_names[i]);
		lua_seti(L, -2, i + 1); // one-based indexing
	}
	lua_setfield(L, -2, "types");
	
	return 1;
}