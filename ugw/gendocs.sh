#!/bin/sh

get_staging_dir ()
{
	echo -en "TOPDIR=\${CURDIR}\ninclude rules.mk\ninclude include/image.mk\nall:\n\t@echo \$(STAGING_DIR)\n" > /tmp/__a.mk && {
		make -f /tmp/__a.mk && rm -f /tmp/__a.mk
	}
}

STAGING_DIR=`get_staging_dir`

DOXY_TMP_FILE="./tmp_api_doc"
INPUT_DIR="$STAGING_DIR/usr/docs"
OUTPUT_DIR="./docs/ugw"
FILE_UGW="$INPUT_DIR/ugw_doxy.h"
FILE_MAINPAGE="$INPUT_DIR/ugw_mainpage.h"
FILE_UGW_FAPI="$INPUT_DIR/ugw_doxy_fapi.h"
FILE_UGW_FRAMEWORK="$INPUT_DIR/ugw_doxy_framework.h"
rm -rf $OUTPUT_DIR

gen_defs_ugw() {
	/bin/cat <<EOM >$FILE_UGW
/*! \file ugw_doxy.h
    \brief File contains the doxygen macros definitions for UGW Programmer's Reference
*/

/** \defgroup SYSFRAMEWORK System Framework API
    \brief Provides generic implementation of the core logic. It is a logical grouping of modules
           that implement the bring up, configure and run operations of the  system.
*/

/** \defgroup LIBSCAPI System Configuration API
    \brief SCAPI  manages the platform-specific parts of the system.  It offers both command line
           interfaces and equivalent IOCTLs to configure the Linux system. 
*/

/** \defgroup FAPI Functional API (FAPI)
    \brief FAPI provides an interface to manage platform specific system configure. Each module
           is offered in the form of C libraries and can be integrated with any framework.  
		
*/

EOM
}

gen_defs_ugw_fapi() {
	/bin/cat <<EOM >$FILE_UGW_FAPI
/*! \file ugw_doxy_fapi.h
    \brief File contains the FAPI doxygen macros definitions for UGW Programmer's Reference
*/

/** \defgroup FAPI_SYSTEM System
*   @ingroup FAPI
    \brief Provides functions for system configuration based abstracting hardware type and capabilities
           to higher layer APIs and processes. It provides System and Ethernet FAPI.
*/

/** \defgroup FAPI_DSL DSL
*   @ingroup FAPI
    \brief Provides functions to get or set the configurations for DSL entity through the
           interaction between the DSL SL and FAPI.
*/

/** \defgroup FAPI_WLAN WiFi
*   @ingroup FAPI
    \brief Provides functions to initialize Wi-Fi  radio, and configure security, Hot-spot, SSID, WMM, etc.
*/

/** \defgroup FAPI_LTE LTE
*   @ingroup FAPI
    \brief Provides a set of functions, used to interact with LTE modems that provide ACM interfaces for
           setup with standard V.25ter (AT) commands.		
*/

/** \defgroup FAPI_MCAST Multicast
*   @ingroup FAPI
    \brief It is split between user space application and kernel module, offering a seamless
           integration of Multicast functionality with other modules.
*/

/** \defgroup FAPI_QOS Quality of Service
*   @ingroup FAPI
    \brief QoS FAPI abstracts the complexity of where to add certain QoS objects. 
		It uses unix domain socket based IPC mechanism to submit API request to QoS daemon.		
*/

/** \defgroup TAPI Voice (TAPI)
*   @ingroup FAPI
    \brief Provides interface to initialize, configure various Telephony features. 
*/
EOM
}

gen_defs_ugw_framework() {
	/bin/cat <<EOM >$FILE_UGW_FRAMEWORK
/*! \file ugw_doxy_framework.h
    \brief File contains the framwork API doxygen macros definitions for UGW Programmer's Reference
*/

/** \defgroup LIBCAL Common Adaptation Layer
*   @ingroup SYSFRAMEWORK
    \brief CAL is the common interface for the management entities (web, UpnP, CLI, CWMP) to the
           database and the system to enable interaction.
*/

/** \defgroup LIBHELP System Helper
*   @ingroup SYSFRAMEWORK
    \brief Provides generic APIs used to communicate between the modules and assist in msg construction.
*/

/** \defgroup SYSFRAMEWORK_POLLD Polling
*   @ingroup SYSFRAMEWORK
    \brief Polld daemon provides polling framework for the system. It is capable of performing polling operation
           either at dynamic object or param level.

*/

/** \defgroup SYSFRAMEWORK_LOG Logging
*   @ingroup SYSFRAMEWORK
    \brief Provides the logger framework. It offers  User Space Logging API and Kernel Space Logging API.
*/

EOM
}

