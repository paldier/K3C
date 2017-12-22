#!/bin/sh
rm -rf patch-mod-files
for patch in `ls *GRX500*`
do
	files=`grep -r '+++ b/' $patch |sed 's/+++ //'`
	for file in `echo $files`
	do
		echo $file >> patch-mod-files
	done
done
sort patch-mod-files|uniq > patch-mod-files-bkup
mv patch-mod-files-bkup patch-mod-files
	
for file in `cat patch-mod-files`
do
	if [ `grep -rl $file *GRX500* |wc -l` != 1 ];then
		newfile=0
		echo $file is modified in files:
		echo ------------------------------------------------- 
		patches_list=`grep -rl $file *GRX500*`
		for patchfile in `echo $patches_list`
		do
			echo $patchfile
			#echo `grep -r -B 1 "+++ $file" $patchfile |grep '\-\-\- /dev/null'`
			#if [ `grep -r -B 1 "+++ $file" $patchfile |grep '\-\-\- /dev/null'` == '--- /dev/null' ];then
			#	newfile=1
			#	echo making new file
			#fi
		done
		#if [ "$newfile" == "1" ];then
		#	echo $file is a new file
		#fi
		echo ==================================================
	fi
done

		 
