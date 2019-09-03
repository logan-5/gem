#include "cpu.hpp"
#include "fs.hpp"
#include "gpu.hpp"
#include "mem.hpp"
#include "rom.hpp"

#include <iostream>

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        std::exit(1);
    }
    const char* pathStr = argv[1];
    auto rom = gem::ROM::load(
          gem::fs::AbsolutePath{gem::fs::RelativePathView{pathStr}});
    if (!rom) {
        std::cerr << "couldn't load ROM file at '" << pathStr << "'\n";
        std::exit(1);
    }

    gem::GPU gpu;
    gem::Mem mem{*std::move(rom), gpu};
    gem::CPU cpu{mem};
    while (true) {
        cpu.execute();
    }
}