//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////
// Deal with MSVC internal compiler error, this can be removed when a fix is available

#pragma push_macro("VC_WORKAROUND")
#undef VC_WORKAROUND
#if defined(_MSC_VER)
#define VC_WORKAROUND NODE
#else
#define VC_WORKAROUND (list_node<T> T::*)nullptr != NODE
#endif

//////////////////////////////////////////////////////////////////////

namespace chs
{

    //////////////////////////////////////////////////////////////////////
    // base list node class, 2 pointers

    template <typename T> struct list_node_base
    {
        T *next;
        T *prev;
    };

    //////////////////////////////////////////////////////////////////////
    // base node is wrapped so we can get the offset to it

    template <typename T> struct list_node
    {
        list_node_base<T> node;
    };

    //////////////////////////////////////////////////////////////////////
    // template base

    template <typename T, list_node<T> T::*NODE, bool is_member> class list_base
    {
    };

    //////////////////////////////////////////////////////////////////////
    // specialization for instances using list_node as member field

    template <typename T, list_node<T> T::*NODE> class list_base<T, NODE, true> : protected list_node<T>
    {
    protected:
        static size_t offset()
        {
            list_node<T> *b = &(((T *)0)->*NODE);
            return size_t(&b->node);
        }

        typedef list_base<T, NODE, true> list_type;
    };

    //////////////////////////////////////////////////////////////////////
    // specialization for instances deriving from list_node

    template <typename T, list_node<T> T::*NODE> class list_base<T, NODE, false> : protected list_node<T>
    {
        static_assert(!std::is_polymorphic<T>::value, "polymorphic! use the member-node version");
    protected:
        static size_t offset()
        {
            list_node_base<T> T::*n = static_cast<list_node_base<T> T::*>(&T::node);
            return (size_t)(&(((T *)0)->*n));
        }
        typedef list_base<T, NODE, false> list_type;
    };

    //////////////////////////////////////////////////////////////////////

