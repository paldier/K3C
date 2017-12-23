module("luci.controller.vlmcsd", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/vlmcsd") then
		return
	end
	local page
	page = entry({"admin", "network", "vlmcsd"}, cbi("vlmcsd"), _("vlmcsd"), 100)
	page.i18n = "vlmcsd"
	page.dependent = true
end
