/*
 *
 * Copyright (c) 2012, xhawk18
 * at gmail.com
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose that fits the following requirement
 * is hereby granted without fee, provided that the above copyright notice
 * appear in all copies and that both that copyright notice and this permission
 * notice appear in supporting documentation.  X-Hawk representations about the
 * suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 *
 * Special requirement 1, to use this software in commercial software, 
 * you should claim the usage of this software in striking poistion on 
 * the web page about your software.
 *
 * Special requirement 2, the license of this software is truely free for ever
 * and must not be converted to a GPL like license.
 */

/*
 * ll::lv is a container that has O(log(n)) erase/insert time and random access time,
 * It can replace list/vector/deque in some case.
 */
#ifndef INC_LL_LV_HPP_
#define INC_LL_LV_HPP_

#include <limits>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <vector>

#include "ll_rb_tree.hpp"

namespace ll
{



template< class T >
struct lv_key_of_value
{
    T const& operator()(T const &v) const
    {
        return v;
    }
};

template< class T >
struct lv_key_compare
{
    bool operator()(T const &l, T const &r) const
    {
        return l < r;
    }
};

template<class _Val>
struct lv_size
{
    size_t operator()(const _Val &value) const
    {
        return 1;
    }
};

template<class _Val, class _SizeOfValue>
struct lv_node_base : public _Rb_tree_node_base_template<lv_node_base<_Val, _SizeOfValue> >
{
    typedef _Rb_tree_node_base_template<lv_node_base> _Template_type;
    size_t  _M_size;

    lv_node_base()
        : _M_size((size_t)-1)   /* -1 should be safety for header only */
    {
    }

    static void _Rb_clone_node(lv_node_base *target,
                               lv_node_base const *source)
    {
        _Template_type::_Rb_clone_node(target, source);
        target->_M_size = source->_M_size;
    }

    static void 
    _Rb_tree_rotate_left(lv_node_base* const __x, 
                         typename _Template_type::_Base_ptr_proxy & __root)
    {
        lv_node_base* const __y = __x->_M_right;
        _Template_type::_Rb_tree_rotate_left(__x, __root);

        if (__x->_M_right != 0)
        {
            __x->_M_size = __x->_M_size + __x->_M_right->_M_size - __y->_M_size;
            __y->_M_size = __y->_M_size + __x->_M_size - __x->_M_right->_M_size;
        }
        else
        {
            __x->_M_size = __x->_M_size - __y->_M_size;
            __y->_M_size = __y->_M_size + __x->_M_size;
        }
    }

    static void 
    _Rb_tree_rotate_right(lv_node_base* const __x, 
                          typename _Template_type::_Base_ptr_proxy & __root)
    {
        lv_node_base* const __y = __x->_M_left;
        _Template_type::_Rb_tree_rotate_right(__x, __root);

        if (__x->_M_left != 0)
        {
            __x->_M_size = __x->_M_size + __x->_M_left->_M_size - __y->_M_size;
            __y->_M_size = __y->_M_size + __x->_M_size - __x->_M_left->_M_size;
        }
        else
        {
            __x->_M_size = __x->_M_size - __y->_M_size;
            __y->_M_size = __y->_M_size + __x->_M_size;
        }
    }

    static void
    _Rb_tree_adjust_node_size(lv_node_base* __x, /* Element to adjust size */
                              lv_node_base& __header,
                              size_t             add_size,
                              size_t             sub_size)
    {
        if(add_size == sub_size)
            return;
        for (lv_node_base *__parent = __x;
            __parent != &__header;
            __parent = __parent->_M_parent())
            __parent->_M_size = __parent->_M_size + add_size - sub_size;
    }

    static void
    _Rb_tree_on_insert(const bool __insert_left,
                       lv_node_base* __x, /* New inserted element */
                       lv_node_base* __p, /* Parent of element x */
                       lv_node_base& __header)
    {
        _Template_type::_Rb_tree_on_insert(__insert_left, __x, __p, __header);
        _Val &value = static_cast<_Rb_tree_node<_Val, lv_node_base> *>(__x)->_M_value_field;
        size_t size = _SizeOfValue()(value);

        __x->_M_size = size;
        _Rb_tree_adjust_node_size(__p, __header, size, 0);
    }

