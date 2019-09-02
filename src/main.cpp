#include "cpu.hpp"
#include "mem.hpp"

int main() {
    gem::Mem mem;
    gem::CPU cpu{mem};
    while (true) {
        cpu.execute();
    }
}