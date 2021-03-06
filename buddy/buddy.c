/**
 * Buddy Allocator
 *
 * For the list library usage, see http://www.mcs.anl.gov/~kazutomo/list/
 */

/**************************************************************************
 * Conditional Compilation Options
 **************************************************************************/
#define USE_DEBUG 0

/**************************************************************************
 * Included Files
 **************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include "buddy.h"
#include "list.h"

/**************************************************************************
 * Public Definitions
 **************************************************************************/
#define MIN_ORDER 12
#define MAX_ORDER 20

#define PAGE_SIZE (1<<MIN_ORDER)
/* page index to address */
#define PAGE_TO_ADDR(page_idx) (void *)((page_idx*PAGE_SIZE) + g_memory)

/* address to page index */
#define ADDR_TO_PAGE(addr) ((unsigned long)((void *)addr - (void *)g_memory) / PAGE_SIZE)

/* find buddy address */
#define BUDDY_ADDR(addr, o) (void *)((((unsigned long)addr - (unsigned long)g_memory) ^ (1<<o)) \
									 + (unsigned long)g_memory)

#if USE_DEBUG == 1
#  define PDEBUG(fmt, ...) \
	fprintf(stderr, "%s(), %s:%d: " fmt,			\
		__func__, __FILE__, __LINE__, ##__VA_ARGS__)
#  define IFDEBUG(x) x
#else
#  define PDEBUG(fmt, ...)
#  define IFDEBUG(x)
#endif

/**************************************************************************
 * Public Types
 **************************************************************************/
typedef struct {
	struct list_head list;
	/* TODO: DECLARE NECESSARY MEMBER VARIABLES */
	int index;
	int order;
	char * addr;
} page_t;

/**************************************************************************
 * Global Variables
 **************************************************************************/
/* free lists*/
struct list_head free_area[MAX_ORDER+1];

/* memory area */
char g_memory[1<<MAX_ORDER];

/* page structures */
page_t g_pages[(1<<MAX_ORDER)/PAGE_SIZE];

/**************************************************************************
 * Public Function Prototypes
 **************************************************************************/

/**************************************************************************
 * Local Functions
 **************************************************************************/

void split(page_t* page, int order, int requested)
{
	if (order == requested)
	{
		return;
	}
	page_t* buddy = &g_pages[ADDR_TO_PAGE(BUDDY_ADDR(page->addr, (order-1)))];
	buddy->order = order-1;
	list_add(&(buddy->list), &free_area[order-1]);
	split(page, order-1, requested);
}

int find_order(int size)
{
	for (int i = MIN_ORDER; i <= MAX_ORDER; i++)
	{
		if ((1<<i) >= size)
		{
			return i;
		}
	}
	return -1;
}
 
 
/**
 * Initialize the buddy system
 */
void buddy_init()
{
	int i;
	int n_pages = (1<<MAX_ORDER) / PAGE_SIZE;
	for (i = 0; i < n_pages; i++) {
		/* TODO: INITIALIZE PAGE STRUCTURES */
		g_pages[i].index = i;
		g_pages[i].order = -1;
		g_pages[i].addr = PAGE_TO_ADDR(i);
	}
	
	g_pages[0].order = MAX_ORDER;

	/* initialize freelist */
	for (i = MIN_ORDER; i <= MAX_ORDER; i++) {
		INIT_LIST_HEAD(&free_area[i]);
	}

	/* add the entire memory as a freeblock */
	list_add(&g_pages[0].list, &free_area[MAX_ORDER]);
}

/**
 * Allocate a memory block.
 *
 * On a memory request, the allocator returns the head of a free-list of the
 * matching size (i.e., smallest block that satisfies the request). If the
 * free-list of the matching block size is empty, then a larger block size will
 * be selected. The selected (large) block is then splitted into two smaller
 * blocks. Among the two blocks, left block will be used for allocation or be
 * further splitted while the right block will be added to the appropriate
 * free-list.
 *
 * @param size size in bytes
 * @return memory block address
 */
void *buddy_alloc(int size)
{
	/* TODO: IMPLEMENT THIS FUNCTION */
	int req_order = find_order(size);
	 
	 if (req_order == -1)
	 {
		 return NULL;
	 }
	
	for (int i = req_order; i <= MAX_ORDER; i++)
	{
		//if empty block is found, break to right size
		if (!list_empty(&free_area[i]))
		{
			page_t* page = list_entry(free_area[i].next, page_t, list);
			list_del_init(&(page->list));
			split(page, i, req_order);
			page->order = req_order;
			return page->addr;
		}
	}
	return NULL;
}

/**
 * Free an allocated memory block.
 *
 * Whenever a block is freed, the allocator checks its buddy. If the buddy is
 * free as well, then the two buddies are combined to form a bigger block. This
 * process continues until one of the buddies is not free.
 *
 * @param addr memory block address to be freed
 */
void buddy_free(void *addr)
{
	/* TODO: IMPLEMENT THIS FUNCTION */
	page_t* page = &g_pages[ADDR_TO_PAGE(addr)];
	int i;
	
	for ( i = page->order; i < MAX_ORDER; i++)
	{
		page_t* page_buddy = &g_pages[ADDR_TO_PAGE(BUDDY_ADDR(page->addr, i))];
		bool freed = false;
		struct list_head *list_position;
		
		list_for_each(list_position, &free_area[i])
		{
			if (list_entry(list_position, page_t, list) == page_buddy)
			{
				freed = true;
			}
		}
		
		if (!freed)
		{
			break;
		}
		
		list_del_init(&page_buddy->list);
		
		if(page_buddy<page)
		{
			page = page_buddy;
		}
		
	}
	page->order = i;
	list_add(&page->list, &free_area[i]);
}

/**
 * Print the buddy system status---order oriented
 *
 * print free pages in each order.
 */
void buddy_dump()
{
	int o;
	for (o = MIN_ORDER; o <= MAX_ORDER; o++) {
		struct list_head *pos;
		int cnt = 0;
		list_for_each(pos, &free_area[o]) {
			cnt++;
		}
		printf("%d:%dK ", cnt, (1<<o)/1024);
	}
	printf("\n");
}









































//