    static void
    _Rb_tree_on_erase(lv_node_base* const __z,  /* Element to be erased */
                      lv_node_base* __y,        /* Element to replace the position of __z */
                      lv_node_base* __x,        /* Origal right element of __y */
                      lv_node_base* __x_parent, /* Virtual parent of __x */
                      lv_node_base& __header)
    {
        _Template_type::_Rb_tree_on_erase(__z, __y, __x, __x_parent, __header);
        _Val &value_z = static_cast<_Rb_tree_node<_Val, lv_node_base> *>(__z)->_M_value_field;
        size_t size_z = _SizeOfValue()(value_z);

        _Val &value_y = static_cast<_Rb_tree_node<_Val, lv_node_base> *>(__y)->_M_value_field;
        size_t size_y = _SizeOfValue()(value_y);

        __y->_M_size = __z->_M_size;
        
        lv_node_base *__parent;
        for (__parent = __x_parent;
            __parent != &__header && __parent != __y;
            __parent = __parent->_M_parent())
            __parent->_M_size = __parent->_M_size - size_y;

        for(; __parent != &__header;
            __parent = __parent->_M_parent())
            __parent->_M_size = __parent->_M_size - size_z;
    }
};


template<typename _Val, typename _Rb_tree_node_base>
struct lv_const_iterator;

template<typename _Val, typename _Rb_tree_node_base>
struct lv_iterator
{
protected:
    typedef lv_const_iterator<_Val, _Rb_tree_node_base> const_iterator;
    typedef ll::_Rb_tree_iterator<_Val, _Rb_tree_node_base>  _Rb_tree_iterator;
public:
    _Rb_tree_iterator _M_itr;
    typedef _Val                                       value_type;
    typedef _Val                                      &reference;
    typedef _Val                                      *pointer;
    typedef std::random_access_iterator_tag            iterator_category;
    typedef ptrdiff_t                                  difference_type;

    typedef lv_iterator<_Val, _Rb_tree_node_base> _Self;
    typedef typename _Rb_tree_iterator::_Base_ptr      _Base_ptr;
    typedef typename _Rb_tree_iterator::_Link_type     _Link_type;

    lv_iterator()
        : _M_itr()
    {
    }

    explicit
    lv_iterator(_Rb_tree_iterator __x)
        : _M_itr(__x)
    {
    }

    explicit
    lv_iterator(typename _Rb_tree_iterator::_Link_type __x)
        : _M_itr(__x)
    {
    }

    reference
    operator*() const
    { return *_M_itr; }

    pointer
    operator->() const
    { return _M_itr.operator->(); }

    _Self&
    operator++()
    {
        ++_M_itr;
        return *this;
    }

    _Self
    operator++(int)
    {
        _Self __tmp = *this;
        _M_itr++;
        return __tmp;
    }

    _Self&
    operator--()
    {
        --_M_itr;
        return *this;
    }

    _Self
    operator--(int)
    {
        _Self __tmp = *this;
        _M_itr--;
        return __tmp;
    }

    _Self
    prev()
    {
        _Self __tmp = *this;
        --__tmp;
        return __tmp;
    }

    _Self
    next()
    {
        _Self __tmp = *this;
        ++__tmp;
        return __tmp;
    }

    bool
    operator==(const _Self& __x) const
    { return _M_itr == __x._M_itr; }
    bool
    operator==(const const_iterator& __x) const
    { return _M_itr == __x._M_itr; }

    bool
    operator!=(const _Self& __x) const
    { return _M_itr != __x._M_itr; }
    bool
    operator!=(const const_iterator& __x) const
    { return _M_itr != __x._M_itr; }

    reference
    operator[](const difference_type& __n) const
    {
        return *this->operator+(__n);
    }

    difference_type advance_n(const difference_type& __n)
    {
        difference_type pos = _M_get_begin_distance();
        pos += __n;
        return _M_set_begin_distance(pos);
    }
    
    _Self&
    operator+=(const difference_type& __n)
    {
        advance_n(__n);
        return *this;
    }    

    _Self
    operator+(const difference_type& __n) const
    {
        _Self itr = *this;
        itr += __n;
        return itr;
    }

    _Self&
    operator-=(const difference_type& __n)
    {
        return this->operator+=(-__n);
    }

    _Self
    operator-(const difference_type& __n) const
    {
        return this->operator+(-__n);
    }

    difference_type
    operator-(const _Self& __x) const
    {
        return _M_get_begin_distance() - __x._M_get_begin_distance();
    }
    difference_type
    operator-(const const_iterator& __x) const
    {
        return _M_get_begin_distance() - __x._M_get_begin_distance();
    }

#if 1
    bool
    operator<(const _Self& __x) const
    { return *this - __x < 0; }
    bool
    operator<(const const_iterator& __x) const
    { return *this - __x < 0; }
    bool
    operator<=(const _Self& __x) const
    { return *this - __x <= 0; }
    bool
    operator<=(const const_iterator& __x) const
    { return *this - __x <= 0; }
    bool
    operator>(const _Self& __x) const
    { return *this - __x > 0; }
    bool
    operator>(const const_iterator& __x) const
    { return *this - __x > 0; }
    bool
    operator>=(const _Self& __x) const
    { return *this - __x >= 0; }
    bool
    operator>=(const const_iterator& __x) const
    { return *this - __x >= 0; }
#endif

    bool is_end() const
    {
        return (_M_itr._M_node->_M_size == (size_t)-1);
    }

    _Base_ptr _M_get_header() const
    {
        _Base_ptr ptr = _M_itr._M_node;
        while (ptr->_M_size != (size_t)-1)
            ptr = ptr->_M_parent();
        return ptr;
    }

