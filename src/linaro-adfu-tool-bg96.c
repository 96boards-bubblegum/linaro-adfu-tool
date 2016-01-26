/* 
   Linaro ADFU tools for Linux
   Copyright (C) 2015 Ying-Chun Liu (PaulLiu) <paul.liu@linaro.org>

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
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include <unistd.h>
#include <error.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <libusb.h>

libusb_context *libusb_ctx=NULL;

/**
 * Init libusb
 *
 * @return always 0
 */
int b96_init_usb(void) {
  libusb_init(&libusb_ctx);
  libusb_set_debug(libusb_ctx,3);
  return 0;
}

int b96_uninit_usb(void) {
  libusb_exit(libusb_ctx);
  libusb_ctx=NULL;
  return 0;
}

/**
 * Open the device and init it
 * You should call b96_init_usb() first before using this function
 *
 * @return the handler of the device
 */
libusb_device_handle* b96_init_device(void) {
  libusb_device_handle *handler=NULL;
  int r1;
  int c1;
  struct libusb_device_descriptor desc;
  char buf[1024];
  int i;

  handler = libusb_open_device_with_vid_pid(libusb_ctx, 0x10d6, 0x10d6);
  if (handler == NULL) {
    error_at_line(0,0,__FILE__,__LINE__,"Error: cannot open device 10d6:10d6");
    return handler;
  }

  r1 = libusb_get_device_descriptor(libusb_get_device(handler), &desc);
  if (r1 != 0) {
    error_at_line(0,0,__FILE__,__LINE__,"Error: cannot get device descriptor");
  }
  printf ("bDescriptorType: %d\n", desc.bDescriptorType);
  printf ("bNumConfigurations: %d\n", desc.bNumConfigurations);
  printf ("iManufacturer: %d\n", desc.iManufacturer);

  for (i=1; i<=desc.bNumConfigurations; i++) {
    struct libusb_config_descriptor *config=NULL;
    int r2=0;
    int j;
    r2 = libusb_get_config_descriptor_by_value(libusb_get_device(handler), i, &config);
    if (r2 != 0) {
      error_at_line(0,0,__FILE__,__LINE__,"Error: cannot get configuration %d descriptor", i);
      continue;
    }
    printf ("bNumInterfaces: %d\n", config->bNumInterfaces);
    for (j=0; j<config->bNumInterfaces; j++) {
      
    }
    if (config != NULL) {
      libusb_free_config_descriptor(config);
      config=NULL;
    }
  }

  r1 = libusb_detach_kernel_driver(handler, 0);
  if (r1 != 0) {
    error_at_line(0,0,__FILE__,__LINE__,"Error: cannot detach kernel driver");
  }
  
  c1 = 0;
  r1 = libusb_get_configuration(handler,  &c1);
  if (r1 != 0) {
    error_at_line(0,0,__FILE__,__LINE__,"Error: cannot get device configuration");
  }
  printf ("Configuiration: %d\n",c1);
  r1 = libusb_set_configuration(handler, c1);
  if (r1 != 0) {
    error_at_line(0,0,__FILE__,__LINE__,"Error: cannot set device configuration");
  }

  r1 = libusb_claim_interface(handler, 0);
  if (r1 != 0) {
    error_at_line(0,0,__FILE__,__LINE__,"Error: cannot claim device interface");
  }
  
  return handler;
}

void b96_uninit_device(libusb_device_handle *handler) {
  int r1;
  r1 = libusb_release_interface(handler, 0);
  if (r1 != 0) {
    error_at_line(0,0,__FILE__,__LINE__,"Error: cannot release device interface");
  }
  libusb_close(handler);
}

void resetBulkOnlyMassStorage(libusb_device_handle *handler) {
  int r1;
  r1 = libusb_control_transfer(handler,
			  LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE,
			  '\xff',
			  0,
			  1,
			  NULL,
			  0,
			  1000);
  if (r1 != 0) {
    error_at_line(0,0,__FILE__,__LINE__,"Error: cannot reset device usbms");
  }
}

