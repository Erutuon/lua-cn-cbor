function from_hex(hex)
	return (hex:gsub('%x%x', function (x) return string.char(tonumber(x, 16)) end))
end

local cbor = require 'cn-cbor'
local function decode_from_hex(hex)
	return cbor.decode(from_hex(hex))
end

print(require 'inspect'(decode_from_hex '9f018202039f0405ffff'))
print(require 'inspect'(decode_from_hex '5F456279746573452074657874ff'))

local data = assert(io.open('template_iter/m.cbor', 'rb')):read'a'
local words = {}
local ok, err = pcall(function()
	for pos, template in cbor.iter(data) do
		if template.parameters["1"] == "peo" then
			table.insert(words, template.parameters["2"])
		end
	end
end)
if not ok then print(err) end
print(table.concat(words, ", "))