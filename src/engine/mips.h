/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * mips.h
 * Copyright (C) HjjHjj <hjjhjj@gmail.com>
 *
 * mips.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mips.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef HJJHJJ_MIPS_H
#define HJJHJJ_MIPS_H

#include "base.h"
/*
inline uint64_t MAKE_LONG_LONG(uint32_t LowLong, uint32_t HighLong)
{
    return (uint64_t) LowLong | ((uint64_t) HighLong << 32);
}
*/

static __inline__ int Bsf(uint32_t Operand)
{
    /* Use gcc __buildin function instead of asm */
    if (Operand==0)
        return 0;
    return __builtin_ctzl(Operand);
    /*
        int eax;
        asm __volatile__ (
            "bsfl %0, %0" "\n\t"
        : "=a" (eax)
                    : "0" (Operand)
                );
        return eax;
    */
}

static __inline__ int Bsr(uint32_t Operand)
{
    /* Use gcc __buildin function instead of asm */
    if (Operand==0)
        return 0;
    return 31 - __builtin_clzl(Operand);
    /*
      int eax;
      asm __volatile__ (
          "bsrl %0, %0" "\n\t"
      : "=a" (eax)
                  : "0" (Operand)
              );
      return eax;
      */
}
//#ifdef linux
//#include <sys/time.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
//#endif

static __inline__ uint64_t TimeStampCounter(void)
{
    int fd = -1;
    fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1)
    {
      return 0x12345678;
    }
    uint64_t random;
    read(fd,  &random, sizeof(random));
    close(fd);
    return random;
    /*
        uint32_t eax, edx;
        asm __volatile__ (
            "rdtsc" "\n\t"
        : "=a" (eax), "=d" (edx)
                    :
                );
        return MAKE_LONG_LONG(eax, edx);
    */
}
#endif
