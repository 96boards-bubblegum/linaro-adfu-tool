# Linaro ADFU Tools for flashing Bubblegum-96
==============================================

## Build

cmake .
make

## Things you should have before running this tool

 1. You should have a u-boot image "u-boot-dtb.img" from
    http://builds.96boards.org/snapshots/bubblegum/linaro/u-boot/latest/
 2. You should have a boot img "boot.emmc.img" from
    http://builds.96boards.org/snapshots/bubblegum/linaro/debian/latest/
 3. You should have a Debian rootfs "bubblegum-jessie_*.emmc.img" from
    http://builds.96boards.org/snapshots/bubblegum/linaro/debian/latest/

## How to de-brick Bubblegum-96

 1. Connect the serial console. Baud rate is 115200,8n1.

 2. Connect USB cable.

 3. Don’t connect the power yet.
    Push the ADFU button and then connect the power.
    Wait for 5 secs.
    Release the button.
    Use "lsusb", you should find a device 10d6:10d6

 4. "sudo ./src/linaro-adfu-tool-bg96 u-boot-dtb.img"

 5. When seeing u-boot starts running on serial console, press enter to
    break it and entering the u-boot shell.

 6. Use the following command to re-construct the gpt table:
    setenv partitions "name=BOOTLOADER,size=8MiB;name=BOOT,size=50MiB;name=SYSTEM,size=6500MiB;name=SWAP,size=768MiB;name=BOOT_MSG,size=-;"
    gpt write mmc 1 ${partitions}
 
 7. Run “fastboot usb” in serial console to entering the fastboot mode.

 8. Run “fastboot flash BOOT boot.emmc.img” on host computer.

 9. Run “fastboot flash SYSTEM bubblegum-jessie_*.emmc.img” on host computer.

 10. Redo the step 3 to 5.

 11. Run “setenv bootpart 2; boot”

 12. You should be able to login into Debian.

 13. "sudo dd if=bootloader.bin of=/dev/mmcblk0 seek=4097 bs=512"
     bootloader.bin can be found in the de-brick tool.

 14. "sudo dd if=u-boot-dtb.img of=/dev/mmcblk0 seek=6144 bs=512"

 15. Reboot.

 De-brick done.

## TODO

 This section contains some TODO or ideas for debrick.

### Flashing back to Actions' firmware

 If you want to flash back Actions' firmware instead of Linaro's image. You
 can first use tools/extract_actions_fw.py to de-compose Actions' firmare to
 fastboot images.

 For Actions' Android image. You should alter the gpt tables by:
    setenv partitions "name=MISC,size=48MiB,start=8MiB;name=RECOVERY,size=48MiB;name=SYSTEM,size=1560MiB;name=BOOT_MSG,size=1MiB;name=DATA,size=2048MiB;name=CACHE,size=512MiB;name=DATA_BAK,size=1MiB;name=VENDOR_APP,size=1MiB;name=MNT_MEDIA,size=1MiB;name=SWAP,size=1MiB;name=UDISK,size=-;"
    gpt write mmc 1 ${partitions}
 And then flash MISC, RECOVERY, and SYSTEM partitions by fastboot.

 For Actions' Debian image. You should alter the gpt tables by:
    setenv partitions "name=MISC,size=50MiB,start=8MiB;name=SYSTEM,size=6500MiB;name=SWAP,size=768MiB;name=BOOT_MSG,size=-;"
    gpt write mmc 1 ${partitions}
 And then flash MISC and SYSTEM partitions by fastboot.

### Directly write u-boot from u-boot.

 We can use XMODEM to transfer u-boot-dtb.img to memory. But currently
 u-boot crashed on mmc commands. Thus doesn't work.

 1. "loadx 0x10000000"
 2. Send bootloader.bin through XMODEM protocol.
 3. "mmc dev 1; mmc write 0x10000000 4097 2048"
 4. "loadx 0x10000000"
 5. Send u-boot-dtb.img through XMODEM protocol.
 6. "mmc dev 1; mmc write 0x10000000 6144 2048"

### Create bootloader.img and use fastboot flash bootloader.bin and u-boot.

 We might be able to flash the bootloader by fastboot. But "BOOTLOADER"
 partition needs to be created first.

 The following commands runs on PC:

 1. "dd if=/dev/zero of=bootloader.img bs=1M count=6"
    Creates a 6MiB empty image.
 2. "dd conv=notrunc if=bootloader.bin of=bootloader.img seek=4063 bs=512"
    Place bootloader.bin to correct place.
 3. "dd conv=notrunc if=u-boot-dtb.img of=bootloader.img seek=6110 bs=512"
    Place u-boot-dtb.img to correct place.
 4. "fastboot flash BOOTLOADER bootloader.img"
    Use fastboot to flash BOOTLOADER partition.

 Maybe we can use CI to generate such image automatically?
