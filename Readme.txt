
ll::lv is a container that has O(log(n)) erase/insert time and random access time.

Shortly comparison to C++ std containers --
   1. std::vector
        erase/insert, O(n)
        random access, O(1)
   2. std::list
        erase/insert, O(1)
        random access, O(n)
   3. ll::lv
        erase/insert, O(log(n))
        random access, O(log(n))

Container ll::lv supports all interface for std::list, std::vector and std::deque,
and can used to replace list/vector/deque in some case.

This library is header only. To use this libarary in your project, 
just copy ll_lv.hpp and ll_rb_tree.hpp, then include ll_lv.hpp,

An simple example --

#include "ll_lv.hpp"
#include <stdio.h>

int main(){
	ll::lv<int> c;
	c.push_back(1);
	c.push_back(2);
	printf("%d\n", c[1]);
	return 0;
}





