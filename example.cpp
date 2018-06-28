#include "OptimisticLockCoupling/Tree.h"

#include <thread>
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

#define NTHREAD 10
#define NVALS 1000000
#define KEYSIZE 5

ART_OLC::Tree art;
std::string keys[NVALS];
unsigned vals[NVALS];

struct Element {
    std::string key;
    uintptr_t val;
};

void loadKey(TID tid, Key& key) {
    Element* e = (Element*) tid;
    key.set(e->key.c_str(), e->key.size());
}

std::vector<unsigned char> intToBytes(int paramInt)
{
    std::vector<unsigned char> arrayOfByte(4);
     for (int i = 0; i < 4; i++)
         arrayOfByte[3 - i] = (paramInt >> (i * 8));
     return arrayOfByte;
}

void insertKey(uint64_t k, int thread_id) {
    for (int i = thread_id*(NVALS/NTHREAD); i < (thread_id+1)*NVALS/NTHREAD; i++) {
        auto v = intToBytes(i);
        std::string str(v.begin(),v.end());

        Key art_key;
        art_key.set(str.c_str(), str.size());

        Element* e = new Element();
        e->key = str;
        e->val = i;

        art.insert(art_key, (TID) e, nullptr);
    }
}

void lookupKey(uint64_t k, int thread_id) {
    for (int i = thread_id*(NVALS/NTHREAD); i < (thread_id+1)*NVALS/NTHREAD; i++) {
        auto v = intToBytes(i);
        std::string str(v.begin(),v.end());
        Key art_key;
        art_key.set(str.c_str(), str.size());
        Element* e = (Element*) art.lookup(art_key);
        assert(e->val == i);
    }
}


int main() {
    uint64_t* keys = new uint64_t[NVALS];
    for (uint64_t i = 0; i < NVALS; i++)
        // dense, sorted
        keys[i] = i + 1;

    art.setLoadKey(loadKey);

    // Build tree
    {
        auto starttime = std::chrono::system_clock::now();
        std::thread threads[NTHREAD];
        for (int i = 0; i < NTHREAD; i++) {
            threads[i] = std::thread(insertKey, keys[i], i);
        }

        for (int i = 0; i < NTHREAD; i++) {
            threads[i].join();
        }
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now() - starttime);
        printf("insert,%ld,%f\n", NVALS, (NVALS * 1.0) / duration.count());
    }

    {
        auto starttime = std::chrono::system_clock::now();
        std::thread threads[NTHREAD];
        for (int i = 0; i < NTHREAD; i++) {
            threads[i] = std::thread(lookupKey, keys[i], i);
        }

        for (int i = 0; i < NTHREAD; i++) {
            threads[i].join();
        }
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now() - starttime);
        printf("lookup,%ld,%f\n", NVALS, (NVALS * 1.0) / duration.count());
    }
}
