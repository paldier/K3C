#!/bin/bash

# TBD:
# 1. enter Object in case it is diffrenet then default


# input param list example:
# waveAxEnableTriggerFrameTx : bool
# waveAxVapId : string
# waveAxNumOfParticipatingStations : int
Inputfile=$1
rm -rf "$PWD/fapi_cli_code/"

function prepare_fapi_cli_code()
{
	local file mode script script_orig script_from_list script_temp description description_temp
	echo "create code for FAPI CLI";

	file=$1
	mode=$2
	script=$3
	outFile=$4

	echo "Input params: params list=$file , mode=$mode , script=$script , output file=$outFile"
	echo "Press any key to continue"

	fapiclidir="$PWD/fapi_cli_code/"
	[ ! -d $fapiclidir ] && mkdir "$fapiclidir"
	outFile="$fapiclidir$outFile"
	echo "Parametrs list is:"

	# parse the file and copy the image with correct names.
	while read -r line || [[ -n "$line" ]]
	do
		
		let index=index+1
		echo "line"$index"=$line"

		param=${line%%:*}
		format=${line##*: }
		format=${format%% *}

		# remove sapces
		param=`echo $param | sed 's/ //g'`
		format=`echo $format | sed 's/ //g'`	
		[ ! -z $script_orig	] && script=$script_orig

		script_temp=${line##*-s }
		script_from_list=${script_temp%% *}
		[ "$script_from_list" != "$param" ] && script_orig=$script && script=$script_from_list
 
		description=${line##*-d }
		[ "$description" = "$line" ] && description=""

	if  [ "$mode" = "cli" ] ; then
		# add underscore before uppercase
		param_temp=`echo $param | sed 's/[A-Z]/_&/g;s/^_//'`
		# convert all to uppercase
		param_upper_cli=${param_temp^^}
		outFile1="${outFile}_1"
		echo "    GET_$param_upper_cli," >> $outFile1
		echo "    SET_$param_upper_cli," >> $outFile1
		param_no_space=`echo $param | sed 's/ //g'`
		outFile2="${outFile}_2"
		echo "	{ \"get$param_no_space\" , GET_$param_upper_cli}," >> $outFile2
		echo "	{ \"set$param_no_space\" , SET_$param_upper_cli}," >> $outFile2
		
		if [ "$format" = "int" ] ; then
		
			outFile3="${outFile}_3"
			echo "" >> $outFile3
			echo "case GET_$param_upper_cli:" >> $outFile3
			echo "	retVal = wlan_get$param(index, (int *)&output_int);" >> $outFile3
			echo "	printf(\"cli_return=%d\\n\", output_int);" >> $outFile3
			echo "	break;" >> $outFile3
			echo "" >> $outFile3
			echo "case SET_$param_upper_cli:" >> $outFile3
			echo "	retVal = wlan_set$param(index, atoi(parameter));" >> $outFile3
			echo "	break;" >> $outFile3
		fi
		
		if [ "$format" = "bool" ] ; then
		
			outFile3="${outFile}_3"
			echo "" >> $outFile3
			echo "case GET_$param_upper_cli:" >> $outFile3
			echo "	retVal = wlan_get$param(index, &enable_bool);" >> $outFile3
			echo "	changeBooleanToChar(&enable, &enable_bool);" >> $outFile3
			echo "	printf(\"cli_return=%s\\n\", enable);" >> $outFile3
			echo "	break;" >> $outFile3
			echo "" >> $outFile3
			echo "case SET_$param_upper_cli:" >> $outFile3
			echo "	if (changeCharToBoolean(enable, &enable_bool) != UGW_FAILURE)" >> $outFile3
			echo "	{" >> $outFile3
			echo "		retVal = wlan_set$param(index, enable_bool);" >> $outFile3
			echo "	}" >> $outFile3
			echo "	break;" >> $outFile3
		fi
		
		
		if [ "$format" = "string" ] ; then
		
			outFile3="${outFile}_3"
			echo "" >> $outFile3
			echo "case SET_$param_upper_cli:" >> $outFile3
			echo "	retVal = wlan_set$param(index, parameter);" >> $outFile3
			echo "	break;" >> $outFile3
			echo "" >> $outFile3
			echo "case GET_$param_upper_cli:" >> $outFile3
			echo "	retVal = wlan_get$param(index, output_string);" >> $outFile3
			echo "	printf(\"cli_return=%s\\n\", output_string);" >> $outFile3
			echo "	break;" >> $outFile3
		fi
			
		continue
	fi

	if [ "$format" = "int" ] ; then
		echo "" >> $outFile
		echo "/**************************************************************************/" >> $outFile
		echo "/*! \fn int wlan_get$param(int index, int *value)" >> $outFile
		echo "**************************************************************************" >> $outFile
		echo "*  \brief get $param $description" >> $outFile
		echo "*  \param[in] int index - AP index" >> $outFile
		echo "*  \param[out] int *value" >> $outFile
		echo "*  \return 0 if success, negative if error / timeout" >> $outFile
		echo "***************************************************************************/" >> $outFile
		if  [ "$mode" = "header" ] ; then
			echo "int wlan_get$param(int index, int *value);" >> $outFile
			echo ""  >> $outFile
		fi
		if  [ "$mode" = "server" ] ; then
			echo "int wlan_get$param(int index, int *value)" >> $outFile
			echo "{" >> $outFile
			echo "	return intValueGet(__FUNCTION__," >> $outFile
			echo "		index," >> $outFile
			echo "		value," >> $outFile
			echo "		DEVICE_RADIO_VENDOR," >> $outFile
			echo "		\"$param\");" >> $outFile
			echo "}" >> $outFile
		fi


		echo "/**************************************************************************/" >> $outFile
		echo "/*! \fn int wlan_set$param(int index, int value)" >> $outFile
		echo " **************************************************************************" >> $outFile
		echo " *  \brief set $param $description" >> $outFile
		echo " *  \param[in] int index - AP index" >> $outFile
		echo " *  \param[in] int value" >> $outFile
		echo " *  \return 0 if success, negative if error / timeout" >> $outFile
		echo " ***************************************************************************/" >> $outFile
		if  [ "$mode" = "header" ] ; then
			echo "int wlan_set$param(int index, int value);" >> $outFile
			echo ""  >> $outFile
		fi
		if  [ "$mode" = "server" ] ; then
			echo "int wlan_set$param(int index, int value)" >> $outFile
			echo "{" >> $outFile
			echo "	return intValueSet("$script"," >> $outFile
			echo "		index," >> $outFile
			echo "		value," >> $outFile
			echo "		DEVICE_RADIO_VENDOR," >> $outFile
			echo "		\"$param\");" >> $outFile
			echo "}" >> $outFile
		fi
	fi # int 

	if [ "$format" = "bool" ] ; then
		
		echo "" >> $outFile
		echo "/**************************************************************************/" >> $outFile
		echo "/*! \fn int wlan_get$param(int index, bool * enable)" >> $outFile
		echo "**************************************************************************" >> $outFile
		echo "*  \brief get $param $description" >> $outFile
		echo "*  \param[in] int index - AP index" >> $outFile
		echo "*  \param[out]  bool *enable true or false" >> $outFile
		echo "*  \return 0 if success, negative if error / timeout" >> $outFile
		echo "***************************************************************************/" >> $outFile
		if  [ "$mode" = "header" ] ; then
			echo "int wlan_get$param(int index, bool * enable);" >> $outFile
			echo ""  >> $outFile
		fi
		if  [ "$mode" = "server" ] ; then
			echo "int wlan_get$param(int index, bool * enable)" >> $outFile
			echo "{" >> $outFile
			echo "	return boolValueGet(__FUNCTION__," >> $outFile
			echo "		index," >> $outFile
			echo "		enable," >> $outFile
			echo "		DEVICE_RADIO_VENDOR," >> $outFile
			echo "		\"$param\");" >> $outFile
			echo "}" >> $outFile
		fi

		echo "/**************************************************************************/" >> $outFile
		echo "/*! \fn int wlan_set$param(int index, bool enable)" >> $outFile
		echo " **************************************************************************" >> $outFile
		echo " *  \brief set $param $description" >> $outFile
		echo " *  \param[in] int index - AP index" >> $outFile
		echo " *  \param[in] bool enable true or false" >> $outFile
		echo " *  \return 0 if success, negative if error / timeout" >> $outFile
		echo " ***************************************************************************/" >> $outFile
		if  [ "$mode" = "header" ] ; then
			echo "int wlan_set$param(int index, bool enable);" >> $outFile
			echo ""  >> $outFile
		fi
		if  [ "$mode" = "server" ] ; then
			echo "int wlan_set$param(int index, bool enable)" >> $outFile
			echo "{" >> $outFile
			echo "	return boolValueSet("$script"," >> $outFile
			echo "		index," >> $outFile
			echo "		enable," >> $outFile
			echo "		DEVICE_RADIO_VENDOR," >> $outFile
			echo "		\"$param\");" >> $outFile
			echo "}" >> $outFile
		fi
	fi # bool

	if [ "$format" = "string" ] ; then
		
		if  [ "$mode" = "cli" ] ; then
		
				outFile3="${outFile}_3"
				echo "	case SET_COUNTRY_CODE:" >> $outFile3
				echo "		retVal = wlan_set$param(index, parameter);" >> $outFile3
				echo "		break;" >> $outFile3
				echo "	case GET_COUNTRY_CODE:" >> $outFile3
				echo "		retVal = wlan_get$param(index, output_string);" >> $outFile3
				echo "		printf(\"cli_return=%s\\n\", output_string);" >> $outFile3
				echo "		break;" >> $outFile3
				
				continue
		fi
		echo "" >> $outFile
		echo "/**************************************************************************/" >> $outFile
		echo "/*! \fn int wlan_get$param(int index, char* strValue)" >> $outFile
		echo "**************************************************************************" >> $outFile
		echo "*  \brief get $param $description" >> $outFile
		echo "*  \param[in] int index - AP index" >> $outFile
		echo "*  \param[out] int *strValue" >> $outFile
		echo "*  \return 0 if success, negative if error / timeout" >> $outFile
		echo "***************************************************************************/" >> $outFile
		if  [ "$mode" = "header" ] ; then
			echo "int wlan_get$param(int index, char* strValue);" >> $outFile
			echo ""  >> $outFile
		fi
		if  [ "$mode" = "server" ] ; then
			echo "int wlan_get$param(int index, char* strValue)" >> $outFile
			echo "{" >> $outFile
			echo "	return stringValueGet(__FUNCTION__," >> $outFile
			echo "		index," >> $outFile
			echo "		strValue," >> $outFile
			echo "		DEVICE_RADIO_VENDOR," >> $outFile
			echo "		\"$param\");" >> $outFile
			echo "}" >> $outFile
		fi

		echo "/**************************************************************************/" >> $outFile
		echo "/*! \fn int wlan_set$param(int index, char* strValue)" >> $outFile
		echo " **************************************************************************" >> $outFile
		echo " *  \brief set $param $description" >> $outFile
		echo " *  \param[in] int index - AP index" >> $outFile
		echo " *  \param[in] char* strValue" >> $outFile
		echo " *  \return 0 if success, negative if error / timeout" >> $outFile
		echo " ***************************************************************************/" >> $outFile
		if  [ "$mode" = "header" ] ; then
			echo "int wlan_set$param(int index, char* strValue);" >> $outFile
			echo ""  >> $outFile
		fi
		if  [ "$mode" = "server" ] ; then
			echo "int wlan_set$param(int index, char* strValue)" >> $outFile
			echo "{" >> $outFile
			echo "	return stringValueSet("$script"," >> $outFile
			echo "		index," >> $outFile
			echo "		strValue," >> $outFile
			echo "		DEVICE_RADIO_VENDOR," >> $outFile
			echo "		\"$param\");" >> $outFile
			echo "}" >> $outFile
		fi
	fi # string	

	done < $file

	if  [ "$mode" = "cli" ] ; then
		echo "" >> $outFile
		echo "enum" >> $outFile
		echo "{" >> $outFile
		cat $outFile1 >> $outFile
		rm -f $outFile1
		echo "};" >> $outFile
		echo "" >> $outFile	
		echo "static lookupstruct_t lookuptable[] = {" >> $outFile
		cat $outFile2 >> $outFile
		rm -f $outFile2
		echo "};" >> $outFile
		echo "" >> $outFile
		echo "switch (keyFromString(apiName))" >> $outFile
		cat $outFile3 >> $outFile
		rm -f $outFile3
		echo "	}"
	fi
}
# end of function prepare_fapi_cli_code


if [ -z "$Inputfile" ] || [ "$Inputfile" = "--help" ] || [ "$Inputfile" = "--h" ] || [ "$Inputfile" = "-h" ]; then
	echo ""
	echo ""
	echo "############################ Create FAPI CLI code help ###########################################"
	echo "bash fapi_cli_code_generate.sh <input param list> <mode:cli/header/server> <handler script or none> <output file name>"
	echo "#############################        examples:         ###########################################"
	echo "input param list example ( take from DB or clish show ):"
	echo "			waveAxEnableTriggerFrameTx : bool -s fapi_wlan_radio_set -d enable disable the trigger frame"
	echo "			waveAxVapId : string"
	echo "			waveAxNumOfParticipatingStations : int"
	echo " [-s] other script then the default script or none(set in the script call to:prepare_fapi_cli_code)"
	echo " [-d] description or none"
	echo ""
	echo "bash fapi_cli_code_generate.sh <param list file>"
	echo ""
	exit
fi

prepare_fapi_cli_code $Inputfile cli "" fapi_wlan_cli.c
echo "################### fapi_wlan_cli.c DONE ##########################"
echo "check "$outFile""
echo "###################################################################"
prepare_fapi_cli_code $Inputfile header "" wlan_config_api.h
echo "################### wlan_config_api.h DONE ########################"
echo "check "$outFile""
echo "###################################################################"
prepare_fapi_cli_code $Inputfile server fapi_wlan_radio_set wlan_config_server_api.c
echo "################### wlan_config_server_api.c DONE #################"
echo "check "$outFile""
echo "###################################################################"