    difference_type _M_get_begin_distance() const
    {
        _Base_ptr ptr = _M_itr._M_node;
        
        if (ptr->_M_size == (size_t)-1)
            return (ptr->_M_parent() != 0 && ptr->_M_parent() != ptr) ? ptr->_M_parent()->_M_size : 0;
        
        difference_type pos = ( ptr->_M_left == 0 ? 0 : ptr->_M_left->_M_size );

        while (ptr->_M_parent()->_M_size != (size_t)-1)
        {
            if (ptr->_M_parent()->_M_right == ptr)
                pos += ptr->_M_parent()->_M_size - ptr->_M_size;
            ptr = ptr->_M_parent();
        }
        return pos;
    }
    
    difference_type _M_set_begin_distance(const difference_type& pos)
    {
        difference_type left = 0;

        _Base_ptr header = _M_get_header();
        _Base_ptr __x = header->_M_parent();
        while (__x != 0)
        {
            if ((difference_type)__x->_M_size <= pos - left)
            {
                left += __x->_M_size;
                break;
            }
        
            difference_type left_size  = __x->_M_left ? __x->_M_left->_M_size : 0;
            difference_type right_size = __x->_M_right ? __x->_M_right->_M_size : 0;
            if (pos - left < left_size)
                __x = __x->_M_left;
            else if (pos - left < (difference_type)(__x->_M_size - right_size))
            {
                _M_itr._M_node = __x;
                return left + left_size;
            }
            else
            {
                right_size = __x->_M_right ? __x->_M_right->_M_size : 0;  
                left += __x->_M_size - right_size;

                __x = __x->_M_right;
            }
        }
      
        _M_itr._M_node = header;
        return left;
    }
};

template<typename _Val, typename _Rb_tree_node_base>
struct lv_const_iterator
{
protected:
    typedef lv_iterator<_Val, _Rb_tree_node_base>            iterator;
    typedef ll::_Rb_tree_const_iterator<_Val, _Rb_tree_node_base> _Rb_tree_const_iterator;
public:
    _Rb_tree_const_iterator _M_itr;
    typedef _Val                                              value_type;
    typedef const _Val                                       &reference;
    typedef const _Val                                       *pointer;
    typedef std::random_access_iterator_tag                   iterator_category;
    typedef ptrdiff_t                                         difference_type;

    typedef lv_const_iterator<_Val, _Rb_tree_node_base>  _Self;
    typedef typename _Rb_tree_const_iterator::_Const_Base_ptr _Const_Base_ptr;
    typedef typename _Rb_tree_const_iterator::_Link_type      _Link_type;


    lv_const_iterator()
        : _M_itr()
    {
    }

    explicit
    lv_const_iterator(_Rb_tree_const_iterator __x)
        : _M_itr(__x)
    {
    }

    explicit
    lv_const_iterator(typename _Rb_tree_const_iterator::_Link_type __x)
        : _M_itr(__x)
    {
    }
  
    lv_const_iterator(const iterator& __it)
        : _M_itr(__it._M_itr)
    {
    }

    reference
    operator*() const
    { return *_M_itr; }

    pointer
    operator->() const
    { return _M_itr.operator->(); }
  
    _Self&
    operator++()
    {
        ++_M_itr;
        return *this;
    }
  
    _Self
    operator++(int)
    {
        _Self __tmp = *this;
        _M_itr++;
        return __tmp;
    }
  
    _Self&
    operator--()
    {
        --_M_itr;
        return *this;
    }

    _Self
    operator--(int)
    {
        _Self __tmp = *this;
        _M_itr--;
        return __tmp;
    }

    _Self
    prev()
    {
        _Self __tmp = *this;
        --__tmp;
        return __tmp;
    }

    _Self
    next()
    {
        _Self __tmp = *this;
        ++__tmp;
        return __tmp;
    }

    bool
    operator==(const _Self& __x) const
    { return _M_itr == __x._M_itr; }
    bool
    operator==(const iterator& __x) const
    { return _M_itr == __x._M_itr; }
  
    bool
    operator!=(const _Self& __x) const
    { return _M_itr != __x._M_itr; }
  
    bool
    operator!=(const iterator& __x) const
    { return _M_itr != __x._M_itr; }


    reference
    operator[](const difference_type& __n) const
    {
        return *this->operator+(__n);
    }

    difference_type advance_n(const difference_type& __n)
    {
        difference_type pos = _M_get_begin_distance();
        pos += __n;
        return _M_set_begin_distance(pos);
    }
    
    _Self&
    operator+=(const difference_type& __n)
    {
        advance_n(__n);
        return *this;
    }
    

    _Self
    operator+(const difference_type& __n) const
    {
        _Self itr = *this;
        itr += __n;
        return itr;
    }

    _Self&
    operator-=(const difference_type& __n)
    {
        return this->operator+=(-__n);
    }

    _Self
    operator-(const difference_type& __n) const
    {
        return this->operator+(-__n);
    }

    difference_type
    operator-(const _Self& __x) const
    {
        return _M_get_begin_distance() - __x._M_get_begin_distance();
    }
    difference_type
    operator-(const iterator& __x) const
    {
        return _M_get_begin_distance() - __x._M_get_begin_distance();
    }

#if 1
    bool
    operator<(const _Self& __x) const
    { return *this - __x < 0; }
    bool
    operator<(const iterator& __x) const
    { return *this - __x < 0; }
    bool
    operator<=(const _Self& __x) const
    { return *this - __x <= 0; }
    bool
    operator<=(const iterator& __x) const
    { return *this - __x <= 0; }
    bool
    operator>(const _Self& __x) const
    { return *this - __x > 0; }
    bool
    operator>(const iterator& __x) const
    { return *this - __x > 0; }
    bool
    operator>=(const _Self& __x) const
    { return *this - __x >= 0; }
    bool
    operator>=(const iterator& __x) const
    { return *this - __x >= 0; }
#endif

