# Parse output of scan and return in table format
# Based on https://gist.github.com/elecnix/182fa522da5dc7389975
# but without need for double indexed array, missing from busybox awk
# Optional arg: select output: return in table format for console debug, MAPI format (like old STA scan?), FAPI format (objlist)

# Usage: 
#   iw dev wlan0 scan > /tmp/scan.dump
#   cat /tmp/scan.dump | awk -v output="MAPI" -f wave_wlan_ap_scan_parse.awk
 
BEGIN {
	i=0
	if (output == "") {output = "MAPI" }
    if (output == "table") {printf "%s\t%s\t%s\t%s\t%s\t%s\t%s\n","BSS","SSID","Network Mode","Channel","Bandwidth","Encryption","RSSI"}
}

func output_mapi() {
	print "bss_" i  "=\"" e["MAC"] "\""
	print "ssid_" i  "=\"" e["SSID"] "\""
	print "network_mode_" i  "=\"" e["mode"] "\""
	print "channel_" i  "=\"" e["chan"] "\""
	print "bandwidth_" i  "=\"" e["BW"] "\""
	print "encryption_" i  "=\"" e["enc"] "\""
	print "rssi_" i  "=\"" e["sig"] "\""
}

func output_fapi() {
	print "TODO"
}

func output_table() {
	printf "%s\t%s\t%s\t%s\t%s\t%s\t%s\n",e["MAC"],e["SSID"],e["mode"],e["chan"],e["BW"],e["enc"],e["sig"]
}

# When detecting new BSS, print the previous BSS output (unless this is the first one) 
$1 == "BSS" {
    if (e["MAC"] != "") {
		if (output == "table") { output_table() }
		if (output == "MAPI")  { output_mapi()  }
		if (output == "FAPI")  { output_fapi()  }
		i=i+1
	}
    e["MAC"] = $2
    e["enc"] = "Open"
    e["BW"] = "20MHz"
}
$1 == "SSID:" {
    e["SSID"] = $2
	if (e["SSID"] == "\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00") {e["SSID"] = "--HIDDEN--"}
}

# Channel: Convert from freq
$1 == "freq:" {
    e["freq"] = $NF
	if (e["freq"] < 4000) { e["chan"] = (e["freq"] - 2412) / 5  + 1 ; e["mode"] = "BG" }
	if (e["freq"] > 5000) { e["chan"] = (e["freq"] - 5000) / 5  ; e["mode"] = "A" }
	if (e["freq"] > 4000 && e["freq"] < 5000) { e["chan"] = (e["freq"] - 4000) / 5 ; e["mode"] = "A"}
}
$1 == "signal:" {
    e["sig"] = $2 " " $3
}

$1 == "HT" && $2 == "capabilities:" {
    e["mode"] = e["mode"] "N" 
}

# Bandwidth. TODO: Identify 80MHz
# TODO: Do we want to expose secondary upper/lower to web?
$1 == "HT20" {
    e["BW"] = "20MHz" 
}
$1 == "HT40" {
    e["BW"] = "40MHz" 
}
$1 == "HT20/HT40" {
    e["BW"] = "20/40MHz" 
}

# Security: Assumption: In WPA/WPA2 mixed mode, the RSN IE is printed first (if not, mixed mode will be reported as WPA2)
$1 == "WPA:" {
    if (e["enc"] == "Open") {e["enc"] = "WPA"}
    if (e["enc"] == "WPA2") {e["enc"] = "WPA/WPA2"}
}
$1 == "RSN:" {
    e["enc"] = "WPA2"
}
$1 == "WEP:" {
    e["enc"] = "WEP"
}

END {
		if (output == "table") { output_table() }
		if (output == "MAPI") {
			output_mapi()
			if (e["MAC"] != "") { i=i+1 }
			print "ap_scan_count=\"" i "\"" }
		if (output == "FAPI") {
			output_fapi()
			if (e["MAC"] != "") { i=i+1 }
			print "TODO: ap_scan_count=\"" i "\"" }
}
 