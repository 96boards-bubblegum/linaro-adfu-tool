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

 1. Connect the serial console

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
    setenv uuid_gpt_disk e9d6dc79-fad4-48a0-a38c-f1817663e943
    setenv uuid_uboot 79080809-6905-4b7c-bb72-ad67a590e184
    setenv uuid_boot 4e38916b-e416-478e-95d3-f5759b67e83b
    setenv uuid_system 6a65fe5e-0c68-4396-a2ba-291a919c30f0
    setenv uuid_swap 99f7ba03-3a0b-461b-bba1-3e3a963c81c8
    setenv uuid_boot_msg 1cd1f387-29fd-43b7-80a0-593c5e23e7c4
    setenv partitions "uuid_disk=${uuid_gpt_disk};name=UBOOT,size=8MiB,uuid=${uuid_uboot};name=BOOT,size=50MiB,uuid=${uuid_boot};name=SYSTEM,size=6500MiB,uuid=${uuid_system};name=SWAP,size=768MiB,uuid=${uuid_swap};name=BOOT_MSG,size=-,uuid=${uuid_boot_msg};"
    gpt write mmc 1 ${partitions}
 
 7. Run “fastboot usb” in serial console to entering the fastboot mode.

 8. Run “fastboot flash BOOT boot.emmc.img” on host computer.

 9. Run “fastboot flash SYSTEM bubblegum-jessie_*.emmc.img” on host computer.

 10. Redo the step 3 to 5.

 11. Run “fatload mmc 1:2 ${scriptaddr} boot.scr; source ${scriptaddr}”

 12. You should be able to login into Debian.

 13. "sudo dd if=bootloader.bin of=/dev/mmcblk0 seek=4097 bs=512"
     bootloader.bin can be found in the de-brick tool.

 14. "sudo dd if=u-boot-dtb.img of=/dev/mmcblk0 seek=6144 bs=512"

 15. Reboot.
