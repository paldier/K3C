/******************************************************************************

                         Copyright (c) 2012, 2014, 2015
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/
#ifndef _GSW_TYPES_H_
#define _GSW_TYPES_H_
/** \file gsw_types.h GSW Base Types */

/** \brief MAC Address Field Size.
    Number of bytes used to store MAC address information. */
#define GSW_MAC_ADDR_LEN 6
/** \brief Instantiated tables entries name  string length.
    The user can supply a name and get in return an id from Switch API. */
#define GSW_NAME_LEN	32 
/** \brief This is the unsigned 64-bit datatype. */
typedef unsigned long long    u64;
/** \brief This is the unsigned 32-bit datatype. */
typedef unsigned int    u32; 
/** \brief This is the unsigned 8-bit datatype. */
typedef unsigned char   u8;
/** \brief This is the unsigned 16-bit datatype. */
typedef unsigned short  u16;
/** \brief This is the signed 16-bit datatype. */
typedef short  i16;
/** \brief This is the signed 8-bit datatype. */
typedef char  i8;
/** \brief This is the signed 32-bit datatype. */
typedef long  i32;
#if 0
/** \brief This enumeration type has two operation states, disable and enable. */
typedef enum {
	/** Disable Operation. */
	LTQ_DISABLE		= 0,
	/** Enable Operation. */
	LTQ_ENABLE		= 1
} ltq_enDis_t;
#endif

/** \brief This enumeration type defines two boolean states: False and True. */
typedef enum {
	/** Boolean False. */
	LTQ_FALSE		= 0,
	/** Boolean True. */
	LTQ_TRUE		= 1
} ltq_bool_t;

/** \brief This is a union to describe the IPv4 and IPv6 Address in numeric representation. Used by multiple Structures and APIs. The member selection would be based upon \ref GSW_IP_Select_t */
typedef union
{
   /** Describe the IPv4 address.
       Only used if the IPv4 address should be read or configured.
       Cannot be used together with the IPv6 address fields. */
   u32	nIPv4;
   /** Describe the IPv6 address.
       Only used if the IPv6 address should be read or configured.
       Cannot be used together with the IPv4 address fields. */
   u16	nIPv6[8];
} GSW_IP_t;

/** \brief Selection to use IPv4 or IPv6.
    Used  along with \ref GSW_IP_t to denote which union member to be accessed.
*/
typedef enum
{
   /** IPv4 Type */
   GSW_IP_SELECT_IPV4	= 0,
   /** IPv6 Type */
   GSW_IP_SELECT_IPV6	= 1
} GSW_IP_Select_t;


#endif 