void readCSW(libusb_device_handle *handler) {
  int r1;
  unsigned char buf1[1024];
  int transferred=0;
  int i;

  int state = 1;

  while (state != 0) {
    
    switch(state) {
    case 1:
      r1 = libusb_bulk_transfer(handler, 0x82, buf1, 1024, &transferred, 3000);
      if (r1==0) {
	printf ("CSW:");
	for (i=0; i<transferred; i++) {
	  printf (" %02x",buf1[i]);
	}
	printf ("\n");
      }
      state = 2;
      break;
      
    case 2:
      if (r1 == 0) {
	state = 3;
      } else {
	state = 5;
      }
      break;

    case 3:
      if (transferred == 13 && buf1[0] == '\x55' && buf1[1]=='\x53' && buf1[2]=='\x42' && buf1[3]=='\x53') {
	state = 4;
      } else {
	state = 8;
      }
      break;

    case 4:
      if (transferred == 13 && buf1[12] == '\x02') {
	state = 8;
      } else {
	state = 0;
      }
      break;
      
    case 5:
      r1 = libusb_clear_halt(handler, 0x82);
      if (r1 != 0) {
	error_at_line(0,0,__FILE__,__LINE__,"Error: cannot clear halt for endpoints 0x82");
      }
      state = 6;
      break;

    case 6:
      r1 = libusb_bulk_transfer(handler, 0x82, buf1, 1024, &transferred, 3000);
      if (r1==0) {
	printf ("CSW:");
	for (i=0; i<transferred; i++) {
	  printf (" %02x",buf1[i]);
	}
	printf ("\n");
      }
      state = 7;
      break;

    case 7:
      if (r1 == 0) {
	state = 3;
      } else {
	state = 8;
      }
      break;

    case 8:      
      r1 = libusb_reset_device(handler);
      if (r1 != 0) {
	error_at_line(0,0,__FILE__,__LINE__,"Error: cannot reset device");
      }
      state = 0;
      break;

    }
  }
  return;
}

void writeBinaryFileSeek(libusb_device_handle *handler,
			 unsigned char cmd,
			 unsigned int sector,
			 const char *filename,
			 unsigned int seek,
			 unsigned int len,
			 unsigned int sector2,
			 const unsigned char *flags) {
  FILE *file1=NULL;
  unsigned char data[1024];
  int transferred;
  int r1;
  static unsigned char buf1[2*1024*1024];
  int i;
  int tLen;

  memset (data,0,sizeof(data));
  
  file1 = fopen(filename,"rb");
  if (file1 == NULL) {
    error_at_line(0,0,__FILE__,__LINE__,"Error: cannot read file %s",filename);
    return;
  }
  fseek(file1,seek,SEEK_SET);

  /* signature */
  data[0] = '\x55';
  data[1] = '\x53';
  data[2] = '\x42';
  data[3] = '\x43';
  /* tag */
  data[4] = '\0';
  data[5] = '\0';
  data[6] = '\0';
  data[7] = '\0';
  /* Data Transfer Length */
  data[8] = len%256;
  data[9] = (len/256)%256;
  data[10] = (len/256/256)%256;
  data[11] = (len/256/256/256)%256;
  /* flags */
  data[12] = '\x00';
  /* lun */
  data[13] = '\x00';
  /* cdb length: 16 */
  data[14] = '\x10';

  /* cbwcb */
  /* cbwcb - scsi cmd */
  data[15] = cmd;
  /* sector ? */
  data[16] = sector%256;
  data[17] = (sector/256)%256;
  data[18] = (sector/256/256)%256;
  data[19] = (sector/256/256/256)%256;
  /* length ? */
  data[20] = len%256;
  data[21] = (len/256)%256;
  data[22] = (len/256/256)%256;
  data[23] = (len/256/256/256)%256;

  data[24] = sector2%256;
  data[25] = (sector2/256)%256;
  data[26] = (sector2/256/256)%256;
  data[27] = (sector2/256/256/256)%256;

  
  if (flags != NULL) {
    for (i=28; i<31; i++) {
      data[i] = flags[i-28];
    }
  }

  printf ("CBW:");
  for (i=0; i<31; i++) {
    printf (" %02x", data[i]);
  }
  printf ("\n");
  transferred=0;
  r1 = libusb_bulk_transfer(handler, 0x01, data, 31, &transferred, 0);
  if (r1 != 0) {
    error_at_line(0,0,__FILE__,__LINE__,"Error: cannot send CBW for read");
  }
  if (transferred != 31) {
    error_at_line(0,0,__FILE__,__LINE__,"Error: transferred %d != 31",transferred);
  }
  sleep(1);

  
  for (tLen = len; tLen > 0 && !feof(file1); ) {
    int rLen=0;
    if (tLen > sizeof(buf1)) {
      rLen = fread(buf1,1,sizeof(buf1),file1);
    } else {
      rLen = fread(buf1,1,tLen, file1);
    }
    if (rLen <= 0) {
      break;
    }
    transferred = 0;
    r1 = libusb_bulk_transfer(handler, 0x01, buf1, rLen, &transferred, 0);
    if (r1 != 0) {
      error_at_line(0,0,__FILE__,__LINE__,"Error: cannot send data");
      break;
    }
    if (transferred != rLen) {
      error_at_line(0,0,__FILE__,__LINE__,"Error: transferred %d != rLen = %d",transferred,rLen);
      break;
    }
    sleep(1);
    printf ("Bulk transferred %d bytes\n",rLen);
    tLen -= rLen;
  }
  
  fclose(file1);
  file1 = NULL;

  readCSW(handler);
}

