#include "OptimisticLockCoupling/Tree.h"

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

int main() {
    art.setLoadKey(loadKey);

    // Build tree
    {
        auto starttime = std::chrono::system_clock::now();
        for (int i = 0; i < NVALS; i++) {
            auto v = intToBytes(i);
            std::string str(v.begin(),v.end());

            Key art_key;
            art_key.set(str.c_str(), str.size());

            Element* e = new Element();
            e->key = str;
            e->val = i;

            art.insert(art_key, (TID) e, nullptr);
        }

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now() - starttime);
        printf("insert,%ld,%f\n", NVALS, (NVALS * 1.0) / duration.count());
    }

    {
        auto starttime = std::chrono::system_clock::now();
        for (int i = 0; i < NVALS; i++) {
            auto v = intToBytes(i);
            std::string str(v.begin(),v.end());
            Key art_key;
            art_key.set(str.c_str(), str.size());
            Element* e = (Element*) art.lookup(art_key);
            assert(e->val == i);
        }

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now() - starttime);
        printf("lookup,%ld,%f\n", NVALS, (NVALS * 1.0) / duration.count());
    }
}
