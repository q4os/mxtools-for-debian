#include "aptcache.h"
#include "../Timer.h"
#include <iostream>

int main() {
    std::cout << "=== APT Cache Performance Test ===" << std::endl;

    for (int i = 0; i < 3; ++i) {
        std::cout << "--- Run " << (i + 1) << " ---" << std::endl;
        ScopedTimer totalTimer("Total test time");

        AptCache cache;
        auto candidates = cache.getCandidates();

        std::cout << "Found " << candidates.size() << " packages" << std::endl;
    }

    return 0;
}