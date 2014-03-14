//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

#if defined(DEBUG)
#define FULL_CHECKING
#endif

//////////////////////////////////////////////////////////////////////

template <typename T> struct Node
{
	T *prev;
	T *next;
};

//////////////////////////////////////////////////////////////////////

template <typename T> struct OldList
{
}
/*
template <typename T> struct List
{
	T *head;
	T *tail;

	List();
	List &	operator = (List const &other);
	bool	IsEmpty() const;
	bool	IsInList(T *item) const;
	T *		Find(T const *item) const;	// use operator == and return the one in the list or null
	void	Clear();
	void	AddTail(T *data);
	void	AddHead(T *data);
	void	InsertAfter(T *entry, T *data);
	void	InsertBefore(T *entry, T *data);
	T *		PopTail();
	T *		PopHead();
	void	Remove(T *item);
	void	ForEach(function<void (T *)> func);
	void	ReverseForEach(function<void (T *)> func);
	size_t	CountElements() const;
};

//////////////////////////////////////////////////////////////////////

template<typename T> size_t List<T>::CountElements() const
{
	size_t count = 0;
	for(T *p = head; p != null; p = p->next)
	{
		++count;
	}
	return count;
}

//////////////////////////////////////////////////////////////////////

template<typename T> void List<T>::ForEach(function<void (T *)> func)
{
	T *n = null;
	for(T *p = head; p != null; p = n)
	{
		n = p->next;
		func(p);
	}
}

//////////////////////////////////////////////////////////////////////

template<typename T> void List<T>::ReverseForEach(function<void (T *)> func)
{
	T *n = null;
	for(T *p = tail; p != null; p = n)
	{
		n = p->prev;
		func(p);
	}
}

//////////////////////////////////////////////////////////////////////

template<typename T> T *List<T>::Find(T const *item) const
{
	for(T *h = head; h != 0; h = h->next)
	{
		if(*h == *item)
		{
			return h;
		}
	}
	return null;
}

//////////////////////////////////////////////////////////////////////

template<typename T> List<T>::List() : head(null), tail(null)
{
}

//////////////////////////////////////////////////////////////////////

template<typename T> List<T> &List<T>::operator = (List<T> const &other)
{
	head = other.head;
	tail = other.tail;
	return *this;
}

//////////////////////////////////////////////////////////////////////

template<typename T> bool List<T>::IsEmpty() const
{
	assert((head == null && tail == null) || (head != null && tail != null));
	assert((head != null && tail != null) || (head == null && tail == null));
	return head == null;
}

//////////////////////////////////////////////////////////////////////

template<typename T> bool List<T>::IsInList(T *item) const
{
	for(T *p = head; p != null; p = p->next)
	{
		if(p == item)
		{
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////

template<typename T> void List<T>::Clear()
{
	head = tail = null;
}

//////////////////////////////////////////////////////////////////////

template<typename T> void List<T>::AddTail(T *data)
{
	assert(data != null);

	data->next = null;
	data->prev = tail;

	if(tail != null)
	{
		tail->next = data;
	}
	else
	{
		head = data;
	}
	tail = data;
}

//////////////////////////////////////////////////////////////////////

template<typename T> void List<T>::AddHead(T *data)
{
	assert(data != null);

	data->next = head;
	data->prev = null;

	if(head != null)
	{
		head->prev = data;
	}
	else
	{
		tail = data;
	}
	head = data;
}

//////////////////////////////////////////////////////////////////////

template<typename T> void List<T>::InsertAfter(T *entry, T *data)
{
	assert(entry != null);
	assert(data != null);
	#if defined(FULL_CHECKING)
		assert(IsInList(entry));
	#endif

	T *n = entry->next;
	data->next = n;
	data->prev = entry;
	entry->next = data;
	if(n != null)
	{
		n->prev = data;
	}
	else
	{
		tail = data;
	}
}

//////////////////////////////////////////////////////////////////////

template<typename T> void List<T>::InsertBefore(T *entry, T *data)
{
	assert(entry != null);
	assert(data != null);
	#if defined(FULL_CHECKING)
		assert(IsInList(entry));
	#endif

	T *p = entry->prev;
	data->next = entry;
	data->prev = p;
	entry->prev = data;
	if(p != null)
	{
		p->next = data;
	}
	else
	{
		head = data;
	}
}

//////////////////////////////////////////////////////////////////////

template<typename T> T *List<T>::PopTail()
{
	T *temp = tail;
	if(tail)
	{
		tail = tail->prev;
		if(tail)
		{
			tail->next = null;
		}
		else
		{
			head = null;
		}
	}
	return temp;
}

//////////////////////////////////////////////////////////////////////

template<typename T> T *List<T>::PopHead()
{
	T *temp = head;
	if(head)
	{
		head = head->next;
		if(head)
		{
			head->prev = null;
		}
		else
		{
			tail = null;
		}
	}
	return temp;
}

//////////////////////////////////////////////////////////////////////

template<typename T> void List<T>::Remove(T *item)
{
	#if defined(FULL_CHECKING)
		assert(IsInList(item));
	#endif

	T *prev = item->prev;
	T *next = item->next;
	if(prev != null)
	{
		prev->next = next;
	}
	else
	{
		head = next;
	}
	if(next != null)
	{
		next->prev = prev;
	}
	else
	{
		tail = prev;
	}
}
*/