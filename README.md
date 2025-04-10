# cmpt432-implementation-team-Alpha

# How to Build and Run

## 1. Requirements
- ARM toolchain (`arm-none-eabi-gcc`, `arm-none-eabi-ld`, `arm-none-eabi-objcopy`, etc.)
- `make`
- Optional: `mtools` (for rootless SD image creation)

---

## 2. Build `sd.img`
```bash
sudo make
```

This will generate `build/sd.img`.  
Copy it to your SD card manually using your preferred method (e.g., `dd` or `balenaEtcher`).

Then insert the SD card into the BeagleBone Black and boot.
⚠️ if you get a `PANIC: Failed to mount FAT32 filesystem: Invalid boot sector`
   during boot, make again and reflash the sd card.

---

## 3. Build and Flash to SD Card (Preferred)
```bash
sudo make flash DEV=/path/to/sdcard
```

> Replace `/path/to/sdcard` with the correct device (e.g., `/dev/sdb`).  
> ⚠️ **Be careful** to select the right device — this will overwrite it!

Once flashed, insert the SD card into the BBB and power it on.

⚠️ if you get a `PANIC: Failed to mount FAT32 filesystem: Invalid boot sector`
   during boot, make again and reflash the sd card.