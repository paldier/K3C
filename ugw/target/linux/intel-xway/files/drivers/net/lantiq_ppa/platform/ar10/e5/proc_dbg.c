/******************************************************************************
**
** FILE NAME    : proc_dbg.c
** PROJECT      : UEIP
** MODULES      : ATM + MII0/1 Acceleration Package (AR9 PPA A5)
**
** DATE         : 23 MAR 2008
** AUTHOR       : Xu Liang
** DESCRIPTION  : ATM + MII0/1 Driver with Acceleration Firmware (A5)
** COPYRIGHT    :   Copyright (c) 2006
**          Infineon Technologies AG
**          Am Campeon 1-12, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
** HISTORY
** $Date        $Author         $Comment
** 23 MAR 2008  Xu Liang        Initiate Version
*******************************************************************************/

/*
 * ####################################
 *              Head File
 * ####################################
 */

/*
 *  Common Head File
 */
#include <linux/autoconf.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <asm/unistd.h>
#include <asm/delay.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/errno.h>

/*
 *  Chip Specific Head File
 */

/*
 * ####################################
 *              Definition
 * ####################################
 */
#define MAX_STVAR_NUM     32
#define MAX_NAME_LEN      32
#define MAX_ST_NUM        128
#define MAX_MAP_NUM       16

#define PD_INLINE            inline

 /*
 * ####################################
 *              Data Type
 * ####################################
 */    

typedef struct flex_var{
    uint8_t var_name[MAX_NAME_LEN];
    uint32_t var_start;
    uint32_t var_len;
}PPA_FLEX_VAR;

typedef struct flex_st{
    uint8_t st_name[MAX_NAME_LEN];
    uint32_t st_avail;
    uint32_t st_addr;
    uint32_t st_paddr;
    uint32_t st_size;
    PPA_FLEX_VAR st_var[MAX_STVAR_NUM];
}PPA_FLEX_ST;

typedef struct flex_map{
    uint32_t mp_avail;
    uint32_t mps_addr;
    uint32_t mpe_addr;
    uint32_t mpfpi_addr;
}PPA_FLEX_MAP;

enum{
    FLEX_NONE = 0,
    FLEX_READ = 1,
    FLEX_DEL,
    FLEX_EXIT,
    FLEX_STRUCT,
    FLEX_SHOW,
    FLEX_LIST,
    FLEX_WRITE,
    FLEX_MAP,
    FLEX_PMAP,
    FLEX_HELP
};

/*
 *  Helper Macro
 */
#define NUM_ENTITY(x)                           (sizeof(x) / sizeof(*(x)))
#define BITSIZEOF_UINT32                        (sizeof(uint32_t) * 8)

/*
 * ####################################
 *             Declaration
 * ####################################
 */

static void show_flex_struct(int idx);
static uint32_t get_var_value(uint32_t *p, uint32_t start, uint32_t len);
static void set_var_value(uint32_t *p, uint32_t start, uint32_t len, uint32_t value);
static void set_flex_val(int idx, uint32_t addr, uint32_t startno, uint32_t endno, char *member, uint32_t value);
static void print_flex_struct(int idx, uint32_t addr, uint32_t startno, uint32_t endno);
static void print_flex_help(void);
static uint32_t get_fpi_addr(uint32_t SB_addr);
static void parse_read_st_name(char *str, int *idx, uint32_t *startno, uint32_t *endno);
static void parse_write_st_name(char **str, char **var_name, int *idx, uint32_t *startno, uint32_t *endno, uint32_t *value);
static int proc_write_dbg_print(struct file *file, const char *buf, unsigned long count, void *data);
static PD_INLINE void _proc_file_create(void);
static PD_INLINE void _proc_file_delete(void);
static PD_INLINE int pd_get_number(char **p, int *len, int is_hex);
static PD_INLINE int pd_get_token(char **p1, char **p2, int *len, int *colon);
static PD_INLINE int pd_stricmp(const char *p1, const char *p2);
static PD_INLINE int pd_strincmp(const char *p1, const char *p2, int n);
static PD_INLINE void pd_ignore_space(char **p, int *len);
static PD_INLINE void *pd_memcpy(unsigned char *dest, const unsigned char *src, unsigned int count);

