#ifdef _MSC_VER
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "ll_lv.hpp"
#include <time.h>
#include <cstdio>
#include <list>
#include <deque>
#include <vector>
#include <set>
#include <typeinfo>
#include <algorithm>


struct obj
{
    obj()
        : size_((size_t)-1)
        , value_(rand())
    {
    }
    obj(size_t size)
        : size_(size)
        , value_(rand())
    {
    }
        

    size_t size_;
    int    value_;
};


struct obj_size
{
    size_t operator()(const obj &value) const
    {
        if(value.size_ == (size_t)-1)
            const_cast<obj &>(value).size_ = 5 * (rand() % 2);
        return value.size_;
    }
};

int main()
{
    typedef ll::lv<obj, obj_size> L;
    srand(time(0));
    L l;
    
    for(size_t i = rand() % 10; i-- != 0;)
        l.push_back(obj());

    size_t i = 0;
    for(L::iterator it = l.begin(); it != l.end(); ++it, ++i)
        printf("*(iter %d 0x%08x) = %d, size = %d\n", i, it.operator->(), it->value_, it->size_);
    printf("-----------------------------\n");
        
    for(size_t i = 0; i < l.expand_size(); ++i)
    {
        L::iterator it = l.begin() + i;
        printf("*(iter %d 0x%08x) = %d, size = %d, distance = %d\n",
            i, it.operator->(), it->value_, it->size_,
            it - l.begin());
    }
    printf("%d %d\n", l.size(), l.expand_size());

	return 0;
}

