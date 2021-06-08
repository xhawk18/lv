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
//#include <boost/container/stable_vector.hpp>

template<class VALUE>
void sort(std::deque<VALUE> &c)
{
    std::sort(c.begin(), c.end());
}
template<class VALUE>
void sort(std::vector<VALUE> &c)
{
    std::sort(c.begin(), c.end());
}

template<class VALUE>
void sort(ll::lv<VALUE> &c)
{
    c.sort();
}

template<class VALUE>
void sort(std::list<VALUE> &c)
{
    c.sort();
}

#define MAX_LEN 100000
template<class CONTAINER, class VALUE>
struct test
{
    /* Check the operation times in giving seconds. */
    static void test_speed(const char *name, time_t seconds)
    {
        CONTAINER container;
        for(int i = 0; i < MAX_LEN; ++i)
            container.push_back(VALUE(rand()));

        time_t start = time(0);
        unsigned int times = 0;
        while(time(0) - start < 1);
        start++;

        while(time(0) - start < seconds)
        {
            {
                typename CONTAINER::iterator itr = container.begin();
                std::advance( itr, rand()%MAX_LEN );
                *itr = VALUE(rand());

                itr = container.erase(itr);
                itr = container.insert(itr, VALUE(rand()));
                
                //sort(container);
            }
            ++times;
        }

        printf("%s, loop %d times\n", name, times);
    }
};


struct value
{
    virtual ~value()
    {
    }
    value(int n):
        m(n), v(n*100)
    {
        //printf("const-----------\n");
    };
    bool operator==( value const &right ) const
    {
        return m == right.m && v == right.v;
    }
    bool operator!=( value const &right ) const
    {
        return !operator==(right);
    }
    bool operator<( value const &right ) const
    {
        if(m < right.m)
            return true;
        if(right.m < m)
            return false;
        if(v < right.v)
            return true;
        return false;
    }
    int m;
    int v;
    char buf[256];
};


template<class CONTAINER>
void test_dump(CONTAINER const &container)
{
    for (typename CONTAINER::const_iterator itr = container.begin();
        itr != container.end();
        ++itr)
        printf("%d,", *itr);
    printf("\n\n");
}

template<class CONTAINER>
void test_insert(CONTAINER const &dummy)
{
    CONTAINER container;
    for (typename CONTAINER::size_type i = 0; i < 100; ++i)
        container.push_back(i*10);
    test_dump (container);

    typename CONTAINER::iterator itr = container.begin();
    ++itr;
    itr++;
    itr = container.insert(itr, 10000);
    test_dump (container);

    itr = container.insert(itr, 20000);
    test_dump (container);

    container.insert(itr, 3, 50000);
    test_dump (container);

    int source[] = {100, 200, 300, 400, 500};
    container.insert(itr, source, source + sizeof(source)/sizeof(source[0]));
    test_dump (container);
}


void test_compare(time_t seconds)
{
    printf("Do long time compare testing...\n");

    //typedef ll::alv<int> T1;
    typedef ll::lv<int> T1;
    typedef std::list<int> T2;
    //typedef ll::alv<int> T4;
    
    std::list<size_t> offset;
    typedef std::list<T1::iterator> L1;
    typedef std::list<T2::iterator> L2;
    L1 l1;
    L2 l2;

    T1 c1;
    T2 c2;
    for(int i = 0; i < MAX_LEN; ++i)
    {
        int r = rand();
        c1.push_back(r);
        c2.push_back(r);
    }

    time_t start = time(0);
    while(time(0) - start < 1) {
    }
    start++;

    while(time(0) - start < seconds)
    {
        printf("Test compare time %d\n", (int)time(0));
        size_t erase0 = 0;
        size_t erase1 = 0;
        size_t insert_pos = 0;
        size_t insert_size = 0;
        std::vector<int> ins;
        
        if(c1.size() > 0)
        {
            erase0 = rand() % c1.size();
            erase1 = rand() % c1.size();
        }
        if(erase0 > erase1)
            std::swap(erase0, erase1);
        
        size_t new_size = c1.size() - (erase1 - erase0);
        if(new_size > 0)
            insert_pos = rand() % new_size;
        if(new_size < MAX_LEN)
            insert_size = rand() % (MAX_LEN * 3 / 2 - new_size);
        
        /* create insert data */
        if(rand() % 2 == 0)
            ins.resize(0);
        else
        {
            ins.resize(insert_size);
            for(size_t i = 0; i < ins.size(); ++i)
                ins[i] = rand();
        }
                    
        int r0 = rand();

        /* Erase and insert on l1 */
        {
            l1.clear();
            for(T1::iterator t = c1.begin(); t != c1.end(); ++t)
                l1.push_back(t);
            L1::iterator ii0 = l1.begin();
            L1::iterator ii1 = l1.begin();
            std::advance(ii0, erase0);
            std::advance(ii1, erase1);
            l1.erase(ii0, ii1);
                
            //Erase
            T1::iterator i0 = c1.begin();
            T1::iterator i1 = c1.begin();
            std::advance(i0, erase0);
            std::advance(i1, erase1);            
            c1.erase(i0, i1);
            
            //Insert
            T1::iterator i = c1.begin();
            std::advance(i, insert_pos);
            if(ins.size() > 0)
                c1.insert(i, ins.begin(), ins.end());
            else
                c1.insert(i, insert_size, r0);
        }

        /* Erase and insert on l2 */
        {
            l2.clear();
            for(T2::iterator t = c2.begin(); t != c2.end(); ++t)
                l2.push_back(t);
            L2::iterator ii0 = l2.begin();
            L2::iterator ii1 = l2.begin();
            std::advance(ii0, erase0);
            std::advance(ii1, erase1);
            l2.erase(ii0, ii1);
            
            //Erase
            T2::iterator i0 = c2.begin();
            T2::iterator i1 = c2.begin();
            std::advance(i0, erase0);
            std::advance(i1, erase1);
            c2.erase(i0, i1);
            
            //Insert
            T2::iterator i = c2.begin();
            std::advance(i, insert_pos);            
            if(ins.size() > 0)
                c2.insert(i, ins.begin(), ins.end());
            else
                c2.insert(i, insert_size, r0);
        }
        
        /* Compare result */
        if(!std::equal(c1.begin(), c1.end(), c2.begin()))
        {
            printf("1-2 test failed, result not equal!\n");
            getchar();
            break;
        }

        //Compare iterator
        if(l1.size() != l2.size())
        {
            printf("iterator list's size not match 1 vs 2\n");
            getchar();
            break;
        }
        L1::const_iterator ii1 = l1.begin();
        L2::const_iterator ii2 = l2.begin();
        
        while(ii1 != l1.end() && ii2 != l2.end())
        {
            if(**ii1 != **ii2)
            {
                printf("iterator not match 1 vs 2!\n");
                getchar();
                break;
            }
            ++ii1;
            ++ii2;
        }
    }

    printf("Long time compare testing finished\n");
}