/*
 * ####################################
 *            Local Variable
 * ####################################
 */

static PPA_FLEX_ST g_st_flex[MAX_ST_NUM];
static PPA_FLEX_MAP  g_map_flex[16];
static struct proc_dir_entry *g_flex_proc_dir = NULL;
static int g_flex_proc_dir_flag = 0;


/*
 * ####################################
 *            Local Function
 * ####################################
 */

static int proc_write_dbg_print(struct file *file, const char *buf, unsigned long count, void *data)
{
    char *p1, *p2, *p3;
    int len;
    int colon;
    char local_buf[2048];
    int i, idx;
    int instn = FLEX_NONE;
    int startno, endno, value;
    uint32_t addr;

    len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
    len = len - copy_from_user(local_buf, buf, len);
    local_buf[len] = 0;

    p1 = local_buf;
    colon = 1;

    while ( pd_get_token(&p1, &p2, &len, &colon) )
    {
        if ( pd_stricmp(p1, "r") == 0 || pd_stricmp(p1, "read") == 0 ){
            instn = FLEX_READ;
            break;
        }else if(pd_stricmp(p1, "d") == 0 || pd_stricmp(p1, "del") == 0){
            instn = FLEX_DEL;
            break;
        }else if(pd_stricmp(p1, "e") == 0 || pd_stricmp(p1, "exit") == 0){
            instn = FLEX_EXIT;
            break;
        }else if(pd_stricmp(p1, "struct") == 0){
            instn = FLEX_STRUCT;
            break;
        }else if(pd_stricmp(p1, "s") == 0 || pd_stricmp(p1, "show") == 0){
            instn = FLEX_SHOW;
            break;
        }else if(pd_stricmp(p1, "l") == 0 || pd_stricmp(p1, "list") == 0){
            instn = FLEX_LIST;
            break;
        }else if(pd_stricmp(p1, "w") == 0 || pd_stricmp(p1, "write") == 0){
            instn = FLEX_WRITE;
            break;
        }else if(pd_stricmp(p1, "m") == 0 || pd_stricmp(p1, "map") == 0){
            instn = FLEX_MAP;
            break;
        }else if(pd_stricmp(p1, "pmap") == 0){
            instn = FLEX_PMAP;
            break;
        }else if(pd_stricmp(p1, "h") == 0 || pd_stricmp(p1, "help") == 0){
            instn = FLEX_HELP;
            break;
        }
        p1 = p2;
        colon = 1;
    }

    pd_ignore_space(&p2, &len);
    p1 = p2;
    switch(instn){
        case FLEX_READ: 
            if(len == 0){// print all
                for(i = 0; i < MAX_ST_NUM; i ++){
                    print_flex_struct(i, 0, 0, 0);
                }
            }else{
                /*format: echo r struct_name[startno..endno] @reg > /proc/eth/print */
                p1 = p2;
                pd_get_token(&p1, &p2, &len, &colon); //get structure name

                parse_read_st_name(p1, &idx, &startno, &endno);
                
                if(idx < 0 || idx >= MAX_ST_NUM || endno < startno){
                    printk("idx: %d, startno: %d, endno %d\n", idx, startno, endno);
                    goto print_exit;
                }

                addr = 0;
                pd_ignore_space(&p2, &len);
                if(len && (*p2 == '@')){
                    p2 ++; len --;
                    addr = (uint32_t)pd_get_number(&p2, &len, 1);
                    addr = get_fpi_addr(addr);
                }
                printk("idx: %d, 0x%0x, %d:%d\n", idx, addr, startno, endno);
                print_flex_struct(idx, addr, startno, endno);
            }
            break;
        case FLEX_WRITE:
            /*format: struct_name.var[startno..endno]=value @addr*/
            pd_get_token(&p1, &p2, &len, &colon);
            parse_write_st_name(&p1, &p3, &idx, &startno, &endno, &value);
            
            if(p1 == p3 || idx < 0 || idx > MAX_ST_NUM || startno > endno){
                printk("Error: idx: %d, startno: %d, endno: %d, struct: %s, member: %s\n", idx, startno, endno,p1,p3);
                goto print_exit;
            }
            
            pd_ignore_space(&p2, &len);
            addr = 0;
            if(len && (*p2 == '@')){
                p2 ++; len --;
                addr = pd_get_number(&p2, &len, 1);
                addr = get_fpi_addr(addr);
            }
            printk("idx: %d, 0x%x, %d:%d, %s=0x%x\n", idx, addr, startno, endno, p3, value);
            set_flex_val(idx, addr, startno, endno, p3, value);
            
            break;
        case FLEX_DEL: 
            idx = pd_get_number(&p2, &len, 0);
            if(idx >= 0 && idx < MAX_ST_NUM){
                memset(&g_st_flex[idx], 0, sizeof(g_st_flex[idx]));
	        printk("NO. %d Deleted\n", idx);
            }
            break;
        case FLEX_EXIT:            
            memset(g_st_flex, 0, sizeof(g_st_flex));
            memset(g_map_flex, 0, sizeof(g_map_flex));
	        printk("All sturcture cleared\n");
            break;
        case FLEX_STRUCT:
            for( i = 0; i < MAX_ST_NUM; i ++){
                if(g_st_flex[i].st_avail == 0){
                    idx = i;
                    break;
                }
                if(i >= MAX_ST_NUM)
                    goto print_exit;
            }
            
            if(pd_get_token(&p1, &p2, &len, &colon)){
                for(i = 0; p1[i]!= 0 && p1[i]!='{'; i ++);
                if(p1[i] == '{'){
                    p1[i] = 0;
                }
                if(idx >= MAX_ST_NUM){
                    printk("Struct Buffer full!\n");
                    goto print_exit;
                }
                pd_memcpy(g_st_flex[idx].st_name,p1, sizeof(g_st_flex[idx].st_name));
                i = 0;
		        colon = 2;
                p3 = p1=p2;
                addr = 0;
                while(pd_get_token(&p1, &p2, &len, &colon)){
                    if(colon == 2){
                        value = pd_get_number(&p2, &len, 0);
                        if(value <= 0)
                        	continue;

                        if(i < MAX_STVAR_NUM){
                            pd_memcpy(g_st_flex[idx].st_var[i].var_name, p3, sizeof(g_st_flex[idx].st_var[i].var_name));

                            g_st_flex[idx].st_var[i].var_start = g_st_flex[idx].st_size;
                            g_st_flex[idx].st_var[i].var_len = value;
                            g_st_flex[idx].st_size += value;
                            i ++;
                        }
                    }else if(colon == 3){//register address follow the structrue definition
                        addr = pd_get_number(&p2, &len, 1);
                        g_st_flex[idx].st_paddr = addr;
                        addr = get_fpi_addr(addr);
                        g_st_flex[idx].st_addr = addr;
                    }else{
                    	p3 = p1;
                    }
                    p1 = p2;
		            colon = 2;
                }
            }
	        g_st_flex[idx].st_avail = 1;

            break;
        case FLEX_MAP:
            for(i = 0; i < MAX_MAP_NUM; i ++){
                if(g_map_flex[i].mp_avail == 0){
                    break;
                }
            }
            if(i >= MAX_MAP_NUM){
                printk("Map table is full\n");
                goto print_exit;
            }

            idx = i;
            g_map_flex[idx].mps_addr = pd_get_number(&p2, &len, 1);
            pd_ignore_space(&p2, &len);
            g_map_flex[idx].mpe_addr = pd_get_number(&p2, &len, 1);
            pd_ignore_space(&p2, &len);
            g_map_flex[idx].mpfpi_addr = pd_get_number(&p2, &len, 1);

            if( (g_map_flex[idx].mps_addr > g_map_flex[idx].mpe_addr ) || !g_map_flex[idx].mpfpi_addr){
                printk("Error: Map SB_start: 0x%x, SB_end: 0x%x,  FPI_Start: 0x%x\n",
                    g_map_flex[idx].mps_addr, g_map_flex[idx].mpe_addr, g_map_flex[idx].mpfpi_addr);
                goto print_exit;
            }
            g_map_flex[idx].mp_avail = 1;
            
            break;
            
        case FLEX_PMAP:
            for(i = 0; i < MAX_MAP_NUM; i ++){
                if(g_map_flex[i].mp_avail ){
                    printk("SB_start = 0x%4x, SB_end = 0x%4x, FPI_start = 0x%08x\n",
                            g_map_flex[idx].mps_addr, g_map_flex[idx].mpe_addr, g_map_flex[idx].mpfpi_addr);
                }
            }
            break;
        case FLEX_LIST:
            show_flex_struct(-1);
            break;
        case FLEX_SHOW:
            idx = MAX_ST_NUM;
            if(len > 0){
                i = len;
                idx = pd_get_number(&p2,&len,0);
                if(i == len){//not number, assume we were given struct name
                    pd_get_token(&p1, &p2, &len, &colon);
                    for(i = 0; i < MAX_ST_NUM; i ++){
                        if(strncmp(p1, g_st_flex[i].st_name, sizeof(g_st_flex[i].st_name)) == 0){
                            break;
                        }
                    }
                    idx = i;
                }
            }
            printk("show idx: %d\n", idx);
            show_flex_struct(idx);
            break;
        case FLEX_HELP:
            print_flex_help();
        default:
            break;
    }

print_exit:
    return count;
}

