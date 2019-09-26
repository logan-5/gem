# gem
A no-frills Gameboy emulator made of C++ and Python.

This is my first-ever emulator, so it was a mind-bending learning experience from start to finish. I don't claim that it's ultra-faithful to the hardware, or that it is bug-free. Heck, I don't even claim that there are _few_ bugs. I could spend days and months and years of my life tracking down every last corner case and piece of hardware minutiae, but I think my time and effort is better spent elsewhere. I've gotten what I wanted from this project, which was to learn an incredible amount.

Well, that and... from the very first day working on this project, my gamer goal was to fight the rival in Pallet Town in Pokemon Red/Blue (and win, of course).

![epic gamer moment](https://i.ibb.co/hDW3SxS/epic-gamer-moment.png)

It's difficult to describe the satisfaction of wrecking this clown on an emulator you built yourself.

# What's next
I'm not sure how much more I'll work on this, but there are a couple largeish areas where it's incomplete:
- Sound is currently not supported at all. If I do anything, I'll probably do this, since it'll be another opportunity to learn a ton.
- More complete support for different cartridge types: right now only 'basic' 32kb, MBC1, and MBC3 are supported.
- More complete handling of interrupts. Only VBlank and STAT are fully implemented right now. A couple games I've tried freeze while playing, and I suspect it has to do with expected interrupts not firing.
- Nitty gritty: passing all of the [Blargg tests](http://gbdev.gg8.se/files/roms/blargg-gb-tests/). A large percentage of the CPU tests pass, but I haven't tried many of the others, and I suspect they won't pass, since they're designed to thoroughly test subtle corner cases.