gen_mainpage() {
	/bin/cat <<EOM >$FILE_MAINPAGE
	/*!
		\mainpage Main Page
		\section DISCLAIMER Legal Notice
No license (express or implied, by estoppel or otherwise) to any intellectual property rights is granted by this
document. Intel disclaims all warranties, including without limitation, the implied warranties of merchantability,
fitness for a particular purpose, and non-infringement, as well as any warranty arising from course of performance,
course of dealing, or usage in trade.
\n
All information provided here is subject to change without notice. Intel may make changes to its test conditions
and internal reliability goals at any time. Contact your Intel representative to obtain the latest Intel product
specifications and road-maps.
\n
The products described may contain design defects or errors known as errata which may cause the product to
deviate from published specifications. Current characterized errata are available on request.
\n
Software and workloads used in performance tests may have been optimized for performance only on Intel
microprocessors. Performance tests, such as SYSmark and MobileMark, are measured using specific computer
systems, components, software, operations and functions. Any change to any of those factors may cause the
results to vary. You should consult other information and performance tests to assist you in fully evaluating your
contemplated purchases, including the performance of that product when combined with other products.
\n
AnyWAN™, CONVERGATE™, COSIC™, DSLTE™, DUALFALC™, DUSLIC™, ELIC™, EPIC™, FALC™, GEMINAX™, INCA™, ISAC™, IWORX™, OCTALFALC™, OCTAT™, QUADFALC™, SCOUT™, SEROCCO™, SICOFI™, SLIC™, SMINT™, SOCRATES™, VINAX™, TrustWorld™, VINETIC™, XWAY™, IBunnyPeople, Celeron, Celeron Inside, Centrino, Centrino logo, Chips, Core Inside, Dialogic, EtherExpress, ETOX, FlashFile, i386, i486, i960, iCOMP, InstantIP, Intel, Intel logo, Intel386, Intel486, Intel740, Intel® Media Processor CE 3100, Intel® Atom™ processor CE4100, Intel® Atom™ processor CE4200, Intel® Atom™ processor CE5300, IntelDX2, IntelDX4, IntelSX2, Intel Core, Intel Inside, Intel Inside logo, Intel. Leap ahead., Intel. Leap ahead. logo, Intel NetBurst, Intel NetMerge, Intel NetStructure, Intel SingleDriver, Intel SpeedStep, Intel StrataFlash, Intel Viiv, Intel XScale, IPLink, Itanium, Itanium Inside, MCS, MMX, MMX logo, Optimizer logo, OverDrive, Paragon, PDCharm, Pentium, Pentium II Xeon, Pentium III Xeon, Performance at Your Command, Pentium Inside, Puma, skoool, Sound Mark, The Computer Inside., The Journey Inside, VTune, Xeon, Xeon Inside and Xircom are trademarks of Intel Corporation in the U.S. and/or other countries. *Other names and brands may be claimed as the property of others. © 2015 Intel Corporation
	*/
EOM
}

gen_apidoc() {
	/bin/cat <<EOM >$DOXY_TMP_FILE
	PROJECT_NAME           = "Universal Gateway Software (UGW) Programmer's Reference"
	OUTPUT_DIRECTORY       = $OUTPUT_DIR
	CREATE_SUBDIRS         = NO
	OUTPUT_LANGUAGE        = English
	FULL_PATH_NAMES        = NO
	TAB_SIZE               = 8
	HIDE_UNDOC_MEMBERS     = YES
	HIDE_UNDOC_CLASSES     = YES
	HIDE_FRIEND_COMPOUNDS  = YES
	CASE_SENSE_NAMES       = YES
	GENERATE_TODOLIST      = YES
	SHOW_USED_FILES        = YES
	WARNINGS               = YES
	WARN_IF_UNDOCUMENTED   = YES
	WARN_IF_DOC_ERROR      = YES
	WARN_FORMAT            = "$file:$line: $text"
	INPUT                  = $INPUT_DIR
	FILE_PATTERNS          = *.h
	RECURSIVE              = NO
	GENERATE_HTML          = YES
	HTML_OUTPUT            = html
	HTML_FILE_EXTENSION    = .html
	GENERATE_LATEX         = YES
	LATEX_OUTPUT           = latex
	LATEX_CMD_NAME         = latex
	MAKEINDEX_CMD_NAME     = makeindex
	GENERATE_XML           = YES
	XML_OUTPUT             = xml
	XML_PROGRAMLISTING     = YES
	ENABLE_PREPROCESSING   = YES
	SEARCH_INCLUDES        = YES				
	EXCLUDE = \
	$INPUT_DIR/fapi_wlan_qca.h \
	$INPUT_DIR/fapi_wlan_wave.h
EOM
}

if [ ! -d $INPUT_DIR ]; then
	echo " Please compile the source code first";
	exit
fi

gen_defs_ugw
gen_defs_ugw_fapi
gen_defs_ugw_framework
gen_mainpage
gen_apidoc

doxygen $DOXY_TMP_FILE

rm $DOXY_TMP_FILE
rm $FILE_MAINPAGE
rm $FILE_UGW
rm $FILE_UGW_FAPI
rm $FILE_UGW_FRAMEWORK
exit 0