static void print_flex_help()
{
    printk("\n");
    printk("echo struct_definition[@ address]               > /proc/eth/print   :Add structure definition\n");
    printk("echo s/show                                     > /proc/eth/print   :Show all input structures\n");
    printk("echo r/read st_name [startno[..endno][@addr]]    > /proc/eth/print   :Print structure values\n");
    printk("echo d/del index                                > /proc/eth/print   :Delete the structure index indicated\n");
    printk("echo e/exit                                     > /proc/eth/print   :Remove all the structures\n");
    printk("echo l/list                                     > /proc/eth/print   :List all the structure names\n");
    printk("echo w/write struct_name[startno[..endno].var=value\n"
           "    [@addr]]                                    > /proc/eth/print   :Set new value to the structure member\n"); 
    printk("echo m/map SB_start SB_end FPI_start            > /proc/eth/print   :Set map from SB address to FPI address\n");
    printk("echo h/help                                     > /proc/eth/print   :Print help\n");

    return;
}

static uint32_t get_fpi_addr(uint32_t SB_addr)
{
    int i, offset;
    uint32_t pfi_addr = 0;
    
    if(SB_addr > 0xFFFF){ //no need to do transfer
        return SB_addr;
    }

    for(i = 0; i < MAX_MAP_NUM; i ++){
        if(!g_map_flex[i].mp_avail){
            continue;
        }

        if(SB_addr >= g_map_flex[i].mps_addr && SB_addr <= g_map_flex[i].mpe_addr){
            offset = SB_addr - g_map_flex[i].mps_addr;
            pfi_addr = g_map_flex[i].mpfpi_addr + (offset << 2);
        }
    }
    
    if(!pfi_addr){
        printk("Cannot transfer the given SB addr: 0x%x\n", SB_addr);
    }

    return pfi_addr;
}

