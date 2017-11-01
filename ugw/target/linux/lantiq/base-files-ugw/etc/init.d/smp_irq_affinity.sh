#!/bin/sh /etc/rc.common

START=998

        if [ ! "$CONFIGLOADED" ]; then
        	if [ -r /etc/rc.d/config.sh ]; then
                	. /etc/rc.d/config.sh 2>/dev/null
                	CONFIGLOADED="1"
        	fi
        fi


start(){
	if [ -n "$CONFIG_PACKAGE_KMOD_SMVP" -a "$CONFIG_PACKAGE_KMOD_SMVP" = "1" ]; then
		# irq assigned to CPU0
		for irq_name in dma-core ltqusb2_oc mtlk ltqusb_hcd_1 ltqusb_hcd_2 ltqusb1_oc
		do
			grep -w $irq_name /proc/interrupts | awk '{print $1}' | sed  's/://' | while read irq ;do echo 1 > /proc/irq/$irq/smp_affinity; done
		done

		# irq assigned to CPU1
		for irq_name in  a5_mailbox0_isr a5_mailbox_isr d5_mailbox_isr e5_mailbox_isr DFEIR wifi0 wifi1
		do
			grep -w $irq_name /proc/interrupts | awk '{print $1}' | sed  's/://' | while read irq ;do echo 2 > /proc/irq/$irq/smp_affinity; done
		done
		# put lan on vpe1
		echo 2 > /proc/irq/137/smp_affinity
		
		#put 2.4G QCA on vpe 2
		echo 4 > /proc/irq/144/smp_affinity
	fi
}
