/*
   Linaro ADFU tools for Linux
   Copyright (C) 2016 Yixun Lan <dlan@gentoo.org>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

struct adfu_conf {
  const char *uboot;

  const char *kernel;
  const char *dtb;
  const char *ramdisk;

  int boot_kernel_enable;
};

int adfu_parse_conf(struct adfu_conf *conf, char *file);
