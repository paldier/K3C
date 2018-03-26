# shellinabox
OpenWrt packet of shellinabox

$ git clone https://github.com/paldier/shellinabox.git

$ make package/feeds/k3c_packages/shellinabox/compile

bug for grx350/550
re-run this command
./configure --target=mips-openwrt-linux --host=mips-openwrt-linux --build=x86_64-linux-gnu --program-prefix="" --program-suffix="" --prefix=/usr --exec-prefix=/usr --bindir=/usr/bin --sbindir=/usr/sbin --libexecdir=/usr/lib --sysconfdir=/etc --datadir=/usr/share --localstatedir=/var --mandir=/usr/man --infodir=/usr/info --disable-nls --disable-utmp --disable-runtime-loading
