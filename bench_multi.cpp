#include "OptimisticLockCoupling/Tree.h"

#include <thread>
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>
#include <random>

#define NTHREAD 1
#define NVALS 100000

#define RAND 1

ART_OLC::Tree art;

uint64_t* keys;

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
        auto v = intToBytes(keys[i]);
        std::string str(v.begin(),v.end());

        Key art_key;
        art_key.set(str.c_str(), str.size());

        Element* e = new Element();
        e->key = str;
        e->val = keys[i];

        art.insert(art_key, (TID) e, nullptr);
    }
}

void lookupKey(int thread_id) {
    for (int i = thread_id*(NVALS/NTHREAD); i < (thread_id+1)*NVALS/NTHREAD; i++) {
        auto v = intToBytes(keys[i]);
        std::string str(v.begin(),v.end());
        Key art_key;
        art_key.set(str.c_str(), str.size());
        Element* e = (Element*) art.lookup(art_key);
        assert(e->val == keys[i]);
    }
}

void eraseKey(int thread_id) {
    for (int i = thread_id*(NVALS/NTHREAD); i < (thread_id+1)*NVALS/NTHREAD; i++) {
        auto v = intToBytes(keys[i]);
        std::string str(v.begin(),v.end());
        Key art_key;
        art_key.set(str.c_str(), str.size());
        Element* e = (Element*) art.lookup(art_key);
        art.remove(art_key, (TID) e);
    }
}

int main() {
    art.setLoadKey(loadKey);

    keys = new uint64_t[NVALS];

    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0,(unsigned) -1);

    for (uint64_t i = 0; i < NVALS; i++) {
#ifdef RAND
        keys[i] = dist(rng);
#else
        keys[i] = i;
#endif
    }

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
