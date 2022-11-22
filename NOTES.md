# Notes

i've been making a little progress on my vb emulator. as there's no bios, games have to setup everything themselves manually - which isn't really a bad thing. i thought it would be cool to note down all the init code of all 12 games for the system, like note every single step and explain what and why its doing that.

i'll also talk about other vb related things here. basically, it'll be like a blog or my emudev experience. maybe i'll do a proper blog writeup once this emu starts running stuff.

---

## bugs

a bug that took a while to track down was in `mario's tennis`. i copied the sub/cmp code from my gba emu, specifically the flags. my bug was caused by the carry flag https://github.com/ITotalJustice/notorious_beeg/blob/83d5949755317ea3fab2f53ef48c4fdc0e684776/src/core/arm7tdmi/helper.hpp#L61. so i remember doing `carry = b > a` for gba, but this was wrong. after trial and error, i got it to `a >= b`, idk why that is.  the bug was that the v810 works as i first thought, not how the arm works. this is also how gcc's `__builtin_sub_overflow` works as well, so i can use that

---

## UB detection within games

i was thinking about adding a way to detect "UB" within games. this will be useful for homebrew.

something simple i though of is reading from memory that has not yet been written to. ie, reading from vram before ever writing to it. same for registers etc.

as there's no bios, memory and registers are not init on startup. although maybe ram is set to predefined values on init (or even be all zeros), in which case, this may be pointless.

i thought about this due to 3D-tetris randomly doing 8 32-bit reads from vram before ever writing to it.

---

## Bitfield union abuse

the params for instructions are broken down into formats. there's 7 in total. these are a bitfield of the 16-bit opcode.
to simplify code bloat and possibly better code gen, i could use union / struct to have it auto parse the bits.

```c
// note: these won't be in the correct order and fields are missing.
// this is just to give a quick example.
union Opcode {
  struct Format1 {
    uint8_t reg1 : 5;
    uint8_t reg2 : 5;
    uint8_t opcode : 6;
  } f1;

  struct Format2 {
    uint8_t opcode : 6;
    uint8_t reg2 : 5;
    uint8_t lo5 : 5;
  } f2;

  struct Format3 {
    uint8_t opcode : 3;
    uint8_t cond : 4;
    uint16_t disp : 9;
  } f3;

  struct Format4 {
    uint8_t opcode : 6;
    uint16_t disp : 9;
  } f4;

  struct Format5 {
    uint8_t opcode: 6;
    uint8_t reg2: 5;
    uint8_t reg1: 5;
  } f5;

  struct Format6 {
    uint8_t opcode: 6;
    uint8_t reg2: 5;
    uint8_t reg1: 5;
  } f6;

  uint16_t raw;
};

void AND(Opcode op) {
  uint32_t result = REGISTERS[op.f1.reg2] & REGISTERS[op.f1.reg1];
  // ...
}

void execute(Opcode op) {
  switch (op.raw) {
    // ...
  }
}

void fetch() {
  Opcode op;
  op.raw = READ16(REG_PC);
  REG_PC += 2;

  execute(op);
}

```

---

## timing

i haven't yet implemented timing, however, i will just assume
that all instructions take 4 cycles for now, which is likely too many.

instructions vary from 1 cycle to 20 cycles. there's also rom waitstates and instruction cache that needs to be emulated.

---

## Jack Bros

the game seems to memset ALL memory on init. this includes wram, vsu and vip - in that order.

for the vsu, it first writes a one to `SSTOP` to disable all channels and then it zeros all waveram and vsu registers.

this is normal, however it also writes a 0 to `SSTOP` at the end. this makes no sense as only writing a 1 to `SSTOP` does anything at all. maybe this was a memset16 done, and they just set the size for the full range of the vsu.

for vip, it zeros the area `[0x0] - [0x0005DFFF]`

next, it then writes `0x03` to `WCR [0x02000024]`, which means that `ROM` access is now 1 waitstate(s) and `EXP` (game expansion pack) is 1 waitstate(s).

next, it reads from the vip `DPSTTS`, which is the status control register.

next, it writes `0x0101` to `DPCTRL`, this sets `DPRST` and `RE`.

next, it writes `0x0` to `INTENB`, which disables all interrupts for the vip.

next, it writes `0xE01F` to `INTCLR`, which i think clears all pending interrupts, apart from `TIMEERR`. `TIMEERR` is set when drawing is currently in progress (which is all the time?).

next, it writes `0x1` to `XPCTRL`, which resets all drawing actions.

---

## Mario's Tennis

(init is similar to jack bros)

for vip, it zeros the area `[0x0] - [0x0005DFFF]`

