#!/bin/sh /etc/rc.common
#START=99

PMUTILROOT="/opt/lantiq/bin"
bin_dir=/opt/lantiq/bin 
ppmd_bin=ppmd 

start() {
	if [ A"$CONFIG_LTQ_CPUFREQ_ENABLED" = "A1" -a A$coc_status = "A1" ]; then 
		if [ -f $PMUTILROOT/pm_util ]; then
			${bin_dir}/pm_util -yon&
		fi
	fi
	if [ A"$CONFIG_LTQ_DVS_ENABLED" = "A1" ]; then 
		if [ -f $PMUTILROOT/pm_util ]; then
			${bin_dir}/pm_util -xon&
		fi
	fi
	if [ A"$CONFIG_LTQ_SCALE_DOWN_ON_HIGH_TEMP" = "A1" ] ||
		[ A"$CONFIG_LTQ_TEMP_EMER_SHUT_DOWN" = "A1" ]; then 
			if [  "$(ps | grep -c ppmd)" -eq 1 ]; then
				echo "Power Policy Management Daemon running" 
				${bin_dir}/${ppmd_bin} -sp1000
				${bin_dir}/${ppmd_bin} -o& 
			fi
	fi
}

stop() {
	if [ A"$CONFIG_LTQ_CPUFREQ_ENABLED" = "A1" -a A$coc_status = "A0" ]; then 
		if [ -f $PMUTILROOT/pm_util ]; then
			${bin_dir}/pm_util -yoff&
		fi
	fi
	if [ A"$CONFIG_LTQ_DVS_ENABLED" = "A1" ]; then 
		if [ -f $PMUTILROOT/pm_util ]; then
			${bin_dir}/pm_util -xoff&
		fi
	fi
	if [ A"$CONFIG_LTQ_SCALE_DOWN_ON_HIGH_TEMP" = "A1" ] ||
		[ A"$CONFIG_LTQ_TEMP_EMER_SHUT_DOWN" = "A1" ]; then 
		if [ -e ${bin_dir}/${ppmd_bin} ];then
			echo "Power Policy Management Daemon disabled" 
			${bin_dir}/${ppmd_bin} -sp0
			killall ${ppmd_bin}
		else
			echo "${bin_dir}/${ppmd_bin} not found"
			exit 1
		fi 
	fi

}
