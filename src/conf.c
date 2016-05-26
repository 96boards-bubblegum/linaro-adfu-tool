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


#include <stdio.h>
#include <stdlib.h>
#include <libconfig.h>
#include "conf.h"

#define ADFU_CONF_ERR(name)  do {fprintf(stderr, "No '%s' in config file\n", name); goto err; } while(0)

int adfu_parse_conf(struct adfu_conf *conf, char *file)
{
  config_t cfg;

  config_init(&cfg);

  /* Read the file. If there is an error, report it and exit. */
  if(! config_read_file(&cfg, file))
  {
    fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
            config_error_line(&cfg), config_error_text(&cfg));
    config_destroy(&cfg);
    return -1;
  }

  /* Get the store name. */
  if(!config_lookup_string(&cfg, "uboot", &conf->uboot))
    ADFU_CONF_ERR("uboot");

  fprintf(stdout, "  uboot: %s\n", conf->uboot);

  if(!config_lookup_int(&cfg, "boot_kernel_enable", &conf->boot_kernel_enable))
    ADFU_CONF_ERR("boot_kernel_enable");

  if(conf->boot_kernel_enable == 1) 
  {
    if(!config_lookup_string(&cfg, "kernel", &conf->kernel))
      ADFU_CONF_ERR("kernel");

    if(!config_lookup_string(&cfg, "dtb", &conf->dtb))
      ADFU_CONF_ERR("dtb");

    if(!config_lookup_string(&cfg, "ramdisk", &conf->ramdisk))
      ADFU_CONF_ERR("ramdisk");

    fprintf(stdout, "\n  boot_kernel_enable: %s\n",
      conf->boot_kernel_enable ? "enable" : "disable");

    fprintf(stdout, "  kernel: %s\n  dtb: %s\n  ramdisk: %s\n\n",
      conf->kernel, conf->dtb, conf->ramdisk);

  }



  return 0;
err:
    config_destroy(&cfg);
    return -1;
}
