#include <iostream>
#include <vector>
#include "myalloc.hpp"

int main(int argc, char* argv[]) {
    MyAllocator allocator(100, FIRST_FIT);
    std::cout << "Using first fit algorithm on memory size 100" << std::endl;

    std::vector<int*> p(50, nullptr);
    for(int i = 0; i < 10; ++i) {
        p[i] = static_cast<int*>(allocator.allocate(sizeof(int)));
        if(p[i] == nullptr) {
            std::cout << "Allocation failed" << std::endl;
            continue;
        }
        *(p[i]) = i;
        std::cout << "p[" << i << "] = " << p[i] << " ; *p[" << i << "] = " << *(p[i]) << std::endl;
    }

    allocator.printStatistics();

    for(int i = 0; i < 10; ++i) {
        if(i % 2 == 0) {
            continue;
        }
        std::cout << "Freeing p[" << i << "]" << std::endl;
        allocator.deallocate(p[i]);
        p[i] = nullptr;
    }

    std::cout << "available_memory " << allocator.availableMemory() << std::endl;

    std::vector<void*> before(100, nullptr);
    std::vector<void*> after(100, nullptr);
    allocator.compactAllocation(before, after);

    allocator.printStatistics();

    // allocator's destructor will automatically free allocated memory
    return 0;
}