#ifndef __RTL8328_TYPES_H__
#define __RTL8328_TYPES_H__


#ifndef uint64
#define uint64					unsigned long long
#endif
#ifndef int64
#define int64					long long
#endif
#ifndef uint32
#define uint32					unsigned int
#endif
#ifndef int32
#define int32					int
#endif
#ifndef uint16
#define uint16					unsigned short	
#endif
#ifndef int16
#define int16					short
#endif
#ifndef uint8
#define uint8					unsigned char
#endif
#ifndef int8
#define int8					char
#endif
#ifndef ipaddr_t
#define ipaddr_t          	uint32
#endif

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN		6
#endif

typedef struct ether_macaddr_s {
	uint8 octet[ETHER_ADDR_LEN];
} ether_macaddr_t;

#define swapl32(x)\
        ((((x) & 0xff000000U) >> 24) | \
         (((x) & 0x00ff0000U) >>  8) | \
         (((x) & 0x0000ff00U) <<  8) | \
         (((x) & 0x000000ffU) << 24))
#define swaps16(x)        \
        ((((x) & 0xff00) >> 8) | \
         (((x) & 0x00ff) << 8))  

#define UNCACHE_MASK		0x20000000
#define UNCACHE(addr)		((UNCACHE_MASK)|(uint32)(addr))
#define CACHED(addr)			((uint32)(addr) & ~(UNCACHE_MASK))



#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAILED
#define FAILED -1
#endif

#ifndef ENABLE
#define ENABLE 1
#endif

#ifndef DISABLE
#define DISABLE 0
#endif

#ifdef __KERNEL__
#define rtlglue_printf printk
#else
#define rtlglue_printf printf
#endif

#endif /* __RTL8328_TYPES_H__ */

