#ifndef PLURIBUS_HASH_H
#define PLURIBUS_HASH_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>

using namespace std;

namespace common {
    //
    uint64_t Hash(const void *buf, size_t len);
}

#endif //PLURIBUS_HASH_H

