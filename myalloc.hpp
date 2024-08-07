#ifndef __MYALLOCATOR_H__
#define __MYALLOCATOR_H__

#include <cstddef>
#include <pthread.h>
#include <vector>

enum AllocationAlgorithm { FIRST_FIT, BEST_FIT };

class MyAllocator {
public:
    MyAllocator(int size, AllocationAlgorithm algorithm);
    ~MyAllocator();

    void* allocate(int size);
    void deallocate(void* ptr);
    int availableMemory();
    void printStatistics();
    int compactAllocation(std::vector<void*>& before, std::vector<void*>& after);

private:
    struct Block {
        size_t size;
    };

    struct Node {
        Block* block;
        Node* next;
    };

    AllocationAlgorithm algorithm_;
    int size_;
    void* memory_;
    Node* freeList_;
    Node* allocatedList_;
    pthread_mutex_t lock_;

    void initialize(int size, AllocationAlgorithm algorithm);
    void destroy();
};

#endif // __MYALLOCATOR_H__