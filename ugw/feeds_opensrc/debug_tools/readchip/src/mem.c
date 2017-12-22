#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LINE 0
#define TABLE 1
#define BYTE 0
#define WORD 1
#define SWITCH_BASE 0xB0106000
#define SWITCH_RANGE 0x3FF
#define DMA_BASE    0xB0103000
#define DMA_RANGE   0x7FF
#define TPE_BASE    0x0
#define TPE_RANGE   0x0
#define WDT_BASE    0xB0100900
#define WDT_RANGE   0xFF

static int c;
//static int start_address = 0;
static int end_address = 0;
static int end_input = 0;
static int range = 1;
//static int range_input=0;
static int input_flag = 0;
//static int digit_optind=0;
static int format_flag = LINE;
static int length_flag = WORD;
static int write_value = 0;
static void (*func) () = NULL;
static void *g_address;
int fd;
void
display_version ()
{
	printf
		("Lantiq memory test version 1.1\nby Wu Qi Ming\nQi-Ming.Wu@lantiq.com\n");

	return;
}

void
mem_help ()
{
	printf ("Usage:mem [options] [parameter] ...\n");
	printf ("options:\n");
	printf ("-h --help            Display help information\n");
	printf ("-d --dump            Display memory content \n");
	printf ("-s --start=ADDRESS   Set starting address\n");
	//printf("-e --end=ADDRESS     Set end address\n");
	printf ("-r --range=RANGE     Set address range\n");
	printf ("-w --write           Write to memory\n");
	printf ("-l --line            Display in consecutive lines format\n");
	printf ("-t --table           Display in table format\n");
	printf ("-b --byte            Display in byte form\n");
	printf ("-u --word            Display in word form\n");
	//printf("--switch             range=switch register\n");
	//printf("--dma                range=dma register\n");
	//printf("--tpe                range=TPE register\n");
	//printf("--wdt                range=watch dog timer register\n");
	printf ("-v --version         Display version information\n");
	return;
}

/*
* Function: neededBytes
* Purpose: get the number of bytes
* Argument: range: the range of memory need to read or write
* Return: number of bytes
*/
int
neededBytes (int range)
{
	switch (length_flag)
	 {
	 case BYTE:
		 return range;
	 case WORD:
		 return range * 4;
	 }
	return range;
}

/*
* Function: map
* Purpose: map a specific IO address to virtual address
* Argument: fname: the device memory, alwasy /dev/mem
*           startaddress: the IO address
*           len: number of bytes need to be mapped.
*/
int
map (char *fname, unsigned int startaddress, unsigned int len)
{
	FILE *f;
	unsigned page_size, mapped_size, offset_in_page, offset;
	fd = open (fname, O_RDWR | O_SYNC);
	if (fd < 0)
	 {
		 printf ("Can't open /dev/mem\n");
		 return -EIO;
	 }
	mapped_size = page_size = getpagesize ();
	offset_in_page = (unsigned) startaddress & (page_size - 1);
	//printf("offset in page: %d, len %d \n", offset_in_page, len, page_size);
	if (offset_in_page + len > page_size)
	 {
		 /* This access spans pages.
		  * Must map two pages to make it possible: */
		 mapped_size *= 2;
	 }
	/* Get the offset of IO mem which aligns to page size */
	offset = startaddress & ~(page_size - 1);
	g_address =
		mmap (NULL, mapped_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
			  offset);
	//printf("%s: 0x%x 0x%x\n", __func__, mapped_size, offset);
	//printf("%s: g_address 0x%x\n", __func__, g_address); 
	if (g_address == (void *) -1)
	 {
		 printf ("Mapping error\n");
		 close (fd);
		 return -EFAULT;
	 }
	return mapped_size;
}

/*
* Function: unmap
* Purpose: clean resource after use
* Argument: the map size, a global variable contains mapped address
*/
int
unmap (int mapped_size)
{
	munmap (g_address, mapped_size);
	close (fd);
}

