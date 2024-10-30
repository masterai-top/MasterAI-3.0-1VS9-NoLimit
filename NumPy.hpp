#ifndef __CFR_NUMPY_H_
#define __CFR_NUMPY_H_

#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <filesystem>
#include "Single.hpp"
#include "ShareMem.hpp"

using namespace std;

class NumPy : public Singleton<NumPy> {
public:
    //
    NumPy();
    //
    ~NumPy();
    //
    int init();
    //
    int final();
    //
    int testing();
    //
    ShareMem *ShmPointer();

private:
    //
    ShareMem *pShareMem;
};


#endif