void test_splice(time_t seconds)
{
    printf("Do long time splice testing...\n");

    //typedef ll::vlv<int> T1;
    typedef std::list<int> T1;
    typedef ll::lv<int> T2;
    //typedef ll::alv<int> T4;
 
    T1 c1a, c1b;
    T2 c2a, c2b;
    for(int i = 0; i < MAX_LEN; ++i)
    {
        int r = rand();
        c1a.push_back(r);
        c2a.push_back(r);
    }

    time_t start = time(0);
    while(time(0) - start < 1) {
    }
    start++;

    while(time(0) - start < seconds)
    {
        size_t max_size = c1a.size();
        size_t min_size = c1b.size();
        if(max_size < min_size)
            std::swap(max_size, min_size);

        printf("Test splice time %d\n", (int)time(0));
        size_t s0 = 0;
        size_t s1 = 0;
        size_t p = 0;
        if(max_size > 0)
        {
            s0 = rand() % max_size;
            s1 = rand() % max_size;
        }
        if(min_size > 0)
            p = rand() % min_size;
        if(s0 > s1)
            std::swap(s0, s1);
        
#if 0   //Dump for compare */
        for(T2::iterator i = c2a.begin(); i != c2a.end(); ++i)
            printf("%d ", *i);
        printf("\n");
        for(T2::iterator i = c2b.begin(); i != c2b.end(); ++i)
            printf("%d ", *i);
        printf("\n");
        printf("p = %d <= %d, %d\n", p, s0, s1);
#endif
        
        if(c1b.size() < c1a.size())
        {
            {
                T1::iterator i = c1b.begin();
                std::advance(i, p);
                T1::iterator i0 = c1a.begin();
                std::advance(i0, s0);
                T1::iterator i1 = c1a.begin();
                std::advance(i1, s1);            
                c1b.splice(i, c1a, i0, i1);
            }

            {
                T2::iterator i = c2b.begin();
                std::advance(i, p);
                T2::iterator i0 = c2a.begin();
                std::advance(i0, s0);
                T2::iterator i1 = c2a.begin();
                std::advance(i1, s1);            
                c2b.splice(i, c2a, i0, i1);
            }
        }
        else
        {
            {
                T1::iterator i = c1a.begin();
                std::advance(i, p);
                T1::iterator i0 = c1b.begin();
                std::advance(i0, s0);
                T1::iterator i1 = c1b.begin();
                std::advance(i1, s1);                
                c1a.splice(i, c1b, i0, i1);
            }

            {
                T2::iterator i = c2a.begin();
                std::advance(i, p);
                T2::iterator i0 = c2b.begin();
                std::advance(i0, s0);
                T2::iterator i1 = c2b.begin();
                std::advance(i1, s1);                
                c2a.splice(i, c2b, i0, i1);
            }
        }

#if 0   //Dump for compare */
        for(T2::iterator i = c2a.begin(); i != c2a.end(); ++i)
            printf("%d ", *i);
        printf("\n");
        for(T2::iterator i = c2b.begin(); i != c2b.end(); ++i)
            printf("%d ", *i);
        printf("\n");
        for(T1::iterator i = c1a.begin(); i != c1a.end(); ++i)
            printf("%d ", *i);
        printf("\n");
        for(T1::iterator i = c1b.begin(); i != c1b.end(); ++i)
            printf("%d ", *i);
        printf("\n");
        printf("-----------------p = %d <= %d, %d\n", p, s0, s1);
#endif        
        
        if(!std::equal(c1a.begin(), c1a.end(), c2a.begin()))
        {
            printf("1test failed, result not equal!\n");
            getchar();
            break;
        }
        if(!std::equal(c1b.begin(), c1b.end(), c2b.begin()))
        {
            printf("2test failed, result not equal!\n");
            getchar();
            break;
        }
    }

    printf("Long time compare testing finished\n");
}


int main()
{
#ifdef _MSC_VER
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    srand(time(0));
    typedef char VALUE;
        
    while(1)
    {
        test_compare(1);
        test_splice(1);

        //test<boost::container::stable_vector<VALUE>, VALUE>::test_speed(1);
        test<ll::lv<VALUE>, VALUE >::test_speed("ll::lv", 1);
        test<std::vector<VALUE>, VALUE>::test_speed("std::vector", 1);
        test<std::deque<VALUE>, VALUE>::test_speed("std::deque", 1);
        test<std::list<VALUE>, VALUE>::test_speed("std::list", 1);
    
        test_insert(std::list<int>());
        test_insert(ll::lv<int>());
    }

    return 0;
}