next, it reads from the vip `DPSTTS`, which is the status control register.

next, it writes `0x0101` to `DPCTRL`, this sets `DPRST` and `RE`.

next, it writes `0x0` to `INTENB`, which disables all interrupts for the vip.

next, it writes `0xE01F` to `INTCLR`, which i think clears all pending interrupts, apart from `TIMEERR`. `TIMEERR` is set when drawing is currently in progress (which is all the time?).

next, it writes `0x1` to `XPCTRL`, which resets all drawing actions.

next, it writes `0x1` to `FRMCYC`, which...i dont understand this register. TODO:!

next, it does an 8-bit of `0x28` write to `0x0000000D` which is the vip `Left frame buffer 0`. ~~8-bit writes to the vip do some strange stuff says the scroll.~~ the above only applies to vip io writes!

next, another 8-bit write `0x30` to `0x0000002A` which is again the `Left frame buffer 0`.

next, it writes `0x000C` to `BRTA` which is the brightness control register. this...todo:!

---

## Mario Clash

3 instrucions in and it does a LDSR with the value `0x0` to `PSW`.

next, calls SEI to enable interrupts.

next, it writes `0x0` to `INTENB`, which disables all interrupts for the vip.

next, it writes `0x0` to `CCR` which is the link registers, this doesn't really matter much.

next, it writes `0x80` to `CCSR` which disables the link interrupt.

next, it writes `0x80` to `SCR` which disables the key input interrupt.

next, it then writes `0x01` to `WCR [0x02000024]`, which means that `ROM` access is now 1 waitstate(s) and `EXP` (game expansion pack) is 2 waitstate(s).

next, it seems to setup waveram 1-5 and modulation ram with real values (non-zero).

---

## Teleroboxer

seems to be the exact same init code as `mario clash`
the waveram and modulation ram is the exact same as well!

3 instrucions in and it does a LDSR with the value `0x0` to `PSW`.

next, calls SEI to enable interrupts.

next, it writes `0x0` to `INTENB`, which disables all interrupts for the vip.

next, it writes `0x0` to `CCR` which is the link registers, this doesn't really matter much.

next, it writes `0x80` to `CCSR` which disables the link interrupt.

next, it writes `0x80` to `SCR` which disables the key input interrupt.

next, it then writes `0x01` to `WCR [0x02000024]`, which means that `ROM` access is now 1 waitstate(s) and `EXP` (game expansion pack) is 2 waitstate(s).

next, it seems to setup waveram 1-5 and modulation ram with real values (non-zero).

---

## 3D Tetris

doesn't seem to memset vram or audio.

for whatever reason, it does 8 consecutive 32bit reads via `IN_W`. reg1 will be `0x00008000` and the disp is `-32768`, so the addr is 0. the game does these random reads before it even writes anything to vip, so i have no clue what's going on.

next, `0x0` to `PSW` via `LDSR`.

next, it writes `0x2` to `CHCW` via `LDSR` which enables cache (not emulated).

next, it writes `0x0` to `TCR` which is the timer control register.

next, it disables interrupts with `CLI`, again very odd as interrupts are already disabled due to `LDSR` to `PSW`...

next, it writes valid data to waveram 1-5, mainly 3-5 (rest are `0x00` or `0x3F`). due to this, i assume that all channels are disabled on boot, which makes sense otherwise there would be awful noise on booting the system.

next, it writes `0x00` to `S1INT`.

next, it writes `0x00` to `S2INT`.

next, it writes `0x00` to `S3INT`.

next, it writes `0x00` to `S4INT`.

next, it writes `0x00` to `S5INT`.

next, it writes `0x00` to `S6INT`.

next, it writes `0x00` to `SSTOP` which is odd, as this does nothing at all.

next, it writes `0x00` to `S1RAM`.

next, it writes `0x00` to `S2RAM`.

next, it writes `0x00` to `S3RAM`.

next, it writes `0x00` to `S4RAM`.

next, it writes `0x00` to `S5RAM`.

next, it writes `0x00` to `S1EV0`.

next, it writes `0x00` to `S2EV0`.

next, it writes `0x00` to `S3EV0`.

next, it writes `0x00` to `S4EV0`.

next, it writes `0x00` to `S5EV0`.

next, it writes `0x00` to `S6EV0`.

next, it writes `0x00` to `S1EV1`.

next, it writes `0x00` to `S2EV1`.

next, it writes `0x00` to `S3EV1`.

next, it writes `0x00` to `S4EV1`.

next, it writes `0x00` to `S5EV1`.

next, it writes `0x00` to `S6EV1`.
