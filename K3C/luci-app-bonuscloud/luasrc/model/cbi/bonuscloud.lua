--[[
Copyright 2018 Hikaru Chang <i@rua.moe>
Licensed to the public under the Apache License 2.0.
--]]
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"

local m,s,b,o
local Version,Status

if SYS.call("[ -f /usr/share/bxc/version ]") then
	Version = SYS.exec("cat /usr/share/bxc/version")
else
	Version = "Unknown"
end

if SYS.call("[ -f /usr/share/bxc/crt/client.crt ]") == 0 then
	Bound = translate("<strong><font color=\"green\">Bounded</font></strong>")
elseif SYS.call("[ -f /tmp/bxcerror ]") == 0 then
	Bound = NXFS.readfile("/tmp/bxcerror")
else
	Bound = translate("<strong><font color=\"red\">Unbound</font></strong>")
end

if SYS.call("cat /usr/share/bxc/macaddr") then
	MacAddr = SYS.exec("cat /usr/share/bxc/macaddr")
else
	MacAddr = "Unknown"
end

MacAddrShort = SYS.exec("cat /usr/share/bxc/macaddr | sed \'s/://g\'")

m = Map("bonuscloud", "%s - %s" %{translate("BonusCloud"), translate("Settings")}, translate("BonusCloud is committed to building a decentralized, trusted, global infrastructure platform that combines blockchain with cloud computing technology, and an open, shared ecosystem based on it."))
m:append(Template("bonuscloud/status"))

s = m:section(NamedSection, "general", "general", translate("General Settings"))
s.anonymous = true
s.addremove = false

o = s:option(DummyValue, "version", translate("Version"))
o.value = translate(string.format("%s", Version))

enable = s:option(Flag, "enabled", translate("Enable"))
enable.rmempty = false
function enable.cfgvalue(self, section)
	return luci.sys.init.enabled("bonuscloud") and self.enabled or self.disabled
end

o = s:option(Value, "email", translate("Email"))
o.rmempty = true
o.description = translate("<font color=\"red\"><a href=\"https://console.bonuscloud.io/signUp?refer=febe6c30ce2e11e8a498d7b0a825c162\" target=\"_blank\"><strong>Register a BonusCloud account.</strong></a><br /></font>")

o = s:option(Value, "bcode", translate("BCode"))
o.rmempty = true
o.description = translate(string.format("%s<br />", Bound))

o = s:option(Value, "sckey", translate("ServerChan Key"))
o.rmempty = true
o.description = translate("<font color=\"red\"><a href=\"http://sc.ftqq.com/?c=code\" target=\"_blank\"><strong>Get your SCKEY</strong></a><br /></font>")

o = s:option(DummyValue, "macaddr", translate("Mac Address"))
o.value = translate(string.format("%s", MacAddr))


o = s:option(FileUpload,"c1status",translate("Restore"))
o.inputstyle="reload"
o.template="bonuscloud/caupload"

if nixio.fs.access("/usr/share/bxc/crt/client.crt") then
	o = s:option(Button,"certificate",translate("Backup"))
	o.inputtitle=translate("Backup Download")
	o.inputstyle="reload"
	o.write=function()
		luci.sys.call("/usr/share/bxc/bin/bxc-main -b 2>&1 >/dev/null")
		Download()
	end
end

function enable.write(self, section, value)
	if value == "1" then
		SYS.call("/etc/init.d/bonuscloud enable >/dev/null")
		SYS.call("/etc/init.d/bonuscloud start >/dev/null")
	else
		SYS.call("/etc/init.d/bonuscloud stop >/dev/null")
		SYS.call("/etc/init.d/bonuscloud disable >/dev/null")
	end
	Flag.write(self, section, value)
end

function Download()
	local t,e
	t=nixio.open(string.format("/tmp/upload/%s-bxcbak.tar.gz", MacAddrShort),"r")
	luci.http.header('Content-Disposition',string.format('attachment; filename="%s-bxcbak.tar.gz"', MacAddrShort))
	luci.http.prepare_content("application/octet-stream")
	while true do
		e=t:read(nixio.const.buffersize)
		if(not e)or(#e==0)then
			break
		else
			luci.http.write(e)
		end
	end
	t:close()
	luci.http.close()
end
local t,e
t="/tmp/upload/"
nixio.fs.mkdir(t)
luci.http.setfilehandler(
function(o,a,i)
	if not e then
		if not o then return end
		e=nixio.open(t..o.file,"w")
		if not e then
			return
		end
	end
	if a and e then
		e:write(a)
	end
	if i and e then
		e:close()
		e=nil
	end
	luci.sys.call("/usr/share/bxc/bin/bxc-main -r 2>&1 >/dev/null")
end
)

return m