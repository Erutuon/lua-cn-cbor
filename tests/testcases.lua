function from_hex(hex)
	return (hex:gsub("%x%x", function (x) return string.char(tonumber(x, 16)) end))
end

local cbor = require "cncbor"
local function decode_from_hex(hex)
	return cbor.decode(from_hex(hex))
end

print(require "inspect" (decode_from_hex "9f018202039f0405ffff"))

local cbor_text = from_hex("5F456279746573452074657874ff")
print(require "inspect"(cbor.decode(cbor_text)))

local repetitions = 2
local cbor_sequence = cbor_text:rep(repetitions)

local sequence = {}
for i, val in cbor.iter(cbor_sequence) do
    print("byte " .. i)
    table.insert(sequence, val)
end

assert(#sequence == repetitions)
