#/usr/bin/lua
os.execute("mtd dump pro_info > /tmp/.restore")
local fr=io.open("/tmp/.restore", "r")
local count=96
local value
fr:seek("set", 0)
while(count>0)
do
	str=fr:read(1)
	if(string.byte(str) == 255) then
		value = str
		break;
	end
	count=count-1
end
fr:close()
count=2
local fw=io.open("/tmp/.restore", "r+")
fw:seek("set", 96)
while(count>0)
do
	fw:write(value)
	count=count-1
end
fw:close()
os.execute("mtd write /tmp/.restore pro_info")
os.execute("rm -rf /overlay/*")
os.execute("reboot")
