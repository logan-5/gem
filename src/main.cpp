#include "cpu.hpp"
#include "fs.hpp"
#include "gpu.hpp"
#include "io.hpp"
#include "mem.hpp"
#include "rom.hpp"
#include "screen.hpp"

#include <iostream>

int main(int argc, const char* argv[]) {
    std::ios::sync_with_stdio(false);

    if (argc < 2) {
        GEM_LOG("please provide a ROM as a command line argument");
        std::exit(1);
    }
    const char* pathStr = argv[1];
    auto rom = gem::ROM::load(
          gem::fs::AbsolutePath{gem::fs::RelativePathView{pathStr}});
    if (!rom) {
        std::cerr << "couldn't load ROM file at '" << pathStr << "'\n";
        std::exit(1);
    }

    if (argc > 2) {
        std::vector<gem::u16> breakpoints;
        for (int i = 2; i < argc; ++i) {
            breakpoints.push_back(
                  static_cast<gem::u16>(std::strtol(argv[i], nullptr, 16)));
        }
        gem::op::pcBreakpoints = {std::move(breakpoints)};
    }

    gem::Window window;
    gem::Screen screen{window};
    gem::GPU gpu{screen};
    gem::IO io;
    gem::Mem mem{*std::move(rom), gpu, io};
    gem::CPU cpu{mem};
    while (window.isOpen()) {
        cpu.execute();
        gpu.step(cpu.getDeltaTicks());
        io.update();
    }
}
