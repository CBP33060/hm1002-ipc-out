/*
 *	common.h
 */

#ifndef __COMMON_H__
#define __COMMON_H__

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;


extern u32 start_addr;
extern u32 jump_offset;
extern u32 spl_size;
extern int cpustate;
extern u32* heap_addr;
enum {
	ERROR = -1,
	SUCCESS = 0,
};

#define REG32(addr)	*((volatile unsigned int *)(addr))
/*
 * Define the bit field macro to avoid the bit mistake
 */
#define BIT0            (1 << 0)
#define BIT1            (1 << 1)
#define BIT2            (1 << 2)
#define BIT3            (1 << 3)
#define BIT4            (1 << 4)
#define BIT5            (1 << 5)
#define BIT6            (1 << 6)
#define BIT7            (1 << 7)
#define BIT8            (1 << 8)
#define BIT9            (1 << 9)
#define BIT10           (1 << 10)
#define BIT11           (1 << 11)
#define BIT12 	        (1 << 12)
#define BIT13 	        (1 << 13)
#define BIT14 	        (1 << 14)
#define BIT15 	        (1 << 15)
#define BIT16 	        (1 << 16)
#define BIT17 	        (1 << 17)
#define BIT18 	        (1 << 18)
#define BIT19 	        (1 << 19)
#define BIT20 	        (1 << 20)
#define BIT21 	        (1 << 21)
#define BIT22 	        (1 << 22)
#define BIT23 	        (1 << 23)
#define BIT24 	        (1 << 24)
#define BIT25 	        (1 << 25)
#define BIT26 	        (1 << 26)
#define BIT27 	        (1 << 27)
#define BIT28 	        (1 << 28)
#define BIT29 	        (1 << 29)
#define BIT30 	        (1 << 30)
#define BIT31 	        (1 << 31)

#define NULL 0
#define false 0
#define true 1

/* Get the bit field value from the data which is read from the register */
static inline void* memset(void *p, int c,int size)
{
	int i;
	for (i = 0; i < size; i++)
		((u8 *)p)[i] = c;

	return p;
}

static inline char* strcpy (char *dest,char *src)
{
	int i = 0;
	do {
		dest[i++] = *src;
	} while (*src++ != '\0');

	return dest;
}

static inline int memcmp(void * dest,const void *src, int count)
{
	char *tmp1 = (char *)dest, *tmp2 = (char *)src;
	while(count--) {
		if(*tmp1 != *tmp2)
			return -1;
		tmp1++;
		tmp2++;
	}

	return 0;
}

static inline void* memcpy(void * dest,const void *src,int count)
{
	char *tmp = (char *) dest, *s = (char *) src;

	while (count--)
		*tmp++ = *s++;

	return dest;
}

#define BLSWAP32(val)											\
    (unsigned int)((((unsigned int)(val) & (unsigned int)0x000000ffU) << 24) |				\
		  (((unsigned int)(val) & (unsigned int)0x0000ff00U) <<	8) |			\
		  (((unsigned int)(val) & (unsigned int)0x00ff0000U) >>	8) |			\
		  (((unsigned int)(val) & (unsigned int)0xff000000U) >> 24))
#endif	/*__COMMON_H__*/
