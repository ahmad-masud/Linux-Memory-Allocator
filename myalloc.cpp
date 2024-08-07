#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "myalloc.hpp"

MyAllocator::MyAllocator(int size, AllocationAlgorithm algorithm) {
    initialize(size, algorithm);
}

MyAllocator::~MyAllocator() {
    destroy();
}

void MyAllocator::initialize(int size, AllocationAlgorithm algorithm) {
    assert(size > 0);

    // Align size to 64 bytes
    size = (size + 63) & ~63;

    algorithm_ = algorithm;
    size_ = size;
    memory_ = malloc(static_cast<size_t>(size_));

    if (!memory_) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }

    memset(memory_, 0, static_cast<size_t>(size_));

    Block* initialBlock = static_cast<Block*>(memory_);
    initialBlock->size = size_;

    freeList_ = new Node{ initialBlock, nullptr };
    allocatedList_ = nullptr;

    pthread_mutex_init(&lock_, nullptr);
}

void MyAllocator::destroy() {
    pthread_mutex_lock(&lock_);

    free(memory_);
    memory_ = nullptr;

    Node* current = freeList_;
    while (current) {
        Node* next = current->next;
        delete current;
        current = next;
    }

    freeList_ = nullptr;

    current = allocatedList_;
    while (current) {
        Node* next = current->next;
        delete current;
        current = next;
    }

    allocatedList_ = nullptr;

    pthread_mutex_unlock(&lock_);
    pthread_mutex_destroy(&lock_);
}

void* MyAllocator::allocate(int size) {
    pthread_mutex_lock(&lock_);

    Node* prev = nullptr;
    Node* curr = freeList_;
    Node* bestPrev = nullptr;
    Node* bestFit = nullptr;
    size_t totalSize = size + sizeof(size_t);

    if (algorithm_ == BEST_FIT) {
        while (curr) {
            if (curr->block->size >= totalSize && (!bestFit || curr->block->size < bestFit->block->size)) {
                bestFit = curr;
                bestPrev = prev;
            }
            prev = curr;
            curr = curr->next;
        }
        curr = bestFit;
        prev = bestPrev;
    } else {
        while (curr && curr->block->size < totalSize) {
            prev = curr;
            curr = curr->next;
        }
    }

    if (curr) {
        Block* block = curr->block;

        if (block->size >= totalSize + sizeof(Block)) {
            Block* newBlock = reinterpret_cast<Block*>(reinterpret_cast<char*>(block) + totalSize);
            newBlock->size = block->size - totalSize;

            Node* newNode = new Node{ newBlock, nullptr };

            curr->block->size = totalSize;

            if (prev) {
                prev->next = newNode;
            } else {
                freeList_ = newNode;
            }
        } else {
            if (prev) {
                prev->next = curr->next;
            } else {
                freeList_ = curr->next;
            }
            curr->block->size = totalSize;
        }

        Node* temp = allocatedList_;
        if (!temp) {
            allocatedList_ = curr;
        } else {
            while (temp->next) {
                temp = temp->next;
            }
            temp->next = curr;
        }
        curr->next = nullptr;

        *reinterpret_cast<size_t*>(block) = totalSize;

        pthread_mutex_unlock(&lock_);
        return reinterpret_cast<char*>(block) + sizeof(size_t);
    }

    pthread_mutex_unlock(&lock_);
    return nullptr;
}

void MyAllocator::deallocate(void* ptr) {
    assert(ptr != nullptr);

    pthread_mutex_lock(&lock_);

    Block* block = reinterpret_cast<Block*>(reinterpret_cast<char*>(ptr) - sizeof(size_t));

    Node* newNode = new Node{ block, nullptr };

    if (!freeList_) {
        freeList_ = newNode;
    } else {
        Node* curr = freeList_;
        while (curr->next) {
            curr = curr->next;
        }
        curr->next = newNode;
    }

    Node* curr = allocatedList_;
    Node* prev = nullptr;

    while (curr && curr->block != block) {
        prev = curr;
        curr = curr->next;
    }

    if (curr) {
        if (prev) {
            prev->next = curr->next;
        } else {
            allocatedList_ = curr->next;
        }
        delete curr;
    }

    bool merged;
    do {
        merged = false;
        Node* current = freeList_;
        while (current) {
            Node* checker = current;
            while (checker->next) {
                if (reinterpret_cast<char*>(current->block) + current->block->size == reinterpret_cast<char*>(checker->next->block)) {
                    current->block->size += checker->next->block->size;
                    Node* temp = checker->next;
                    checker->next = checker->next->next;
                    delete temp;
                    merged = true;
                } else {
                    checker = checker->next;
                }
            }
            current = current->next;
        }
    } while (merged);

    pthread_mutex_unlock(&lock_);
}

int MyAllocator::compactAllocation(std::vector<void*>& before, std::vector<void*>& after) {
    pthread_mutex_lock(&lock_);

    int compactedSize = 0;
    size_t offset = 0;
    Node* current = allocatedList_;

    Node* temp;
    while (freeList_) {
        temp = freeList_;
        freeList_ = freeList_->next;
        delete temp;
    }

    while (current) {
        Node* next = current->next;
        Block* block = current->block;
        if (reinterpret_cast<char*>(block) != reinterpret_cast<char*>(memory_) + offset) {
            std::memmove(reinterpret_cast<char*>(memory_) + offset, block, block->size);
            before.push_back(reinterpret_cast<char*>(block) + sizeof(size_t));
            after.push_back(reinterpret_cast<char*>(memory_) + offset + sizeof(size_t));
            compactedSize++;
        }
        block = reinterpret_cast<Block*>(reinterpret_cast<char*>(memory_) + offset);
        current->block = block;
        offset += block->size;
        current = next;
    }

    if (offset < static_cast<size_t>(size_)) {
        freeList_ = new Node{ reinterpret_cast<Block*>(reinterpret_cast<char*>(memory_) + offset), nullptr };
        freeList_->block->size = size_ - offset;
    } else {
        freeList_ = nullptr;
    }

    pthread_mutex_unlock(&lock_);
    return compactedSize;
}

int MyAllocator::availableMemory() {
    pthread_mutex_lock(&lock_);

    int availableMemorySize = 0;
    Node* current = freeList_;
    while (current) {
        availableMemorySize += current->block->size;
        current = current->next;
    }

    pthread_mutex_unlock(&lock_);
    return availableMemorySize;
}

void MyAllocator::printStatistics() {
    pthread_mutex_lock(&lock_);

    int allocatedSize = 0;
    int allocatedChunks = 0;
    int freeSize = 0;
    int freeChunks = 0;
    int smallestFreeChunkSize = size_;
    int largestFreeChunkSize = 0;

    Node* current = allocatedList_;
    while (current) {
        allocatedSize += current->block->size;
        allocatedChunks++;
        current = current->next;
    }

    current = freeList_;
    while (current) {
        freeSize += current->block->size;
        if (current->block->size > largestFreeChunkSize) {
            largestFreeChunkSize = current->block->size;
        }
        if (current->block->size < smallestFreeChunkSize) {
            smallestFreeChunkSize = current->block->size;
        }
        freeChunks++;
        current = current->next;
    }

    printf("Allocated size = %d\n", allocatedSize);
    printf("Allocated chunks = %d\n", allocatedChunks);
    printf("Free size = %d\n", freeSize);
    printf("Free chunks = %d\n", freeChunks);
    printf("Largest free chunk size = %d\n", largestFreeChunkSize);
    printf("Smallest free chunk size = %d\n", smallestFreeChunkSize);

    pthread_mutex_unlock(&lock_);
}