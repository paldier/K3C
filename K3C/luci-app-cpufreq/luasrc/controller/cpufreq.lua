module("luci.controller.cpufreq", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/cpufreq") then
		return
	end
	local page
	page = entry({"admin", "services", "cpufreq"}, cbi("cpufreq"), _("CPU频率设置"), 100)
	page.dependent = true
end