void writeBinaryFile(libusb_device_handle *handler,
		     unsigned char cmd,
		     unsigned int sector,
		     const char *filename,
		     unsigned int sector2,
		     const unsigned char *flags) {
  FILE *file1=NULL;
  struct stat stat1;
  unsigned int len=0;
  int r1;
  int i;
  
  memset (&stat1,0,sizeof(stat1));
  
  file1 = fopen(filename,"rb");
  if (file1 == NULL) {
    error_at_line(0,0,__FILE__,__LINE__,"Error: cannot read file %s",filename);
    return;
  }
  fstat(fileno(file1), &stat1);
  fclose(file1);
  len = (unsigned int)stat1.st_size;
  writeBinaryFileSeek(handler,cmd,sector,filename,0,len,sector2,flags);
  
}

void unknownCMD07(libusb_device_handle *handler) {
  unsigned char data[1024];
  int transferred;
  int r1;
  unsigned char buf1[4096];
  int i;
  int len=0;

  memset(data,0,sizeof(data));
  /* signature */
  data[0] = '\x55';
  data[1] = '\x53';
  data[2] = '\x42';
  data[3] = '\x43';
  /* tag */
  data[4] = '\0';
  data[5] = '\0';
  data[6] = '\0';
  data[7] = '\0';
  /* Data Transfer Length */
  data[8] = len%256;
  data[9] = (len/256)%256;
  data[10] = (len/256/256)%256;
  data[11] = (len/256/256/256)%256;
  /* flags */
  data[12] = '\x00';
  /* lun */
  data[13] = '\x00';
  /* cdb length: 00 */
  data[14] = '\x00';

  /* cbwcb */
  /* cbwcb - scsi cmd */
  data[15] = '\x10';
  /* sector ? */
  data[16] = '\x00';
  data[17] = '\xf0';
  data[18] = '\x06';
  data[19] = '\xe4';
  /* length ? */
  data[20] = len%256;
  data[21] = (len/256)%256;
  data[22] = (len/256/256)%256;
  data[23] = (len/256/256/256)%256;

  printf ("data: \n");
  for (i=0; i<31; i++) {
    printf ("%02x ", data[i]);
  }
  printf ("\n");
  transferred=0;
  r1 = libusb_bulk_transfer(handler, 0x01, data, 31, &transferred, 0);
  if (r1 != 0) {
    error_at_line(0,0,__FILE__,__LINE__,"Error: cannot send CBW for read");
  }
  printf ("Transffered: %d\n",transferred);

  readCSW(handler);
}

void unknownCMD50(libusb_device_handle *handler) {
  unsigned char data[1024];
  int transferred;
  int r1;
  unsigned char buf1[4096];
  int i;
  int len=0;

  memset(data,0,sizeof(data));
  /* signature */
  data[0] = '\x55';
  data[1] = '\x53';
  data[2] = '\x42';
  data[3] = '\x43';
  /* tag */
  data[4] = '\0';
  data[5] = '\0';
  data[6] = '\0';
  data[7] = '\0';
  /* Data Transfer Length */
  data[8] = len%256;
  data[9] = (len/256)%256;
  data[10] = (len/256/256)%256;
  data[11] = (len/256/256/256)%256;
  /* flags */
  data[12] = '\x00';
  /* lun */
  data[13] = '\x00';
  /* cdb length: 00 */
  data[14] = '\x10';

  /* cbwcb */
  /* cbwcb - scsi cmd */
  data[15] = '\xcd';
  /* sector ? */
  data[16] = '\x20';
  data[17] = '\x00';
  data[18] = '\x00';
  data[19] = '\x00';
  /* length ? */
  data[20] = len%256;
  data[21] = (len/256)%256;
  data[22] = (len/256/256)%256;
  data[23] = (len/256/256/256)%256;

  data[27] = '\x1f';

  printf ("data: \n");
  for (i=0; i<31; i++) {
    printf ("%02x ", data[i]);
  }
  printf ("\n");
  transferred=0;
  r1 = libusb_bulk_transfer(handler, 0x01, data, 31, &transferred, 0);
  if (r1 != 0) {
    error_at_line(0,0,__FILE__,__LINE__,"Error: cannot send CBW for read");
  }
  printf ("Transffered: %d\n",transferred);

  readCSW(handler);
}

char* getbinpath(char *buf, int size) {
  char path[4096];
  char dest[4096];
  pid_t pid = getpid();
  snprintf(path, sizeof(path)-1, "/proc/%d/exe", (int)pid);
  if (readlink(path, dest, sizeof(dest)-1)  == -1) {
    return NULL;
  } else {
    snprintf(buf,size,"%s",dest);
  }
  return buf;
}