static void parse_write_st_name(char **str, char **var_name, int *idx, uint32_t *startno, uint32_t *endno, uint32_t *value)
{
    //format: struct_name[startno..endno].var=value

    char *p, *pstart, *p2;
    int i, j, len, nlen;

    *idx = -1;
    *startno = *endno = *value = 0;
    *var_name = *str;

    pstart = p = *str;
    len = strlen(*str);

    for(j = 0; len > 0 && *p != '['; p ++, len --){
        if(*p == '.'){
            j = 1;
            break;
        }
    }

    *p = 0;
    
    p2 = pstart;
    nlen = strlen(pstart);
    i = pd_get_number(&pstart,&nlen, 0);
    if(p2 != pstart){//using idx
        *idx = i;
    }else{
        for(i = 0; i < MAX_ST_NUM; i ++){
            if(strncmp(g_st_flex[i].st_name, pstart, sizeof(g_st_flex[i].st_name)) == 0)
                break;
        }

        *idx = i;
    }

    if(!j){//we got at least one index
        pstart = p;

        for(i = 0, p2 = NULL; len > 0 && *p != ']'; p ++, len --){
            if(*p == '.'){
                *p = 0;
                *(p+1) = 0;
                p = p + 2;
                len -= 2;
                i = 1; 
                p2 = p;
            }
        }
        
        *p++ = 0;
        len -=1;
        
        nlen = strlen(pstart);
        *startno = *endno = pd_get_number(&pstart,&nlen, 0);
        if(p2){
            nlen = strlen(p2);
            *endno = pd_get_number(&p2,&nlen,0);
        }
    }

    *p++; //skip "."
    len --;

    *var_name = p;

    for(; len > 0 && *p != '='; p ++, len --);

    *p++ = 0;
    len --;

    nlen = strlen(p);
    *value = pd_get_number(&p, &nlen, 0);

    return;
}