    bool is_end() const
    {
        return (_M_itr._M_node->_M_size == (size_t)-1);
    }

    _Const_Base_ptr _M_get_header() const
    {
        _Const_Base_ptr ptr = _M_itr._M_node;
        while (ptr->_M_size != (size_t)-1)
            ptr = ptr->_M_parent();
        return ptr;
    }

    difference_type _M_get_begin_distance() const
    {
        _Const_Base_ptr ptr = _M_itr._M_node;
        
        if (ptr->_M_size == (size_t)-1)
            return (ptr->_M_parent() != 0 && ptr->_M_parent() != ptr) ? ptr->_M_parent()->_M_size : 0;
        
        difference_type pos = ( ptr->_M_left == 0 ? 0 : ptr->_M_left->_M_size );

        while (ptr->_M_parent()->_M_size != (size_t)-1)
        {
            if (ptr->_M_parent()->_M_right == ptr)
                pos += ptr->_M_parent()->_M_size - ptr->_M_size;
            ptr = ptr->_M_parent();
        }
        return pos;
    }
    
    
    difference_type _M_set_begin_distance(const difference_type& pos)
    {
        difference_type left = 0;

        _Const_Base_ptr header = _M_get_header();
        _Const_Base_ptr __x = header->_M_parent();
        while (__x != 0)
        {
            if ((difference_type)__x->_M_size <= pos - left)
            {
                left += __x->_M_size;
                break;
            }
        
            difference_type left_size  = __x->_M_left ? __x->_M_left->_M_size : 0;
            difference_type right_size = __x->_M_right ? __x->_M_right->_M_size : 0;
            if (pos - left < left_size)
                __x = __x->_M_left;
            else if (pos - left < (difference_type)(__x->_M_size - right_size))
            {
                _M_itr._M_node = __x;
                return left + left_size;
            }
            else
            {
                right_size = __x->_M_right ? __x->_M_right->_M_size : 0;  
                left += __x->_M_size - right_size;
                __x = __x->_M_right;    
            }
        }
      
        _M_itr._M_node = header;
        return left;
    }    
};


template<typename _Val, typename _Rb_tree_node_base>
inline bool
operator==(const lv_iterator<_Val, _Rb_tree_node_base>& __x,
           const lv_const_iterator<_Val, _Rb_tree_node_base>& __y)
{ return __x._M_itr == __y._M_itr; }

template<typename _Val, typename _Rb_tree_node_base>
inline bool
operator!=(const lv_iterator<_Val, _Rb_tree_node_base>& __x,
           const lv_const_iterator<_Val, _Rb_tree_node_base>& __y)
{ return __x._M_itr != __y._M_itr; }


template<typename _Val, typename _SizeOfValue = lv_size<_Val>, typename _Alloc = std::allocator<_Val> >
class lv
    : protected _Rb_tree<_Val, _Val, lv_key_of_value<_Val>, lv_key_compare<_Val>, _Alloc, lv_node_base<_Val, _SizeOfValue> >
{
protected:
    typedef _Rb_tree
    <
        _Val,
        _Val,
        lv_key_of_value<_Val>,
        lv_key_compare<_Val>,
        _Alloc,
        lv_node_base<_Val, _SizeOfValue>
    >                                            _Rep_type;
    typedef typename _Rep_type::_Link_type       _Link_type;
    typedef typename _Rep_type::_Const_Link_type _Const_Link_type;
    typedef typename _Rep_type::_Base_type       _Base_type;
    typedef typename _Rep_type::_Base_ptr        _Base_ptr;

public:
    typedef _Val              value_type;
    typedef value_type       *pointer;
    typedef const value_type *const_pointer;
    typedef value_type       &reference;
    typedef const value_type &const_reference;
    typedef size_t            size_type;
    typedef ptrdiff_t         difference_type;
    typedef _Alloc            allocator_type;
    typedef _SizeOfValue      sizeof_type;

    allocator_type
    get_allocator() const
    { return _Rep_type::get_allocator(); }

    typedef lv_iterator<value_type, typename _Rep_type::_Base_type>       iterator;
    typedef lv_const_iterator<value_type, typename _Rep_type::_Base_type> const_iterator;

    typedef std::reverse_iterator<iterator>       reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

public:
    // allocation/deallocation
    lv()
    { }

    explicit lv(const allocator_type& __a)
        : _Rep_type(lv_key_compare<_Val>(), __a)
    { }

    explicit lv(size_type __count)
    {
        insert(begin(), __count, value_type());
    }

    lv(size_type __count, const value_type &__val)
    {
        insert(begin(), __count, __val);
    }

    lv(size_type __count, const value_type &__val, const allocator_type& __a)
    : _Rep_type(lv_key_compare<_Val>(), __a)
    {
        insert(begin(), __count, __val);
    }

    template<typename _InputIterator>
    lv(_InputIterator __first, _InputIterator __last)
    {
        insert(begin(), __first, __last);
    }

    template<typename _InputIterator>
    lv(_InputIterator __first, _InputIterator __last, const allocator_type& __a)
        : _Rep_type(__a)
    {
        insert(begin(), __first, __last);
    }

    lv(const lv<_Val, _SizeOfValue, _Alloc>& __x)
    : _Rep_type(static_cast<const _Rep_type&>(__x))
    {
    }

    ~lv()
    {
    }

    void
    assign(size_type __n, const value_type& __val)
    {
        clear();
        insert(begin(), __n, __val);
    }

    template<typename _InputIterator>
    void
    assign(_InputIterator __first, _InputIterator __last)
    {
        clear();
        insert(begin(), __first, __last);
    }

    reference
    operator[](size_type __n)
    {
        return *(begin() + __n);
    }
      
    const_reference
    operator[](size_type __n) const
    {
        return *(begin() + __n);
    }

protected:
    void
    _M_range_check(size_type __n) const
    {
        if (__n >= expand_size())
            throw std::out_of_range("lv:_M_range_check");
    }
public:
    reference
    at(size_type __n)
    {
        _M_range_check(__n);
        return this->operator[](__n);
    }
      
    const_reference
    at(size_type __n) const
    {
        _M_range_check(__n);
        return this->operator[](__n);
    }

    // Accessors.
    iterator
    begin()
    { 
        return iterator(_Rep_type::begin());
    }

    const_iterator
    begin() const
    { 
        return const_iterator(_Rep_type::begin());
    }

    iterator
    end()
    {
        return iterator(_Rep_type::end());
    }

    const_iterator
    end() const
    {
        return const_iterator(_Rep_type::end());
    }

    reverse_iterator
    rbegin()
    {
        return reverse_iterator(end());
    }

    const_reverse_iterator
    rbegin() const
    {
        return const_reverse_iterator(end());
    }

    reverse_iterator
    rend()
    {
        return reverse_iterator(begin());;
    }

    const_reverse_iterator
    rend() const
    {
        return const_reverse_iterator(begin());;
    }

    reference
    front()
    {
        return *begin();
    }

    const_reference
    front() const
    {
        return *begin();
    }

    reference
    back()
    {
        return *rbegin();
    }

    const_reference
    back() const
    {
        return *rbegin();
    }

    bool
    empty() const
    { return _Rep_type::empty(); }

    size_type
    size() const
    { return _Rep_type::size(); }

    size_type
    max_size() const
    { return _Rep_type::max_size(); }
    
    size_type
    expand_size() const
    {
        return _Rep_type::_M_begin()
            ? _Rep_type::_M_begin()->_M_size
            : 0;
    }

    void
    swap(lv<_Val, _SizeOfValue, _Alloc>& __t);

    lv<_Val, _SizeOfValue, _Alloc>&
    operator=(const lv<_Val, _SizeOfValue, _Alloc>& __t) {
        lv<_Val, _SizeOfValue, _Alloc>(__t).swap(*this);
        return *this;
    }

    // Insert/erase.
    iterator
    insert(iterator __position, const value_type& __x);

    void
    insert(iterator __position, size_type __n, const value_type& __x)
    {
        _insert_n(__position, __n, __x);
    }

    template<bool is_integer>
    struct _insert_chooser
    {
    };

    template<typename _InputIterator>
    void
    insert(iterator __position, _InputIterator __first, _InputIterator __last)
    {
        _insert(__position, __first, __last, _insert_chooser<std::numeric_limits<_InputIterator>::is_integer>());
    }
        
    void
    _insert_n(iterator __position, size_type __n, const value_type& __x)
    {
        size_type old_size = __n;
        try
        {
            while (__n > 0)
            {
                insert(__position, __x);
                --__n;
            }
        }
        catch(...)
        {
            while (old_size > __n)
            {
                iterator before = __position;
                erase(--before);
                --old_size;
            }
            throw;
        }
    }

    template<typename _InputIterator>
    void
    _insert(iterator __position, _InputIterator __first, _InputIterator __last, const _insert_chooser<false> &)
    {
        _InputIterator old_first = __first;
        try
        {
            while (__first != __last)
            {
                insert(__position, *__first);              
                ++__first;
            }
        }
        catch(...)
        {
            while (old_first != __first)
            {
                iterator before = __position;
                erase(--before);
                ++old_first;
            }
            throw;
        }
    }

    template<typename _InputIterator>
    void
    _insert(iterator __position, _InputIterator __first, _InputIterator __last, const _insert_chooser<true> &)
    {
        _insert_n(__position, __first, __last);
    }

     
    iterator
    erase(iterator __position);

    const_iterator
    erase(const_iterator __position);

    iterator
    erase(iterator __first, iterator __last);

    const_iterator
    erase(const_iterator __first, const_iterator __last);

    void
    clear()
    {
        _Rep_type::clear();
    }

#if 1   //Functions in this block need the values are well sorted
    iterator
    find(const value_type& __v)
    { return iterator(_Rep_type::find(__v)); }

    const_iterator
    find(const value_type& __v) const
    { return const_iterator(_Rep_type::find(__v)); }
   
    iterator
    lower_bound(const value_type& __v)
    { return iterator(_Rep_type::lower_bound(__v)); }

    const_iterator
    lower_bound(const value_type& __v) const
    { return const_iterator(_Rep_type::lower_bound(__v)); }

    iterator
    upper_bound(const value_type& __v)
    { return iterator(_Rep_type::upper_bound(__v)); }

    const_iterator
    upper_bound(const value_type& __v) const
    { return const_iterator(_Rep_type::upper_bound(__v)); }

    std::pair<iterator, iterator>
    equal_range(const value_type& __v)
    {
        std::pair<typename _Rep_type::iterator, typename _Rep_type::iterator>
            its = _Rep_type::equal_range(__v);
        return std::pair<iterator, iterator>
            (iterator(its.first), iterator(its.second));
    }

    std::pair<const_iterator, const_iterator>
    equal_range(const value_type& __v) const
    {
        std::pair<typename _Rep_type::const_iterator, typename _Rep_type::const_iterator>
            its = _Rep_type::equal_range(__v);
        return std::pair<const_iterator, const_iterator>
            (const_iterator(its.first), const_iterator(its.second));
    }
#endif

    void
    push_front(const value_type& __x)
    {
        this->insert(begin(), __x);
    }

    void
    push_back(const value_type& __x)
    {
        this->insert(end(), __x);
    }
      
    void
    pop_front()
    {
        this->erase(begin());
    }
      
    void
    pop_back()
    {
        this->erase(end() - 1);
    }

    void
    splice(iterator __position, lv &__x)
    {
        if (!__x.empty())
          splice(__position, __x, __x.begin(), __x.end());
    }
    void
    splice(iterator __position, lv &right, iterator __i)
    {
        iterator __j = __i;
        ++__j;
        if (__position == __i || __position == __j)
          return;
        splice(__position, right, __i, __j);
    }
    void
    splice(iterator __position, lv &right, iterator __first, iterator __last)
    {
        while (__first != __last)
        {
            iterator node = __first;
            ++__first;
            _Link_type __z =
            static_cast<_Link_type>(
                _Rep_type::_Base_type::_Rb_tree_rebalance_for_erase(
                    node._M_itr._M_node,
                    right._M_impl._M_header));
            --right._Rep_type::_M_impl._M_node_count;

            _Base_ptr __p = __position._M_itr._M_node;
      
            _Base_ptr __x = __p;
            _Base_ptr __y = _Rep_type::_M_end();
            if (__x == _Rep_type::_M_end())
                __y = _Rep_type::_M_rightmost();
            else
            {
                __y = __x;
                __x = __x->_M_left;
                while (__x != 0)
                {
                    __y = __x;
                    __x = __x->_M_right;
                }
            }

            bool __insert_left = (size() == 0 || (__p != _Rep_type::_M_end() && __p->_M_left == 0));

            _Rep_type::_Base_type::_Rb_tree_insert_and_rebalance(__insert_left, __z, __y,  
                                    this->_M_impl._M_header);
            ++_Rep_type::_M_impl._M_node_count;
        }
    }
      
    void
    reverse()
    {
        lv tmp;
        tmp.splice(tmp.begin(), *this);
        while (tmp.size() > 0)
            splice(begin(), tmp, tmp.begin());
    }
      
    void
    remove(const value_type &__val)
    {
        for (iterator itr = begin(); itr != end();)
        {
            iterator node = itr;
            ++itr;
          
            if (*node == __val)
                erase(node);
        }
    }
      
    template<class _Predicate>
    void
    remove_if(_Predicate _Pred)
    {
        for (iterator itr = begin(); itr != end();)
        {
            iterator node = itr;
            ++itr;
          
            if (_Pred.operator()(*node))
                erase(node);
        }
    }
      
    void
    unique()
    {
        iterator cmp_node = end();
        for (iterator itr = begin(); itr != end();)
        {
            iterator node = itr;
            ++itr;
          
            if (cmp_node != end() && *node == *cmp_node)
                erase(node);
            else
                cmp_node = node;
        }
    }

    template<class _Predicate>
    void
    unique(_Predicate _Pred)
    {
        iterator cmp_node = end();
        for (iterator itr = begin(); itr != end();)
        {
          iterator node = itr;
          ++itr;
          
          if (cmp_node != end() && _Pred.operator()(*node, *cmp_node))
            erase(node);
          else
            cmp_node = node;
        }
    }
      

protected:
    typedef std::vector<std::pair<size_t, iterator>,
        typename _Alloc::template rebind<std::pair<size_t, iterator> >::other
    > comp_help;
    
    template<typename _StrictWeakOrdering>
    struct compi
    {
        compi(_StrictWeakOrdering comp)
            : _comp(comp)
        {
        }
        bool operator()(const typename comp_help::value_type &l,
                        const typename comp_help::value_type &r)
        {
            return _comp(*l.second, *r.second);
        }
        _StrictWeakOrdering _comp;
    };

public: 
    template<typename _StrictWeakOrdering>
    void
    sort(_StrictWeakOrdering comp)
    {
        if(this->size() == 0)
            return;
        comp_help v;
        std::vector<iterator, typename _Alloc::template rebind<iterator>::other> f;
        v.reserve(this->size());
        f.reserve(this->size());
        size_t j = 0;
        for(iterator i = begin(); i < end(); ++i, ++j)
        {
            v.push_back(std::make_pair(j, i));
            f.push_back(i);
        }
        
        std::sort(v.begin(), v.end(), compi<_StrictWeakOrdering>(comp));
        
        _SizeOfValue getSize;
        for(size_t i = 0; i < v.size(); ++i)
        {
#if 0
            iterator it = v[i].second;
            push_back(*it);
            this->erase(it);
#else                
            iterator stop = f[i];
            
            while(v[i].second != stop)
            {
                size_t size0 = getSize(*f[i]);
                size_t size1 = getSize(*v[i].second);
                
                _Base_type::_Rb_tree_adjust_node_size(
                    f[i]._M_itr._M_node, this->_M_impl._M_header, size1, size0);
                _Base_type::_Rb_tree_adjust_node_size(
                    v[i].second._M_itr._M_node, this->_M_impl._M_header, size0, size1);
                    
                std::swap(*f[i], *v[i].second);
                v[i].second = f[i];
                i = v[i].first;
            }
            if(stop != f[i])
            {
                size_t size0 = getSize(*f[i]);
                size_t size1 = getSize(*stop);
                
                _Base_type::_Rb_tree_adjust_node_size(
                    f[i]._M_itr._M_node, this->_M_impl._M_header, size1, size0);
                _Base_type::_Rb_tree_adjust_node_size(
                    stop._M_itr._M_node, this->_M_impl._M_header, size0, size1);
            
                std::swap(*f[i], *stop);                               
                i = v[i].first;
            }            
#endif
        }

#if 0  //Verify
        iterator j1;
        for(iterator i1 = begin(); i1 != end(); ++i1)
        {
            if(i1 != begin())
            {
                if(*i1 < *j1)
                {
                    printf("Errror\n"); while(1);
                }
            }
            j1 = i1;
        }
#endif
    }

    void
    sort()
    {
        sort(std::less<_Val>());
    }

    void
    resize(size_type __new_size, const value_type& __x)
    {
        while (__new_size < size())
        {
            iterator itr = end();
            erase(--itr);
        }
        if (__new_size > size())
            insert(end(), __new_size - size(), __x);
    }
      
    void
    resize(size_type __new_size)
    { 
        while (__new_size < size())
        {
            iterator itr = end();
            erase(--itr);
        }
        if (__new_size > size())
            insert(end(), __new_size - size(), value_type());
    }
      
    void
    merge(lv &right)
    {
        splice(begin(), right);
        sort();
    }

    template<typename _StrictWeakOrdering>
    void
    merge(lv &right, _StrictWeakOrdering comp)
    {
        splice(begin(), right);
        sort(comp);
    }

    void adjust_node_size(iterator &itr, size_t add_size, size_t sub_size)
    {
        _Base_type::_Rb_tree_adjust_node_size(itr._M_itr._M_node, this->_M_impl._M_header, add_size, sub_size);
    }
    void adjust_node_size(const_iterator &itr, size_t add_size, size_t sub_size)
    {
        _Base_type::_Rb_tree_adjust_node_size(const_cast<typename _Rep_type::_Base_ptr>(itr._M_itr._M_node), this->_M_impl._M_header, add_size, sub_size);
    }

    // Debugging.
    bool
    __rb_verify() const
    {
        size_type i = 0;
        for (const_iterator __it = begin(); __it != end(); ++__it, ++i)
        {
            _Const_Link_type __x = static_cast<_Const_Link_type>(__it._M_itr._M_node);
            _Const_Link_type __L = _Rep_type::_S_left(__x);
            _Const_Link_type __R = _Rep_type::_S_right(__x);

            if (__it._M_get_header() != &this->_M_impl._M_header)
                return false;
            //if (__it._M_get_begin_distance() != i)
            //    return false;
   
            if (__x->_M_size != ( __L == 0 ? 0 : __L->_M_size )
                + ( __R == 0 ? 0 : __R->_M_size )
                + _SizeOfValue()(__x->_M_value_field) )
                return false;
        }

        return _Rep_type::__rb_verify();
    }
};

template<typename _Val, typename _SizeOfValue, typename _Alloc>
inline bool
operator==(const lv<_Val, _SizeOfValue, _Alloc>& __x,
           const lv<_Val, _SizeOfValue, _Alloc>& __y)
{
    return __x.size() == __y.size()
        && std::equal(__x.begin(), __x.end(), __y.begin());
}

template<typename _Val, typename _SizeOfValue, typename _Alloc>
inline bool
operator<(const lv<_Val, _SizeOfValue, _Alloc>& __x,
          const lv<_Val, _SizeOfValue, _Alloc>& __y)
{
    return std::lexicographical_compare(__x.begin(), __x.end(), 
                                        __y.begin(), __y.end());
}

template<typename _Val, typename _SizeOfValue, typename _Alloc>
inline bool
operator!=(const lv<_Val, _SizeOfValue, _Alloc>& __x,
           const lv<_Val, _SizeOfValue, _Alloc>& __y)
{ return !(__x == __y); }

template<typename _Val, typename _SizeOfValue, typename _Alloc>
inline bool
operator>(const lv<_Val, _SizeOfValue, _Alloc>& __x,
          const lv<_Val, _SizeOfValue, _Alloc>& __y)
{ return __y < __x; }

template<typename _Val, typename _SizeOfValue, typename _Alloc>
inline bool
operator<=(const lv<_Val, _SizeOfValue, _Alloc>& __x,
           const lv<_Val, _SizeOfValue, _Alloc>& __y)
{ return !(__y < __x); }

template<typename _Val, typename _SizeOfValue, typename _Alloc>
inline bool
operator>=(const lv<_Val, _SizeOfValue, _Alloc>& __x,
           const lv<_Val, _SizeOfValue, _Alloc>& __y)
{ return !(__x < __y); }

template<typename _Val, typename _SizeOfValue, typename _Alloc>
inline void
swap(lv<_Val, _SizeOfValue, _Alloc>& __x,
     lv<_Val, _SizeOfValue, _Alloc>& __y)
{ __x.swap(__y); }

template<typename _Val, typename _SizeOfValue, typename _Alloc>
inline void
lv<_Val, _SizeOfValue, _Alloc>::
swap(lv<_Val, _SizeOfValue, _Alloc>& __t)
{
    _Rep_type::swap(static_cast<_Rep_type&>(__t));
}

template<typename _Val, typename _SizeOfValue, typename _Alloc>
inline typename lv<_Val, _SizeOfValue, _Alloc>::iterator
lv<_Val, _SizeOfValue, _Alloc>::
insert(iterator __position, const value_type& __v)
{
    _Base_ptr __p = __position._M_itr._M_node;

    _Base_ptr __x = __p;
    _Base_ptr __y = _Rep_type::_M_end();
    if (__x == _Rep_type::_M_end())
        __y = _Rep_type::_M_rightmost();
    else
    {
        __y = __x;
        __x = __x->_M_left;
        while (__x != 0)
        {
            __y = __x;
            __x = __x->_M_right;
        }
    }

    bool __insert_left = (size() == 0 || (__p != _Rep_type::_M_end() && __p->_M_left == 0));

    _Link_type __z = _Rep_type::_M_create_node(__v);
    _Rep_type::_Base_type::_Rb_tree_insert_and_rebalance(
        __insert_left, __z, __y,
        this->_M_impl._M_header);
    ++_Rep_type::_M_impl._M_node_count;
    return iterator(__z);
}

template<typename _Val, typename _SizeOfValue, typename _Alloc>
inline typename lv<_Val, _SizeOfValue, _Alloc>::iterator
lv<_Val, _SizeOfValue, _Alloc>::
erase(iterator __position)
{
    iterator itr = __position;
    ++itr;
    _Link_type __y =
    static_cast<_Link_type>(
        _Rep_type::_Base_type::_Rb_tree_rebalance_for_erase(
            __position._M_itr._M_node,
            this->_M_impl._M_header));
    --_Rep_type::_M_impl._M_node_count;
    _Rep_type::_M_destroy_node(__y);
    return itr;
}

template<typename _Val, typename _SizeOfValue, typename _Alloc>
inline typename lv<_Val, _SizeOfValue, _Alloc>::const_iterator
lv<_Val, _SizeOfValue, _Alloc>::
erase(const_iterator __position)
{
    const_iterator itr = __position;
    ++itr;
    _Link_type __y =
    static_cast<_Link_type>(
        _Rep_type::_Base_type::_Rb_tree_rebalance_for_erase(
            const_cast<_Base_ptr>(__position._M_itr._M_node),
            this->_M_impl._M_header));
    --_Rep_type::_M_impl._M_node_count;
    _Rep_type::_M_destroy_node(__y);
    return itr;
}

template<typename _Val, typename _SizeOfValue, typename _Alloc>
inline typename lv<_Val, _SizeOfValue, _Alloc>::iterator
lv<_Val, _SizeOfValue, _Alloc>::
erase(iterator __first, iterator __last)
{
    typename lv<_Val, _SizeOfValue, _Alloc>::iterator itr = end();
    if (__first == begin() && __last == end())
        clear();
    else
        while (__first != __last)
            itr = erase(__first++);
    return itr;
}

template<typename _Val, typename _SizeOfValue, typename _Alloc>
inline typename lv<_Val, _SizeOfValue, _Alloc>::const_iterator
lv<_Val, _SizeOfValue, _Alloc>::
erase(const_iterator __first, const_iterator __last)
{
    typename lv<_Val, _SizeOfValue, _Alloc>::const_iterator itr = end();
    if (__first == begin() && __last == end())
        clear();
    else
        while (__first != __last)
            itr = erase(__first++);
    return itr;
}

}

#endif