#if 0
void
mem_write ()
{
	int pagesize = getpagesize ();
	int offset = (start_address) & ~(pagesize - 1);	//0xffff0000;
	int mapped_size = 0;
	int numberOfBytes = neededBytes (1);
	mapped_size = map ("/dev/mem", start_address, numberOfBytes);

	if (mapped_size < 0)
		return;
	switch (length_flag)
	 {
	 case BYTE:
		 *((unsigned char *) g_address + (start_address - offset)) =
			 (unsigned char) write_value;
		 break;
	 case WORD:
		 printf ("writing 0x%08x into address 0x%08x\n", write_value,
				 (start_address & (~3)));
		 *((int *) g_address + (((start_address - offset) & (~3)) >> 2)) =
			 (int) write_value;
		 break;
	 }
	unmap (mapped_size);
}
#endif

int mem_dump (int start_address) {
	int i = 0;
	int j = 0;
	int pos = 0;
	int value = 0;
	int finish = 0;
	int pagesize = getpagesize ();
	int offset = (start_address) & ~(pagesize - 1);	//0xfffff000;
	int mapped_size = 0;
	int retValue;

	int numberOfBytes = neededBytes (range);
	if (numberOfBytes > pagesize)
	 {
		 printf ("The range is too big,reduce to page size\n");
		 numberOfBytes = pagesize;
	 }
	mapped_size = map ("/dev/mem", start_address, numberOfBytes);
	if (mapped_size < 0)
		return;
#if 0
	if (length_flag == BYTE)
	 {
		 switch (format_flag)
		  {
		  case LINE:
			  for (i = 0; i < range; i++)
				  printf ("0x%08x:0x%02x\n", (start_address) + i,
						  *((unsigned char *) g_address +
							(start_address - offset) + i));
			  break;

		  case TABLE:
			  printf ("           ");
			  for (i = 0; i < 16; i++)
				  printf ("%02x ", i);
			  printf ("\n");
			  i = 0;
			  while (1)
			   {
				   if (finish == 1)
					   break;
				   printf ("0x%08x:", ((start_address) & (~15)) + i * 16);

				   for (j = 0; j < 16; j++)
					{
						pos = ((start_address - offset) & (~15)) + i * 16 + j;
						if ((pos - (start_address - offset)) >= range)
						 {
							 finish = 1;
							 break;
						 }
						value = *((unsigned char *) g_address + pos);
						if ((pos < (start_address - offset)))
						 {
							 printf ("   ");

						 }
						else
						 {
							 printf ("%02x ", value);
						 }
					}
				   printf ("\n");
				   i++;
			   }
			  break;
		  }
	 }
#endif
	if (length_flag == WORD)
	 {
		 switch (format_flag)
		  {
		  case LINE:

			  for (i = 0; i < range; i++)
			   {
				   pos = (((start_address - offset) & (~3)) + i * 4) >> 2;
				retValue = *((int *) g_address + pos);
#if 0
				   printf ("0x%08x:0x%08x\n",
				   ((start_address) & (~3)) + i * 4,
				   retValue);
#endif
			   }
			  break;

#if 0
		  case TABLE:
			  printf ("           ");
			  for (i = 0; i < 4; i++)
				  printf ("0x%08x ", i * 4);
			  printf ("\n");
			  i = 0;
			  finish = 0;
			  while (1)
			   {
				   if (finish == 1)
					   break;
				   printf ("0x%08x:", ((start_address) & (~15)) + i * 16);

				   for (j = 0; j < 4; j++)
					{
						pos = (((start_address - offset) & (~15)) + i * 16 + j * 4) >> 2;	//old 3
						//printf("pos:%d %d\n", pos,(start_address - offset)>>2);
						if (((pos << 2) - (start_address - offset)) >=
							range * 4)
						 {
							 //printf("%d,%d,%d\n",pos,start_address,(pos-start_address));
							 finish = 1;
							 break;
						 }
						value = *((int *) g_address + pos);
						if ((pos < ((start_address - offset) >> 2)))
						 {
							 printf ("           ");

						 }
						else
						 {
							 printf ("0x%08x ", value);
						 }
					}
				   printf ("\n");
				   i++;
			   }
			  break;
#endif
		  }
	 }
	unmap (mapped_size);
	return retValue;

}

