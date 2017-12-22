/*
 * Compatability glue for systems lacking the if_nametoindex and
 * if_indextoname functions.
 *
 * The file 'if_index.c' was taken verbatimly from the GNU C Library
 * version 2.1 (990920) and is Copyright (C) 1997, 1998, 1999 Free
 * Software Foundation, Inc.
 */

/*
 * On how to use this file: Serge Caron writes:
 *
 * I installed compat-glibc-6.2-2.1.3.2.i386.rpm in my Red Hat 5.2 box
 *
 * from the directory where libbridge is stored I ran
 *
 * gcc -Wall -g -nostdinc -I- -I.
 *   -I /usr/i386-glibc21-linux/include
 *   -I /usr/lib/gcc-lib/i386-redhat-linux/2.7.23/include
 *   -c libbridge_compat.c
 *
 * Now this gives a nice .o file that keeps make happy. Running make in the top
 * directory builds brctl and brctld against glibc-2.0.7.
 */

#define _BITS_LIBC_LOCK_H 1
#define __libc_lock_define_initialized(a,b)
#define __libc_lock_lock(a)
#define __libc_lock_unlock(a)
#define __ioctl ioctl
#define __set_errno(a) {errno = (a);}
#define __socket socket
#define internal_function

#include "if_index.c"
