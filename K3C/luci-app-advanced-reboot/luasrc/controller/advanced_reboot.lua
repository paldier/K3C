-- Copyright 2017 Stan Grishin <stangri@melmac.net>
-- Copyright 2018 paldier <paldier@hotmail.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.advanced_reboot", package.seeall)

errorMessage = ""

    device_name = "Phicomm K3C"
    partition_one_mtd = "mtd10"
    partition_two_mtd = luci.util.trim(luci.sys.exec("grep kernelB /proc/mtd|cut -d: -f1"))
    partition_skip = 32
    boot_envvar1 = "active_bank"
    boot_envvar1_partition_one = "A"
    boot_envvar1_partition_two = "B"
    boot_envvar2 = "active_bank"
    boot_envvar2_partition_one = "A"
    boot_envvar2_partition_two = "B"
    if partition_one_mtd and partition_skip then
      partition_one_label = luci.util.trim(luci.sys.exec("dd if=/dev/" .. partition_one_mtd .. " bs=1 skip=" .. partition_skip .. " count=27" .. "  2>/dev/null"))
      n, partition_one_version = string.match(partition_one_label, '(Linux)-([%d|.]+)')
    end
    if partition_two_mtd and partition_skip then
      partition_two_label = luci.util.trim(luci.sys.exec("dd if=/dev/" .. partition_two_mtd .. " bs=1 skip=" .. partition_skip .. " count=27" .. "  2>/dev/null"))
      n, partition_two_version = string.match(partition_two_label, '(Linux)-([%d|.]+)')
    end
    if string.find(partition_one_label, "LEDE") then partition_one_os = "LEDE" end
    if string.find(partition_one_label, "OpenWrt") then partition_one_os = "OpenWrt" end
    if string.find(partition_one_label, "LTQCPE") then partition_one_os = "ISDK" end
    if string.find(partition_two_label, "LEDE") then partition_two_os = "LEDE" end
    if string.find(partition_two_label, "OpenWrt") then partition_two_os = "OpenWrt" end
    if string.find(partition_two_label, "LTQCPE") then partition_two_os = "ISDK" end
    if not partition_one_os then partition_one_os = "Unknown" end
    if not partition_two_os then partition_two_os = "Unknown" end
    if partition_one_os and partition_one_version then partition_one_os = partition_one_os .. " (Linux " .. partition_one_version .. ")" end
    if partition_two_os and partition_two_version then partition_two_os = partition_two_os .. " (Linux " .. partition_two_version .. ")" end

      if nixio.fs.access("/usr/sbin/uboot_env") then
        current_partition = luci.util.trim(luci.sys.exec("/usr/sbin/uboot_env --get --name active_bank"))
      end
    other_partition = current_partition == boot_envvar1_partition_two and boot_envvar1_partition_one or boot_envvar1_partition_two
  


function index()
  entry({"admin", "system", "advanced_reboot"}, template("advanced_reboot/advanced_reboot"), _("Advanced Reboot"), 90)
  entry({"admin", "system", "advanced_reboot", "reboot"}, call("action_reboot"))
--  if device_name then entry({"admin", "system", "advanced_reboot", "altreboot"}, post("action_altreboot")) end
  entry({"admin", "system", "advanced_reboot", "alternative_reboot"}, call("action_altreboot"))
  entry({"admin", "system", "advanced_reboot", "power_off"}, call("action_poweroff"))
end

function action_reboot()
  local uci = require "luci.model.uci".cursor()
  luci.template.render("admin_system/applyreboot", {
        title = luci.i18n.translate("Rebooting..."),
        msg   = luci.i18n.translate("The system is rebooting now.<br /> DO NOT POWER OFF THE DEVICE!<br /> Wait a few minutes before you try to reconnect. It might be necessary to renew the address of your computer to reach the device again, depending on your settings."),
        addr  = luci.ip.new(uci:get("network", "lan", "ipaddr")) or "192.168.1.1"
      })
  luci.sys.reboot()
end

function action_altreboot()
  local uci = require "luci.model.uci".cursor()
  local zyxelFlagPartition, zyxelBootFlag, zyxelNewBootFlag, errorCode, curEnvSetting, newEnvSetting
  errorMessage = ""
  errorCode = 0
  if luci.http.formvalue("cancel") then
    luci.http.redirect(luci.dispatcher.build_url('admin/system/advanced_reboot'))
