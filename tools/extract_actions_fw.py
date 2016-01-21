#!/usr/bin/env python
#
# Copyright (C) 2015 Ying-Chun Liu (PaulLiu) <paulliu@debian.org>
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
                                           
import mmap
import sys
import struct

def KnuthMorrisPratt(text, pattern):
    # allow indexing into pattern and protect against change during yield
    pattern = list(pattern)

    # build table of shift amounts
    shifts = [1] * (len(pattern) + 1)
    shift = 1
    for pos in range(len(pattern)):
        while shift <= pos and pattern[pos] != pattern[pos-shift]:
            shift += shifts[pos-shift]
        shifts[pos+1] = shift

    # do the actual search
    startPos = 0
    matchLen = 0
    while True:
        cS = text.read(512)
        if cS is None:
            break
        for c in cS:
            while matchLen == len(pattern) or \
                  matchLen >= 0 and pattern[matchLen] != c:
                startPos += shifts[matchLen]
                matchLen -= shifts[matchLen]
            matchLen += 1
            if matchLen == len(pattern):
                yield startPos

def findFAT16PartitionPos(filename):
    fat16Start = -1
    with open(filename, 'rb') as f:
        gen = KnuthMorrisPratt(f, 'FAT16 ')
        for i in gen:
            fat16Start = i-54
            break
    return fat16Start

def findEXT4PartitionPos(filename):
    ext4Start = -1
    with open(filename, 'rb') as f:
        gen = KnuthMorrisPratt(f,'\x53\xef\x01\x00')
        for i in gen:
            ext4Start = i-1080
            break
    return ext4Start

def checkMagicNumber(filename):
    with open(filename, 'rb') as f:
        cS = f.read(16)
        if cS is None:
            return False
        if (len(cS) != 16):
            return False
        magic = "WFDNUOPMOCSTCA"
        for i in range(len(magic)):
            if magic[i] != cS[i]:
                return False
        return True
    return False

def getImageList(filename):
    ret = []
    with open(filename, 'rb') as f:
        f.seek(0x10,0)
        nS = f.read(8)
        headSize = struct.unpack("<Q",nS)[0];
        for i in range(1, headSize/64):
            f.seek(64*i, 0)
            name = f.read(16)
            name = name.rstrip('\0')
            fs = f.read(8)
            fs = fs.rstrip('\0')
            start = f.read(8)
            start = struct.unpack("<Q", start)[0];
            length = f.read(8)
            length = struct.unpack("<Q", length)[0];
            ret.append( (name,fs,start,length) );
    return ret
        

def usage(argv):
    print "Usage: %s image.fw"%(__file__)
                
if __name__ == '__main__':

    if (len(sys.argv)<=1):
        usage(sys.argv)
        sys.exit(2)

    filename = sys.argv[1]

    if (not checkMagicNumber(filename)):
        print '%s is not an Actions firmware file'%(filename)
        sys.exit(1)

    imageList = getImageList(filename)

    for image in imageList:
        with open(filename, 'rb') as f:
            with open(filename.rstrip(".fw")+"."+image[0]+"."+image[1]+'.emmc.img', 'wb') as fo:
                f.seek(image[2], 0)
                readLen = image[3]
                while (readLen > 0):
                    if (readLen > 512):
                        buf = f.read(512)
                        fo.write(buf)
                        readLen -= 512
                    else:
                        buf = f.read(readLen)
                        fo.write(buf)
                        readLen = 0
                fo.close()