static void parse_read_st_name(char *str, int *idx, uint32_t *startno, uint32_t *endno)
{
    //format:  struct_name[startno..endno]   
    
    char *p, *pstart, *p2;
    int i, len, nlen;
    
    *idx = -1;
    *startno = *endno = 0;
    
    len = strlen(str);

    pstart = str;
    for(p = str; len > 0 && p != NULL; p ++, len --){
        if(*p == '['){
            *p = 0;
            p = p + 1;
            len -= 1;
            break;
        }
    }

    p2 = pstart;
    nlen = strlen(pstart);
    i = pd_get_number(&pstart,&nlen, 0);
    if(p2 != pstart){//using idx
        *idx = i;
    }else{
        for(i = 0; i < MAX_ST_NUM; i ++){
            if(strncmp(g_st_flex[i].st_name, pstart, sizeof(g_st_flex[i].st_name)) == 0)
                break;
        }

        *idx = i;
    }
    
    if(i >= MAX_ST_NUM || len <= 0)
        return;

    
    pstart = p;

    for(i = 0, p2 = NULL; len > 0 && *p != ']'; p ++, len --){
        if(*p == '.'){
            *p = 0;
            *(p+1) = 0;
            p = p + 2;
            len -= 2;
            i = 1; 
            p2 = p;
        }
    }

    
    *p = 0;
        
    nlen = strlen(pstart);
    *startno = *endno = pd_get_number(&pstart,&nlen, 0);
    if(p2){
        nlen = strlen(p2);
        *endno = pd_get_number(&p2,&nlen,0);
    }
    
    return;
}

static void show_flex_struct(int idx)
{
    int i, j, max;

    printk("\n");
    if(idx >= MAX_ST_NUM || idx < 0){
        i = 0;
        max = MAX_ST_NUM;
    }else{
        i = idx;
        max = idx + 1;
    }
    
    for(;i < max; i ++){
        if(g_st_flex[i].st_avail == 0)
            continue;

        printk("NO [%d]: The Struct name: %s, addr 0x%x(0x%4x), size :%d\n",
             i, g_st_flex[i].st_name, g_st_flex[i].st_addr, g_st_flex[i].st_paddr, g_st_flex[i].st_size);
        
        if(idx < 0)
            continue;

        for(j = 0; j < MAX_STVAR_NUM ; j ++){
	    if(g_st_flex[i].st_var[j].var_len == 0) break;
		
            printk("\tVAR[%d]:  %32s,\tDWORD offset: %8u,\tBitStart: %8u,\tlen: %8u\n",
                            j, 
                            g_st_flex[i].st_var[j].var_name,
                            g_st_flex[i].st_var[j].var_start/BITSIZEOF_UINT32,
                            g_st_flex[i].st_var[j].var_start%BITSIZEOF_UINT32,
                            g_st_flex[i].st_var[j].var_len);
        }
    }

    return;
}

