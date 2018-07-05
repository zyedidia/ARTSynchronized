#include "OptimisticLockCoupling/Tree.h"

#include <thread>
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

#define NTHREAD 10
#define NVALS 10000000

ART_OLC::Tree art;

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

void insertKey(int thread_id) {
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

void lookupKey(int thread_id) {
    for (int i = thread_id*(NVALS/NTHREAD); i < (thread_id+1)*NVALS/NTHREAD; i++) {
        auto v = intToBytes(i);
        std::string str(v.begin(),v.end());
        Key art_key;
        art_key.set(str.c_str(), str.size());
        Element* e = (Element*) art.lookup(art_key);
        assert(e->val == i);
    }
}

void eraseKey(int thread_id) {
    for (int i = thread_id*(NVALS/NTHREAD); i < (thread_id+1)*NVALS/NTHREAD; i++) {
        auto v = intToBytes(i);
        std::string str(v.begin(),v.end());
        Key art_key;
        art_key.set(str.c_str(), str.size());
        Element* e = (Element*) art.lookup(art_key);
        art.remove(art_key, (TID) e);
    }
}

int main() {
    art.setLoadKey(loadKey);

    // Build tree
    {
        auto starttime = std::chrono::system_clock::now();
        std::thread threads[NTHREAD];
        for (int i = 0; i < NTHREAD; i++) {
            threads[i] = std::thread(insertKey, i);
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
            threads[i] = std::thread(lookupKey, i);
        }

        for (int i = 0; i < NTHREAD; i++) {
            threads[i].join();
        }
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now() - starttime);
        printf("lookup,%ld,%f\n", NVALS, (NVALS * 1.0) / duration.count());
    }

    {
        auto starttime = std::chrono::system_clock::now();
        std::thread threads[NTHREAD];
        for (int i = 0; i < NTHREAD; i++) {
            threads[i] = std::thread(eraseKey, i);
        }

        for (int i = 0; i < NTHREAD; i++) {
            threads[i].join();
        }
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now() - starttime);
        printf("erase,%ld,%f\n", NVALS, (NVALS * 1.0) / duration.count());
    }
}