char* getbindirectory(char *buf, int size) {
  char path[4096];
  char *c;
  char *dir;
  int len_dir;
  if (getbinpath(path,sizeof(path)) == NULL) {
    return NULL;
  }
  c = strrchr(path,'/');
  if (c==NULL) {
    return NULL;
  }
  len_dir = ((int)(c-path))+1;
  dir = (char *)malloc(len_dir+1);
  memset(dir,0,len_dir+1);
  strncpy(dir,path,len_dir);
  snprintf(buf,size,"%s",dir);
  if (dir != NULL) {
    free(dir);
    dir = NULL;
  }
  return buf;
}

char* find_firmware(const char *filename, char *firmwareFilename, int firmwareFilenameSize) {
  char *listOfDataDirectory[] = { NULL, "/usr/local/share/linaro-adfu-tool-bg96/firmwares", "/usr/share/linaro-adfu-tool-bg96/firmwares", "./firmwares", "../firmwares" };
  char dataDirectory[4096];
  int i;
  struct stat fileStat;

  listOfDataDirectory[0] = dataDirectory;

  if (getbindirectory(dataDirectory,sizeof(dataDirectory))==NULL) {
    error_at_line(0,0,__FILE__,__LINE__,"Cannot get executable directory");
    dataDirectory[0]='.';
    dataDirectory[1]='\0';
  } else {
    strncat(dataDirectory,"../firmwares",sizeof(dataDirectory));
  }

  for (i=0; i<sizeof(listOfDataDirectory)/sizeof(listOfDataDirectory[0]); i++) {
    int r1;
    memset(&fileStat,0,sizeof(fileStat));
    snprintf(firmwareFilename, firmwareFilenameSize, "%s/%s", listOfDataDirectory[i], filename);
    r1 = stat(firmwareFilename, &fileStat);
    if (r1 != 0) {
      continue;
    }
    if ( (!S_ISREG(fileStat.st_mode))
	 && (!S_ISLNK(fileStat.st_mode))
	 && (!S_ISFIFO(fileStat.st_mode))
	 ) {
      continue;
    }
    printf ("Use %s for %s\n", firmwareFilename, filename);
    return firmwareFilename;
  }
  error_at_line(0,0,__FILE__,__LINE__, "Cannot find %s", filename);
  memset(firmwareFilename,0,firmwareFilenameSize);
  return NULL;
}

libusb_device_handle* start(int argc, char **argv) {
  libusb_device_handle *handler = NULL;
  int r1=0;
  char firmwareFilename[4096];
  
  handler = b96_init_device();
  if (handler == NULL) {
    return NULL;
  }
  printf ("Handler: %p\n",handler);

  if (find_firmware("adfudec.bin", firmwareFilename, sizeof(firmwareFilename))==NULL) {
    error_at_line(0,0,__FILE__,__LINE__, "Error: Cannot find adfudec.bin");
    return handler;
  }
  writeBinaryFile(handler, '\x05', 0xe406f000, firmwareFilename, 0, NULL);
  sleep(1);

  if (find_firmware("bootloader.bin", firmwareFilename, sizeof(firmwareFilename))==NULL) {
    error_at_line(0,0,__FILE__,__LINE__, "Error: Cannot find bootloader.bin");
    return handler;
  }
  writeBinaryFileSeek(handler, '\x05', 0xe406e000, firmwareFilename, 0x1000, 4096, 0, NULL);
  sleep(1);

  unknownCMD07(handler);
  sleep(10);

  libusb_close(handler);
  handler = b96_init_device();
  if (handler == NULL) {
    return NULL;
  }
  setuid(getuid());

  if (find_firmware("bl31.bin", firmwareFilename, sizeof(firmwareFilename))==NULL) {
    error_at_line(0,0,__FILE__,__LINE__, "Error: Cannot find bl31.bin");
    return handler;
  }
  writeBinaryFile(handler, '\xcd' ,0x13, firmwareFilename,  0x1f000000, NULL);

  if (find_firmware("bl32.bin", firmwareFilename, sizeof(firmwareFilename))==NULL) {
    error_at_line(0,0,__FILE__,__LINE__, "Error: Cannot find bl32.bin");
    return handler;
  }
  writeBinaryFile(handler, '\xcd', 0x13, firmwareFilename, 0x1f202000, NULL);
  
  writeBinaryFile(handler, '\xcd', 0x13, argv[1], 0x10ffffc0, NULL);
  sleep(2);
  
  unknownCMD50(handler);
  sleep(2);

  libusb_close(handler);
  handler = NULL;
  return handler;
}

void usage(int argc, char **argv) {
  printf ("Usage: %s <u-boot-dtb.img>\n", argv[0]);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    usage(argc, argv);
    return 2;
  }

  b96_init_usb();
  
  libusb_device_handle *handler = NULL;
  handler = start(argc, argv);
  if (handler != NULL) {
    b96_uninit_device(handler);
    handler=NULL;
  }

  b96_uninit_usb();
  
  return 0;
}