static void set_var_value(uint32_t *p, uint32_t start, uint32_t len, uint32_t value)
{
    uint32_t n, lshift, mask;
    uint32_t val;
    volatile uint32_t *addr;
    
    if ( (u32)p < KSEG0 )
        return;

    n = 0;
    if(start >= BITSIZEOF_UINT32){
        n = start >> 5;
    }

    addr = p + n;
    val = *addr;
    
    lshift = BITSIZEOF_UINT32 - (start % BITSIZEOF_UINT32) - len;
    mask = (1 << len) - 1;
    value = value & mask;
    val &= ~(mask << lshift);
    val |= value << lshift;
    *addr = val;

    return;
}

static uint32_t get_var_value(uint32_t *p, uint32_t start, uint32_t len)
{
    uint32_t n, rshift, mask;
    uint32_t value;
    volatile uint32_t *addr;
    if ( (u32)p < KSEG0 )
        return 0;

    n = 0;
    if(start >= BITSIZEOF_UINT32){
        n = start >> 5;
    }

    addr = p + n;
    rshift = start % BITSIZEOF_UINT32;
    mask = (rshift == 0)?  -1 : (1 << (BITSIZEOF_UINT32 - rshift )) - 1;
    value = *addr;
   // printk("value: 0x%x, mask: 0x%x, address; 0x%p,  rshift: %d\n", value, mask, addr, rshift);
    value = (value & mask) >> (BITSIZEOF_UINT32 - rshift - len);

    return value;
}

static void set_flex_val(int idx, uint32_t addr, uint32_t startno, uint32_t endno, char *member, uint32_t value)
{
    int i, j;
    uint32_t *p;
    
    if(idx < 0 || idx >= MAX_ST_NUM)
            return;
    
    if(g_st_flex[idx].st_avail == 0 || g_st_flex[idx].st_size == 0)
            return;

    for(i = 0; i < MAX_STVAR_NUM; i ++){
        if(g_st_flex[idx].st_var[i].var_len > 0 
            && strncmp(g_st_flex[idx].st_var[i].var_name, member, sizeof(g_st_flex[idx].st_var[i].var_name)) == 0){
            break;
        }
    }
    if(i >= MAX_STVAR_NUM){
        printk("\n struct %s don't have member: %s \n", g_st_flex[idx].st_name, member);
        return;
    }
    
    if(!addr){
        addr = g_st_flex[idx].st_addr;
    }
    
    if(!addr){
        printk("Write Address Error!\n");
        return;
    }

    for(j = startno; j <= endno; j ++){
        p = (uint32_t *)addr;
        p = p + j * (g_st_flex[idx].st_size >> 5); 
        set_var_value(p, g_st_flex[idx].st_var[i].var_start, g_st_flex[idx].st_var[i].var_len, value);
    }
    
}

