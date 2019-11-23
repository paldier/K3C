--[[
Copyright 2018 paldier <paldier@hotmail.com>
Licensed to the public under the Apache License 2.0.
--]]
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"

m = Map("dc1svr", translate("DC1服务器"), translate("DC1服务器"))

s = m:section(TypedSection, "dc1svr", translate("标准配置"))
s.addremove = false
s.anonymous = true

enable = s:option(Flag, "enabled", translate("启用"))
enable.rmempty = false
enable.default="0"
enable.disabled="0"
enable.enabled="1"

local apply = luci.http.formvalue("cbi.apply")
if apply then
	os.execute("/etc/init.d/dc1svr start >/dev/null 2>&1 &")
end

return m
