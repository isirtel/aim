/*-
 * Copyright (c) 2011 Felix Fietkau <nbd@openwrt.org>
 * Copyright (c) 2010 Isilon Systems, Inc.
 * Copyright (c) 2010 iX Systems, Inc.
 * Copyright (c) 2010 Panasas, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _LINUX_OPLIST_H_
#define _LINUX_OPLIST_H_

#include <stddef.h>
#include <stdbool.h>

#define	prefetch(x)

#ifndef container_of
#define container_of(ptr, type, member)					\
	({								\
		const __typeof__(((type *) NULL)->member) *__mptr = (ptr);	\
		(type *) ((char *) __mptr - offsetof(type, member));	\
	})
#endif

struct oplist_head {
	struct oplist_head *next;
	struct oplist_head *prev;
};

#define OPLIST_HEAD_INIT(name) { &(name), &(name) }
#undef OPLIST_HEAD
#define OPLIST_HEAD(name)	struct oplist_head name = OPLIST_HEAD_INIT(name)

static inline void
OPINIT_LIST_HEAD(struct oplist_head *list)
{
	list->next = list->prev = list;
}

static inline bool
oplist_empty(const struct oplist_head *head)
{
	return (head->next == head);
}

static inline bool
oplist_is_first(const struct oplist_head *list,
	      const struct oplist_head *head)
{
	return list->prev == head;
}

static inline bool
oplist_is_last(const struct oplist_head *list,
	     const struct oplist_head *head)
{
	return list->next == head;
}

static inline void
_oplist_del(struct oplist_head *entry)
{
	entry->next->prev = entry->prev;
	entry->prev->next = entry->next;
}

static inline void
oplist_del(struct oplist_head *entry)
{
	_oplist_del(entry);
	entry->next = entry->prev = NULL;
}

static inline void
_oplist_add(struct oplist_head *_new, struct oplist_head *prev,
    struct oplist_head *next)
{

	next->prev = _new;
	_new->next = next;
	_new->prev = prev;
	prev->next = _new;
}

static inline void
oplist_del_init(struct oplist_head *entry)
{
	_oplist_del(entry);
	OPINIT_LIST_HEAD(entry);
}

#define	oplist_entry(ptr, type, field)	container_of(ptr, type, field)
#define	oplist_first_entry(ptr, type, field)	oplist_entry((ptr)->next, type, field)
#define	oplist_last_entry(ptr, type, field)	oplist_entry((ptr)->prev, type, field)

#define	oplist_for_each(p, head)						\
	for (p = (head)->next; p != (head); p = p->next)

#define	oplist_for_each_safe(p, n, head)					\
	for (p = (head)->next, n = p->next; p != (head); p = n, n = p->next)

#define oplist_for_each_entry(p, h, field)				\
	for (p = oplist_first_entry(h, __typeof__(*p), field); &p->field != (h); \
	    p = oplist_entry(p->field.next, __typeof__(*p), field))

#define oplist_for_each_entry_safe(p, n, h, field)			\
	for (p = oplist_first_entry(h, __typeof__(*p), field),		\
	    n = oplist_entry(p->field.next, __typeof__(*p), field); &p->field != (h);\
	    p = n, n = oplist_entry(n->field.next, __typeof__(*n), field))

#define	oplist_for_each_entry_reverse(p, h, field)			\
	for (p = oplist_last_entry(h, __typeof__(*p), field); &p->field != (h); \
	    p = oplist_entry(p->field.prev, __typeof__(*p), field))

#define	oplist_for_each_prev(p, h) for (p = (h)->prev; p != (h); p = p->prev)
#define	oplist_for_each_prev_safe(p, n, h) for (p = (h)->prev, n = p->prev; p != (h); p = n, n = p->prev)

static inline void
oplist_add(struct oplist_head *_new, struct oplist_head *head)
{
	_oplist_add(_new, head, head->next);
}

static inline void
oplist_add_tail(struct oplist_head *_new, struct oplist_head *head)
{
	_oplist_add(_new, head->prev, head);
}

static inline void
oplist_move(struct oplist_head *list, struct oplist_head *head)
{
	_oplist_del(list);
	oplist_add(list, head);
}

static inline void
oplist_move_tail(struct oplist_head *entry, struct oplist_head *head)
{
	_oplist_del(entry);
	oplist_add_tail(entry, head);
}

static inline void
_oplist_splice(const struct oplist_head *list, struct oplist_head *prev,
    struct oplist_head *next)
{
	struct oplist_head *first;
	struct oplist_head *last;

	if (oplist_empty(list))
		return;

	first = list->next;
	last = list->prev;
	first->prev = prev;
	prev->next = first;
	last->next = next;
	next->prev = last;
}

static inline void
oplist_splice(const struct oplist_head *list, struct oplist_head *head)
{
	_oplist_splice(list, head, head->next);
}

static inline void
oplist_splice_tail(struct oplist_head *list, struct oplist_head *head)
{
	_oplist_splice(list, head->prev, head);
}

static inline void
oplist_splice_init(struct oplist_head *list, struct oplist_head *head)
{
	_oplist_splice(list, head, head->next);
	OPINIT_LIST_HEAD(list);
}

static inline void
oplist_splice_tail_init(struct oplist_head *list, struct oplist_head *head)
{
	_oplist_splice(list, head->prev, head);
	OPINIT_LIST_HEAD(list);
}

#endif /* _LINUX_LIST_H_ */