static void print_flex_struct(int idx, uint32_t addr, uint32_t startno, uint32_t endno)
{
    int i,j;
    uint32_t *p, *p1;
    uint32_t value;
    uint32_t pval[32];
    
    
    if(idx < 0 || idx >= MAX_ST_NUM)
        return;

    if(g_st_flex[idx].st_avail == 0 || g_st_flex[idx].st_size == 0)
        return;

    if(!addr)
        addr = g_st_flex[idx].st_addr;
    
    if(!addr){
        printk("Struct %s:  Address Error\n", g_st_flex[idx].st_name);
        return;
    }
    
    for(i = startno; i <= endno; i ++){
        p = (uint32_t *)addr;
        p =  p + i * (g_st_flex[idx].st_size >> 5);
        p1 = p;
        if(BITSIZEOF_UINT32 * 32 >= g_st_flex[idx].st_size){//can be put in the temp buffer
            pd_memcpy((unsigned char *)pval, (unsigned char *)p, g_st_flex[idx].st_size >> 3);
            p = pval;
        }
        printk("\n%s @0x%p\n", g_st_flex[idx].st_name, p1);
        for(j = 0; j < MAX_STVAR_NUM && g_st_flex[idx].st_var[j].var_len != 0; j ++){
            value = get_var_value(p, g_st_flex[idx].st_var[j].var_start, g_st_flex[idx].st_var[j].var_len);
            printk("    %32s:\t\t0x%x\t\t(%u)\n", g_st_flex[idx].st_var[j].var_name,value,value);
        }
    }

    return;
}

static PD_INLINE int pd_get_number(char **p, int *len, int is_hex)
{
    int ret = 0;
    int n = 0;

    if ( (*p)[0] == '0' && (*p)[1] == 'x' )
    {
        is_hex = 1;
        (*p) += 2;
        (*len) -= 2;
    }

    if ( is_hex )
    {
        while ( *len && ((**p >= '0' && **p <= '9') || (**p >= 'a' && **p <= 'f') || (**p >= 'A' && **p <= 'F')) )
        {
            if ( **p >= '0' && **p <= '9' )
                n = **p - '0';
            else if ( **p >= 'a' && **p <= 'f' )
               n = **p - 'a' + 10;
            else if ( **p >= 'A' && **p <= 'F' )
                n = **p - 'A' + 10;
            ret = (ret << 4) | n;
            (*p)++;
            (*len)--;
        }
    }
    else
    {
        while ( *len && **p >= '0' && **p <= '9' )
        {
            n = **p - '0';
            ret = ret * 10 + n;
            (*p)++;
            (*len)--;
        }
    }

    return ret;
}

static PD_INLINE int pd_get_token(char **p1, char **p2, int *len, int *colon)
{
    int tlen = 0;

    while ( *len && !((**p1 >= 'A' && **p1 <= 'Z') || (**p1 >= 'a' && **p1<= 'z') || (**p1 >= '0' && **p1<= '9')) )
    {
    	if(*colon == 2){ //---special use
    	    if((**p1) == ':'){
        		*p2 = (*p1) + 1;
        		(*len) --;
        		return 1;
            }
            if((**p1) == '@'){
                *p2 = (*p1) + 1;
        		(*len) --;
                *colon = 3;
        		return 1;
            }
            
    	}
	
	    (*p1)++;
        (*len)--;
    }
    if ( !*len )
        return 0;

    if ( *colon )
    {
        *colon = 0;
        *p2 = *p1;
        while ( *len && **p2 > ' ' && **p2 != ',' )
        {
            if ( **p2 == ':' )
            {
                *colon = 1;
                break;
            }
            
            (*p2)++;
            (*len)--;
            tlen++;
        }
        **p2 = 0;
    }
    else
    {
        *p2 = *p1;
        while ( *len && **p2 > ' ' && **p2 != ',' )
        {
            (*p2)++;
            (*len)--;
            tlen++;
        }
        **p2 = 0;
    }

    return tlen;
}

static PD_INLINE void pd_ignore_space(char **p, int *len)
{
    while ( *len && (**p <= ' ' || **p == ':' || **p == '.' || **p == ',') )
    {
        (*p)++;
        (*len)--;
    }
}

static PD_INLINE int pd_strincmp(const char *p1, const char *p2, int n)
{
    int c1 = 0, c2;

    while ( n && *p1 && *p2 )
    {
        c1 = *p1 >= 'A' && *p1 <= 'Z' ? *p1 + 'a' - 'A' : *p1;
        c2 = *p2 >= 'A' && *p2 <= 'Z' ? *p2 + 'a' - 'A' : *p2;
        if ( (c1 -= c2) )
            return c1;
        p1++;
        p2++;
        n--;
    }

    return n ? *p1 - *p2 : c1;
}