    template <typename T, list_node<T> T::*NODE = nullptr> class linked_list : protected list_base<T, NODE, VC_WORKAROUND>
    {
    public:

        //////////////////////////////////////////////////////////////////////

        using list_base<T, NODE, VC_WORKAROUND>::offset;
        using list_base<T, NODE, VC_WORKAROUND>::node;

        //////////////////////////////////////////////////////////////////////

        typedef linked_list<T, NODE> list_t;

        typedef T *          ptr;
        typedef T *          pointer;
        typedef T const *    const_ptr;
        typedef T const *    const_pointer;
        typedef T &          ref;
        typedef T &          reference;
        typedef T const &    const_ref;
        typedef T const &    const_reference;

        //////////////////////////////////////////////////////////////////////

#if !defined(_CHS_LINKED_LIST_DONT_DEFINE_STL_ITERATORS_)

        typedef std::iterator<std::bidirectional_iterator_tag, T> iterator_base;

        //////////////////////////////////////////////////////////////////////

        struct const_iterator : iterator_base
        {
            const_iterator(const_ptr t) : p(t) {}
            const_iterator(const_iterator const &o) : p(o.p) {}
            const_iterator const &operator=(const_iterator const &o) { p = o.p; return *this; }
            const_iterator &operator++() { p = get_node(p).next; return *this; }
            const_iterator &operator--() { p = get_node(p).prev; return *this; }
            bool operator==(const_iterator const &o) { return p == o.p; }
            bool operator!=(const_iterator const &o) { return p != o.p; }
            const_ref operator *() { return *p; }
            const_ptr operator->() { return  p; }
            operator ptr() { return p; }
        private:
            const_ptr p;
        };

        //////////////////////////////////////////////////////////////////////

        struct iterator : iterator_base
        {
            iterator(ptr t) : p(t) {}
            iterator(const_ptr *t) : p(t) {}
            iterator(iterator const &o) : p(o.p) {}
            iterator const &operator=(const_iterator const &o) { p = o.p; return *this; }
            iterator const &operator=(iterator const &o) { p = o.p; return *this; }
            iterator &operator++() { p = get_node(p).next; return *this; }
            iterator &operator--() { p = get_node(p).prev; return *this; }
            bool operator==(iterator const &o) { return p == o.p; }
            bool operator!=(iterator const &o) { return p != o.p; }
            ref operator *() { return *p; }
            ptr operator->() { return  p; }
            operator ptr() { return p; }
        private:
            ptr p;
        };

        //////////////////////////////////////////////////////////////////////

        struct const_reverse_iterator : iterator_base
        {
            const_reverse_iterator(const_ptr t) : p(t) {}
            const_reverse_iterator(const_reverse_iterator const &o) : p(o.p) {}
            const_reverse_iterator const &operator=(const_reverse_iterator const &o) { p = o.p; return o; }
            const_reverse_iterator &operator++() { p = get_node(p).prev; return *this; }
            const_reverse_iterator &operator--() { p = get_node(p).next; return *this; }
            bool operator==(const_reverse_iterator const &o) { return p == o.p; }
            bool operator!=(const_reverse_iterator const &o) { return p != o.p; }
            const_ref operator *() { return *p; }
            const_ptr operator->() { return  p; }
            operator ptr() { return p; }
        private:
            const_ptr p;
        };

        //////////////////////////////////////////////////////////////////////

        struct reverse_iterator : iterator_base
        {
            reverse_iterator(ptr t) : p(t) {}
            reverse_iterator(const_ptr t) : p(t) {}
            reverse_iterator(reverse_iterator const &o) : p(o.p) {}
            reverse_iterator const &operator=(const_reverse_iterator const &o) { p = o.p; return *this; }
            reverse_iterator const &operator=(reverse_iterator const &o) { p = o.p; return *this; }
            reverse_iterator &operator++() { p = get_node(p).prev; return *this; }
            reverse_iterator &operator--() { p = get_node(p).next; return *this; }
            bool operator==(reverse_iterator const &o) { return p == o.p; }
            bool operator!=(reverse_iterator const &o) { return p != o.p; }
            ref operator *() { return *p; }
            ptr operator->() { return  p; }
            operator ptr() { return p; }
        private:
            ptr p;
        };

        //////////////////////////////////////////////////////////////////////

        iterator                  begin()         { return iterator(node.next); }
        const_iterator            begin() const   { return const_iterator(node.next); }

        reverse_iterator          rbegin()        { return reverse_iterator(node.prev); }
        const_reverse_iterator    rbegin() const  { return const_reverse_iterator(node.prev); }

        iterator                  end()           { return iterator(root()); }
        const_iterator            end() const     { return const_iterator(const_root()); }

        reverse_iterator          rend()          { return reverse_iterator(root()); }
        const_reverse_iterator    rend() const    { return const_reverse_iterator(const_root()); }

        const_iterator            cbegin() const  { return const_iterator(node.next); }
        const_iterator            cend() const    { return const_iterator(const_root()); }

        const_reverse_iterator    crbegin() const { return const_reverse_iterator(node.prev); }
        const_reverse_iterator    crend() const   { return const_reverse_iterator(const_root()); }

#endif //!defined(_CHS_LINKED_LIST_DONT_DEFINE_STL_ITERATORS_)

        //////////////////////////////////////////////////////////////////////

        typedef list_node_base<T>        node_t;
        typedef node_t *                 node_ptr;
        typedef node_t &                 node_ref;
        typedef list_node_base<T> const  const_node_t;
        typedef const_node_t *           const_node_ptr;
        typedef const_node_t &           const_node_ref;

    private:

        //////////////////////////////////////////////////////////////////////

        static list_t &transfer(list_t &a, list_t &b)
        {
            if(!a.empty())
            {
                T *ot = a.tail();
                T *oh = a.head();
                T *rt = b.root();
                get_node(ot).next = rt;
                get_node(oh).prev = rt;
                get_node(rt).prev = ot;
                get_node(rt).next = oh;
                a.clear();
            }
            else
            {
                b.clear();
            }
            return b;
        }

        //////////////////////////////////////////////////////////////////////

        static node_ref get_node(ptr obj)
        {
            return *reinterpret_cast<node_ptr>(reinterpret_cast<char *>(obj) + offset());
        }

        //////////////////////////////////////////////////////////////////////

        static const_node_ref get_node(const_ptr obj)
        {
            return *reinterpret_cast<const_node_ptr>(reinterpret_cast<char const *>(obj) + offset());
        }

        //////////////////////////////////////////////////////////////////////

        ptr root()
        {
            return reinterpret_cast<ptr>(reinterpret_cast<char *>(&node) - offset());
        }

        //////////////////////////////////////////////////////////////////////

        const_ptr const_root() const
        {
            return reinterpret_cast<const_ptr>(reinterpret_cast<char const *>(&node) - offset());
        }

    public:

        //////////////////////////////////////////////////////////////////////

        void clear()
        {
            node.next = root();
            node.prev = root();
        }

        //////////////////////////////////////////////////////////////////////

        void insert_before(ptr obj_before, ptr obj)
        {
            ptr &p = get_node(obj_before).prev;
            node_ref n = get_node(obj);
            get_node(p).next = obj;
            n.prev = p;
            p = obj;
            n.next = obj_before;
        }

        //////////////////////////////////////////////////////////////////////

        void insert_after(ptr obj_after, ptr obj)
        {
            ptr &n = get_node(obj_after).next;
            node_ref p = get_node(obj);
            get_node(n).prev = obj;
            p.next = n;
            n = obj;
            p.prev = obj_after;
        }

        //////////////////////////////////////////////////////////////////////

        ptr remove(ptr obj)
        {
            ptr p = prev(obj);
            ptr n = next(obj);
            get_node(p).next = n;
            get_node(n).prev = p;
            return obj;
        }

        //////////////////////////////////////////////////////////////////////

        linked_list()
        {
            clear();
        }

        //////////////////////////////////////////////////////////////////////

        linked_list(list_t &other)
        {
            transfer(other, *this);
        }

        //////////////////////////////////////////////////////////////////////

        list_t &operator = (list_t & o)
        {
            return transfer(o, *this);
        }

        //////////////////////////////////////////////////////////////////////

        list_t &operator = (list_t && o)
        {
            return transfer(o, *this);
        }

        //////////////////////////////////////////////////////////////////////

        bool empty() const
        {
            return node.next == const_root();
        }

        //////////////////////////////////////////////////////////////////////

        size_t size() const
        {
            size_t count = 0;
            for(auto const &i : *this)
            {
                ++count;
            }
            return count;
        }

        //////////////////////////////////////////////////////////////////////

        ptr       head()                         { return node.next; }
        ptr       tail()                         { return node.prev; }

        const_ptr c_head() const                 { return node.next; }
        const_ptr c_tail() const                 { return node.prev; }

        ptr       next(ptr obj)                  { return get_node(obj).next; }
        ptr       prev(ptr obj)                  { return get_node(obj).prev; }

        const_ptr c_next(const_ptr obj) const    { return get_node(obj).next; }
        const_ptr c_prev(const_ptr obj) const    { return get_node(obj).prev; }

        ptr       done()                         { return root(); }
        const_ptr c_done() const                 { return const_root(); }

        //////////////////////////////////////////////////////////////////////

        ptr       remove(ref obj)                 { return remove(&obj); }

        void      push_back(ref obj)              { insert_before(*root(), obj); }
        void      push_back(ptr obj)              { insert_before(root(), obj); }

        void      push_front(ref obj)             { insert_after(*root(), obj); }
        void      push_front(ptr obj)             { insert_after(root(), obj); }

        ptr       pop_front()                     { return empty() ? nullptr : remove(head()); }
        ptr       pop_back()                      { return empty() ? nullptr : remove(tail()); }

        void      insert_before(ref bef, ref obj) { insert_before(&bef, &obj); }
        void      insert_after(ref aft, ref obj)  { insert_after(&aft, &obj); }

        //////////////////////////////////////////////////////////////////////

        void append(list_t &other_list)
        {
            if(!other_list.empty())
            {
                ptr mt = tail();
                ptr oh = other_list.head();
                get_node(mt).next = oh;
                get_node(oh).prev = mt;
                get_node(other_list.tail()).next = root();
                node.prev = other_list.tail();
                other_list.clear();
            }
        }

        //////////////////////////////////////////////////////////////////////

        void prepend(list_t &other_list)
        {
            if(!other_list.empty())
            {
                ptr mh = head();
                ptr ot = other_list.tail();
                get_node(mh).prev = ot;
                get_node(ot).next = mh;
                node.next = other_list.head();
                other_list.clear();
            }
        }

        //////////////////////////////////////////////////////////////////////

        template<typename N> void copy_into(N &n)
        {
            static_assert(!std::is_same<N, list_t>::value, "can't copy to the same type!");
            for(auto &p : *this)
            {
                n.push_back(p);
            }
        }

        //////////////////////////////////////////////////////////////////////

        list_t split(ref obj)
        {
            list_t new_list;
            T *new_root = new_list.root();
            T *old_tail = tail();
            T *prev_obj = prev(&obj);
            get_node(prev_obj).next = root();
            get_node(root()).prev = prev_obj;
            new_list.get_node(new_root).next = &obj;
            new_list.get_node(new_root).prev = old_tail;
            get_node(old_tail).next = new_root;
            return new_list;
        }

        //////////////////////////////////////////////////////////////////////

        static list_t merge(list_t &left, list_t &right)
        {
            list_t new_list;
            while(!left.empty() && !right.empty())
            {
                ptr l = left.head();
                ptr r = right.head();
                ptr w;
                if(*r < *l)
                {
                    w = right.pop_front();
                }
                else
                {
                    w = left.pop_front();
                }
                new_list.push_back(w);
            }
            if(!left.empty())
            {
                new_list.append(left);
            }
            else if(!right.empty())
            {
                new_list.append(right);
            }
            return new_list;
        }

        //////////////////////////////////////////////////////////////////////

        static list_t sort(list_t list)
        {
            ptr h = list.head();
            ptr m = h;
            int s = 0;
            while(h != list.root() && list.next(h) != list.root())
            {
                m = list.next(m);
                h = list.next(m);
                ++s;
            }
            if(s == 0)
            {
                return list;
            }
            list_t right = list.split(*m);
            list = sort(list);
            right = sort(right);
            return merge(list, right);
        }

        //////////////////////////////////////////////////////////////////////

        void sort()
        {
            *this = sort(*this);
        }

        //////////////////////////////////////////////////////////////////////

        void for_each(std::function<void(T &)> func)
        {
            for(auto i = begin(), _end = end(), n = i; ++n, i != _end; i = n)
            {
                func(*i);
            }
        }

        //////////////////////////////////////////////////////////////////////

        list_t select(std::function<bool(T &)> func)
        {
            list_t t;
            for(auto i = begin(), _end = end(), n = i; ++n, i != _end; i = n)
            {
                if(func(*i))
                {
                    t.push_back(remove(i));
                }
            }
            return t;
        }

    };

