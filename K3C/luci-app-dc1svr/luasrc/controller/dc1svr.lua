module("luci.controller.dc1svr", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/dc1svr") then
		return
	end
	local page
	page = entry({"admin", "services", "dc1svr"}, cbi("dc1svr"), _("DC1服务器"), 100)
	page.dependent = true
end

