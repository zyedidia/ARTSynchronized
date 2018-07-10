#include "ROWEX/Tree.h"

#include <thread>
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>
#include <random>

int nthread = 1;
bool rand_keys = 0;

#define NVALS 10000000

ART_ROWEX::Tree art;

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
    for (int i = thread_id*(NVALS/nthread); i < (thread_id+1)*NVALS/nthread; i++) {
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
    for (int i = thread_id*(NVALS/nthread); i < (thread_id+1)*NVALS/nthread; i++) {
        auto v = intToBytes(keys[i]);
        std::string str(v.begin(),v.end());
        Key art_key;
        art_key.set(str.c_str(), str.size());
        Element* e = (Element*) art.lookup(art_key);
        assert(e->val == keys[i]);
    }
}

void eraseKey(int thread_id) {
    for (int i = thread_id*(NVALS/nthread); i < (thread_id+1)*NVALS/nthread; i++) {
        auto v = intToBytes(keys[i]);
        std::string str(v.begin(),v.end());
        Key art_key;
        art_key.set(str.c_str(), str.size());
        Element* e = (Element*) art.lookup(art_key);
        art.remove(art_key, (TID) e);
    }
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        nthread = atoi(argv[1]);
    }
    if (argc > 2) {
        rand_keys = atoi(argv[2]);
    }

    art.setLoadKey(loadKey);

    keys = new uint64_t[NVALS];

    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0,(unsigned) -1);

    for (uint64_t i = 0; i < NVALS; i++) {
        if (rand_keys) {
            keys[i] = dist(rng);
        } else {
            keys[i] = i;
        }
    }

    // Build tree
    {
        auto starttime = std::chrono::system_clock::now();
        std::thread threads[nthread];
        for (int i = 0; i < nthread; i++) {
            threads[i] = std::thread(insertKey, i);
        }

        for (int i = 0; i < nthread; i++) {
            threads[i].join();
        }
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now() - starttime);
        printf("insert,%ld,%f\n", NVALS, (NVALS * 1.0) / duration.count());
    }

    {
        auto starttime = std::chrono::system_clock::now();
        std::thread threads[nthread];
        for (int i = 0; i < nthread; i++) {
            threads[i] = std::thread(lookupKey, i);
        }

        for (int i = 0; i < nthread; i++) {
            threads[i].join();
        }
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now() - starttime);
        printf("lookup,%ld,%f\n", NVALS, (NVALS * 1.0) / duration.count());
    }

    {
        auto starttime = std::chrono::system_clock::now();
        std::thread threads[nthread];
        for (int i = 0; i < nthread; i++) {
            threads[i] = std::thread(eraseKey, i);
        }

        for (int i = 0; i < nthread; i++) {
            threads[i].join();
        }
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now() - starttime);
        printf("erase,%ld,%f\n", NVALS, (NVALS * 1.0) / duration.count());
    }
}
