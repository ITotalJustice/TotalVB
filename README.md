# TotalVB

WIP Virtual Boy emulator, this is going to take me a while, check back later!

---

## Not fininshed

list of stuff that is not yet finished (no games will run yet).

### v810 (cpu)
- all floating point instructions
- all bit string instructions
- interrupts
- exceptions
- div
- correct cycle timing per instruction

### vip (graphics)
- not started

### vsu (audio)
- channel 5 sweep
- noise

### timer
- not started

---

## Building

the build system used is meson

```bash
meson -C builddir
meson compile -C builddir
```

you can of course use the compiler directly like so

```bash
gcc src/main.c src/core/*.c # compiles all source files
```

---

## Credits

One of my fave console fan sites, so much amazing stuff in one place!

- <https://www.virtual-boy.com/>

Probably the best docs i have ever read.

- <https://www.virtual-boy.com/downloads/978651/get/1044877>
