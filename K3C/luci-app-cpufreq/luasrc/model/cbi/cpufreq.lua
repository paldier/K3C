--[[
Copyright 2018 paldier <paldier@hotmail.com>
Licensed to the public under the Apache License 2.0.
--]]
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"

local Status, Max
if SYS.call("cpufreq-info -c 0 |grep 'current CPU' |awk '{printf $5}'") then
	Status = SYS.exec("cpufreq-info -c 0 |grep 'current CPU' |awk '{printf $5}'")
	Max = SYS.exec("cpufreq-info -c 0 |grep 'hardware limits' |awk '{printf $6}'")
else
	Status = "Unknown"
	Max = "Unknown"
end

m = Map("cpufreq", translate("CPU频率设置"), translate("Intel CPU频率设置") .. "<br><br>当前频率 - " .. "<b><font color=\"red\">" .. Status .. "MHz".. "</font></b>".. "<br><br>请合理设置频率，过低会导致运行异常缓慢，频率越低温度越低")

s = m:section(TypedSection, "cpufreq", translate("标准配置"))
s.addremove = false
s.anonymous = true

enable = s:option(Flag, "enabled", translate("启用"))
enable.rmempty = false
enable.default="0"
enable.disabled="0"
enable.enabled="1"

limit = s:option(ListValue, "use", translate("可用频率"))
if Max == "800" then
limit:value("150", translate("150MHz")) 
limit:value("600", translate("600MHz")) 
limit:value("800", translate("800MHz")) 
elseif Max == "1000" then
limit:value("167", translate("167MHz")) 
limit:value("333", translate("333MHz")) 
limit:value("667", translate("667MHz")) 
limit:value("1000", translate("1000MHz")) 
elseif Max == "1200" then
limit:value("150", translate("150MHz")) 
limit:value("600", translate("600MHz")) 
limit:value("800", translate("800MHz")) 
limit:value("1200", translate("1200MHz")) 
end

local apply = luci.http.formvalue("cbi.apply")
if apply then
	os.execute("/etc/init.d/cpufreq start >/dev/null 2>&1 &")
end

return m