#if 0
int
main (int argc, char **argv)
{
	while (1)
	 {
		 int option_index = 0;
		 static struct option long_options[] = {
			 {"help", 0, 0, 0},
			 {"dump", 0, 0, 0},
			 {"start", 1, 0, 0},
			 {"end", 1, 0, 0},
			 {"range", 1, 0, 0},
			 {"line", 0, 0, 0},
			 {"table", 0, 0, 0},
			 {"write", 1, 0, 0},
			 {"switch", 0, 0, 0},
			 {"dma", 0, 0, 0},
			 {"tpe", 0, 0, 0},
			 {"wdt", 0, 0, 0},
			 {"byte", 0, 0, 0},
			 {"word", 0, 0, 0},
			 {"version", 0, 0, 0}

		 };
		 c = getopt_long (argc, argv, "bs:e:r:dtlhw:uv",
						  long_options, &option_index);

		 if (c == -1)
		  {
			  if (input_flag == 0)
			   {
				   printf ("mem:please specify parameters\n");
				   func = &mem_help;
			   }
			  if (func)
				  (*func) ();
			  else
			   {
				 ERROR:mem_help ();
			   }
			  break;
		  }
		 input_flag = 1;
		 switch (c)
		  {
		  case 0:
			  if (option_index == 0)
			   {
				   func = &mem_help;
				   break;
			   }
			  if (option_index == 1)
			   {
				   func = &mem_dump;
				   break;
			   }
			  if (option_index == 2)
			   {
				   if (!optarg)
					   goto ERROR;
				   start_address = strtoul (optarg, NULL, 0);
				   break;
			   }
			  if (option_index == 3)
			   {
				   if (!optarg)
					   goto ERROR;
				   end_address = strtoul (optarg, NULL, 0);
				   break;
			   }
			  if (option_index == 4)
			   {
				   if (!optarg)
					   goto ERROR;
				   range = strtoul (optarg, NULL, 0);
				   break;
			   }
			  if (option_index == 5)
			   {
				   format_flag = LINE;
				   break;
			   }
			  if (option_index == 6)
			   {
				   format_flag = TABLE;
				   break;
			   }
			  if (option_index == 7)
			   {
				   if (!optarg)
					   goto ERROR;
				   write_value = strtoul (optarg, NULL, 0);
				   func = &mem_write;
				   break;
			   }
			  if (option_index == 8)
			   {
				   start_address = SWITCH_BASE;
				   range = SWITCH_RANGE;
				   break;
			   }
			  if (option_index == 9)
			   {
				   start_address = DMA_BASE;
				   range = DMA_RANGE;
				   break;
			   }
			  if (option_index == 10)
			   {
				   start_address = TPE_BASE;
				   range = TPE_RANGE;
				   break;
			   }
			  if (option_index == 11)
			   {
				   start_address = WDT_BASE;
				   range = WDT_RANGE;
				   break;
			   }
			  if (option_index == 12)
			   {
				   length_flag = BYTE;
				   break;
			   }
			  if (option_index == 13)
			   {
				   length_flag = WORD;
				   break;
			   }
			  if (option_index == 14)
			   {
				   func = &display_version;
				   break;

			   }
		  case 'h':
			  func = &mem_help;
			  break;
		  case 's':
			  if (!optarg)
				  goto ERROR;
			  start_address = strtoul (optarg, NULL, 0);
			  break;
		  case 'e':
			  if (!optarg)
				  goto ERROR;
			  end_input = 1;
			  end_address = strtoul (optarg, NULL, 0);
			  break;
		  case 'r':
			  if (!optarg)
				  goto ERROR;
			  range = strtoul (optarg, NULL, 0);
			  break;
		  case 'd':
			  func = &mem_dump;
			  break;
		  case 'b':
			  length_flag = BYTE;
			  break;
		  case 'u':
			  length_flag = WORD;
			  break;
		  case 'l':
			  format_flag = LINE;
			  break;
		  case 't':
			  format_flag = TABLE;
			  break;
		  case 'w':
			  if (!optarg)
				  goto ERROR;
			  write_value = strtoul (optarg, NULL, 0);
			  func = &mem_write;
			  break;
		  case 'v':
			  func = &display_version;
			  break;
		  }
	 }
	return 0;
}
#endif
