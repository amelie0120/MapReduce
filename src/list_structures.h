#ifndef __OS_LIST_H__
#define __OS_LIST_H__	1

#include <stddef.h>

typedef struct queue_list {
	struct queue_list *prev, *next;
} queue_list;

static inline void list_init(queue_list *head)
{
	head->prev = head;
	head->next = head;
}

static inline void list_add(queue_list *head, queue_list *node)
{
	node->next = head->next;
	node->prev = head;
	head->next->prev = node;
	head->next = node;
}

static inline void list_add_tail(queue_list *head, queue_list *node)
{
	node->prev = head->prev;
	node->next = head;
	head->prev->next = node;
	head->prev = node;
}

static inline void list_del(queue_list *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
	node->next = node;
	node->prev = node;
}

static inline int list_empty(queue_list *head)
{
	return (head->next == head);
}

#define list_entry(ptr, type, member) ({			\
		void *tmp = (void *)(ptr);			\
		(type *) (tmp - offsetof(type, member));	\
	})

#define list_for_each_safe(pos, tmp, head) \
	for (pos = (head)->next, tmp = pos->next; pos != (head); \
			pos = tmp, tmp = pos->next)

#endif