--    luci.template.render("advanced_reboot/advanced_reboot",{
--      device_name=device_name,
--      boot_envvar1_partition_one=boot_envvar1_partition_one,
--      partition_one_os=partition_one_os,
--      boot_envvar1_partition_two=boot_envvar1_partition_two,
--      partition_two_os=partition_two_os,
--      current_partition=current_partition,
--      errorMessage = luci.i18n.translate("Alternative reboot cancelled.")})
    return
  end
  local step = tonumber(luci.http.formvalue("step") or 1)
  if step == 1 then
    if device_name and nixio.fs.access("/usr/sbin/uboot_env") then
      luci.template.render("advanced_reboot/alternative_reboot",{})
    else
      luci.template.render("advanced_reboot/advanced_reboot",{errorMessage = luci.i18n.translate("No access to fw_printenv or fw_printenv!")})
    end
  elseif step == 2 then
    if boot_envvar1 or boot_envvar2 then -- Linksys devices
      if boot_envvar1 then
        curEnvSetting = luci.util.trim(luci.sys.exec("/usr/sbin/uboot_env --get --name active_bank"))
        if not curEnvSetting then
          errorMessage = errorMessage .. luci.i18n.translate("Unable to obtain firmware environment variable") .. ": " .. boot_envvar1 .. ". "
          luci.util.perror(luci.i18n.translate("Unable to obtain firmware environment variable") .. ": " .. boot_envvar1 .. ".")
        else
          newEnvSetting = curEnvSetting == boot_envvar1_partition_one and boot_envvar1_partition_two or boot_envvar1_partition_one
          errorCode = luci.sys.call("/usr/sbin/uboot_env --set --name " .. boot_envvar1 .. " --value " .. newEnvSetting .. "  1>/dev/null")
          if newEnvSetting == "A" then
           luci.sys.call("/usr/sbin/uboot_env --set --name update_chk --value 3   1>/dev/null")
          else
           luci.sys.call("/usr/sbin/uboot_env --set --name update_chk --value 1   1>/dev/null")
          end
            if errorCode ~= 0 then
              errorMessage = errorMessage .. luci.i18n.translate("Unable to set firmware environment variable") .. ": " .. boot_envvar1 .. " " .. luci.i18n.translate("to") .. " " .. newEnvSetting .. ". "
              luci.util.perror(luci.i18n.translate("Unable to set firmware environment variable") .. ": " .. boot_envvar1 .. " " .. luci.i18n.translate("to") .. " " .. newEnvSetting .. ".")
            end
        end
      end
    end
    if errorMessage == "" then
      luci.template.render("admin_system/applyreboot", {
            title = luci.i18n.translate("Rebooting..."),
            msg   = luci.i18n.translate("The system is rebooting to an alternative partition now.<br /> DO NOT POWER OFF THE DEVICE!<br /> Wait a few minutes before you try to reconnect. It might be necessary to renew the address of your computer to reach the device again, depending on your settings."),
            addr  = luci.ip.new(uci:get("network", "lan", "ipaddr")) or "192.168.1.1"
          })
      luci.sys.reboot()
    else
      luci.template.render("advanced_reboot/advanced_reboot",{
        device_name=device_name,
        boot_envvar1_partition_one=boot_envvar1_partition_one,
        partition_one_os=partition_one_os,
        boot_envvar1_partition_two=boot_envvar1_partition_two,
        partition_two_os=partition_two_os,
        current_partition=current_partition,
        errorMessage = errorMessage})
    end
  end
end

function action_poweroff()
  local uci = require "luci.model.uci".cursor()
  if luci.http.formvalue("cancel") then
    luci.http.redirect(luci.dispatcher.build_url('admin/system/advanced_reboot'))
    return
  end
  local step = tonumber(luci.http.formvalue("step") or 1)
  if step == 1 then
    if nixio.fs.access("/sbin/poweroff") then
      luci.template.render("advanced_reboot/power_off",{})
    else
      luci.template.render("advanced_reboot/advanced_reboot",{})
    end
  elseif step == 2 then
    luci.template.render("admin_system/applyreboot", {
          title = luci.i18n.translate("Shutting down..."),
          msg   = luci.i18n.translate("The system is shutting down now.<br /> DO NOT POWER OFF THE DEVICE!<br /> It might be necessary to renew the address of your computer to reach the device again, depending on your settings."),
          addr  = luci.ip.new(uci:get("network", "lan", "ipaddr")) or "192.168.1.1"
        })
    luci.sys.call("/sbin/poweroff")
  end
end
