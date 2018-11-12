--[[
Copyright 2018 Hikaru Chang <i@rua.moe>
Licensed to the public under the Apache License 2.0.
--]]

module("luci.controller.bonuscloud", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/bonuscloud") then
		return
	end

	entry({"admin", "services", "bonuscloud"},
	firstchild(), _("BonusCloud")).dependent = false

	entry({"admin", "services", "bonuscloud", "general"},
	cbi("bonuscloud"), _("Settings"), 1)

	entry({"admin", "services", "bonuscloud", "status"}, call("action_status"))
end

local function is_running(name)
	return luci.sys.call("pidof %s >/dev/null" %{name}) == 0
end

function action_status()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		run_state = is_running("bxc-network"),
	})
end