static PD_INLINE int pd_stricmp(const char *p1, const char *p2)
{
    int c1, c2;

    while ( *p1 && *p2 )
    {
        c1 = *p1 >= 'A' && *p1 <= 'Z' ? *p1 + 'a' - 'A' : *p1;
        c2 = *p2 >= 'A' && *p2 <= 'Z' ? *p2 + 'a' - 'A' : *p2;
        if ( (c1 -= c2) )
            return c1;
        p1++;
        p2++;
    }

    return *p1 - *p2;
}

static PD_INLINE void *pd_memcpy(unsigned char *dest, const unsigned char *src, unsigned int count)
{
    char *d = (char *)dest;
    volatile char *s = (char *)src;

    if (count >= 32) {
        int i = 8 - (((unsigned long) d) & 0x7);

        if (i != 8)
            while (i-- && count--) {
                *d++ = *s++;
            }

        if (((((unsigned long) d) & 0x7) == 0) &&
                ((((unsigned long) s) & 0x7) == 0)) {
            while (count >= 32) {
                unsigned long long t1, t2, t3, t4;
                t1 = *(unsigned long long *) (s);
                t2 = *(unsigned long long *) (s + 8);
                t3 = *(unsigned long long *) (s + 16);
                t4 = *(unsigned long long *) (s + 24);
                *(unsigned long long *) (d) = t1;
                *(unsigned long long *) (d + 8) = t2;
                *(unsigned long long *) (d + 16) = t3;
                *(unsigned long long *) (d + 24) = t4;
                d += 32;
                s += 32;
                count -= 32;
            }
            while (count >= 8) {
                *(unsigned long long *) d =
                                            *(unsigned long long *) s;
                d += 8;
                s += 8;
                count -= 8;
            }
        }

        if (((((unsigned long) d) & 0x3) == 0) &&
                ((((unsigned long) s) & 0x3) == 0)) {
            while (count >= 4) {
                *(unsigned long *) d = *(unsigned long *) s;
                d += 4;
                s += 4;
                count -= 4;
            }
        }

        if (((((unsigned long) d) & 0x1) == 0) &&
                ((((unsigned long) s) & 0x1) == 0)) {
            while (count >= 2) {
                *(unsigned short *) d = *(unsigned short *) s;
                d += 2;
                s += 2;
                count -= 2;
            }
        }
    }

    while (count--) {
        *d++ = *s++;
    }

    return d;
}


static PD_INLINE void _proc_file_create(void)
{
    struct proc_dir_entry *res;

    for ( res = proc_root.subdir; res; res = res->next ){
        if ( res->namelen == 3
            && res->name[0] == 'e'
            && res->name[1] == 't'
            && res->name[2] == 'h' ) //  "ppa"
        {
            g_flex_proc_dir = res;
            break;
        }
    }
    
    if ( !res )
    {
        g_flex_proc_dir = proc_mkdir("eth", NULL);
        g_flex_proc_dir_flag = 1;
    }
    
    res = create_proc_read_entry("mprint",
                                  0,
                                  g_flex_proc_dir,
                                  NULL,
                                  NULL);

   if(res){
        res->write_proc = proc_write_dbg_print;
   }

   return;
}

static PD_INLINE void _proc_file_delete(void)
{
    remove_proc_entry("mprint",
                      g_flex_proc_dir);
    if(g_flex_proc_dir_flag)
        remove_proc_entry("eth", NULL);

    return;
}


static int __init ppa_procdbg_init(void)
{
    _proc_file_create();
    memset(g_st_flex, 0, sizeof(g_st_flex));
    memset(g_map_flex, 0, sizeof(g_map_flex));

    return 0;
}

static void __exit ppa_procdbg_exit(void)
{
    _proc_file_delete();
}

#ifdef INDEPENDENG_DEBUG_PROC_MODULE
module_init(ppa_procdbg_init);
module_exit(ppa_procdbg_exit);
#endif

