#include "NumPy.hpp"

NumPy::NumPy(): pShareMem(nullptr) {

}

NumPy::~NumPy() {

}

int NumPy::init() {
    if (nullptr == pShareMem) {
        pShareMem = new ShareMem();
    }
    //
    pShareMem->Init();
    pShareMem->Create(0);
    pShareMem->Create(1);
    pShareMem->Create(2);
    return 0;
}

int NumPy::final() {
    if (nullptr != pShareMem) {
        delete pShareMem;
        pShareMem = nullptr;
    }
    //
    return 0;
}

int NumPy::testing() {
    if (nullptr == pShareMem) {
        cerr << "Invalid pointer!";
        return -1;
    }
    //
    // pShareMem->TestConfig();
    //
    std::cout << "----------- @Preflop -----------" << std::endl;
    pShareMem->TestCard("2s5s", true);
    std::cout << "----------- @Flop -----------" << std::endl;
    pShareMem->TestCard("2s5s6s8sTs", true);
    pShareMem->TestCard("2s3s6s9sTs", true);
    std::cout << "----------- @Turn -----------" << std::endl;
    pShareMem->TestCard("2s5s6s8sTsKs", true);
    std::cout << "----------- @River -----------" << std::endl;
    pShareMem->TestCard("2s5s6s8sTsKsAs", true);
    return 0;
}

ShareMem *NumPy::ShmPointer() {
    return pShareMem;
}
