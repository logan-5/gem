#include "cpu.hpp"

int main() {
    gem::CPU cpu;
    while (true) {
        cpu.execute();
    }
}