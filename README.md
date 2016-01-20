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