    //////////////////////////////////////////////////////////////////////

    template <typename T> std::string to_string(T const &l, char const *separator = ",")
    {
        std::string s;
        char const *sep = "";
        for(auto const &p: l)
        {
            s += sep;
            s += p.to_string();
            sep = separator;
        }
        return s;
    }

    //////////////////////////////////////////////////////////////////////

    template<class T> struct in_reverse
    {
        T &l;
        in_reverse(T &list) : l(list) {}

        auto begin() ->         decltype(l. rbegin())   { return l.rbegin(); } 
        auto begin() const ->   decltype(l.crbegin())   { return l.crbegin(); } 
        auto end() ->           decltype(l. rend())     { return l.rend(); } 
        auto end() const ->     decltype(l.crend())     { return l.crend(); } 
    };

    template<class T> in_reverse<T> reverse(T &l)
    {
        return in_reverse<T>(l);
    }

    //////////////////////////////////////////////////////////////////////

    template<class L, class T> auto find_first_of(L &l, T &t) -> decltype(l.begin())
    {
        return std::find(l.begin(), l.end(), t);
    }

    //////////////////////////////////////////////////////////////////////

    template<class L, class T> auto find_last_of(L &l, T &t) -> decltype(l.rbegin())
    {
        return std::find(l.rbegin(), l.rend(), t);
    }

    //////////////////////////////////////////////////////////////////////

    template<class L> void remove_and_delete_all(L &l)
    {
        while(!l.empty())
        {
            auto i = l.pop_back();
            delete &(*i);
        }
    }

    //////////////////////////////////////////////////////////////////////

    template<class L> void delete_if(L &l, std::function<bool(decltype(*(l.begin())))> func)
    {
        for(auto i = l.begin(), end = l.end(), n = i; ++n, i != end; i = n)
        {
            if(func(*i))
            {
                l.remove(i);
                delete &(*i);
            }
        }
    }

    //////////////////////////////////////////////////////////////////////

} // chs

#pragma warning(push)
#pragma warning(disable: 4602)        // disable spurious warning
#pragma pop_macro("VC_WORKAROUND")
#pragma warning(pop)

