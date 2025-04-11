/* SPDX-License-Identifier: BSD-2-Clause */

/**
 * @file
 *
 * @ingroup RTEMSBSPsAArch64Phytium
 *
 * @brief BSP L3 Cache operations
 */

/*
* Copyright (C) 2024 Phytium Technology Co., Ltd.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

#include <fparameters.h>
#include <fio.h>

#define HNF_BASE (unsigned long)(0x3A200000)
#define HNF_COUNT 0x8
#define HNF_PSTATE_REQ (HNF_BASE + 0x10)
#define HNF_PSTATE_STAT (HNF_BASE + 0x18)
#define HNF_PSTATE_OFF 0x0
#define HNF_PSTATE_SFONLY 0x1
#define HNF_PSTATE_HALF 0x2
#define HNF_PSTATE_FULL 0x3
#define HNF_STRIDE 0x10000

void FCacheL3CacheEnable(void);
void FCacheL3CacheDisable(void);
void FCacheL3CacheInvalidate(void);
void FCacheL3CacheFlush(void);


void FCacheL3CacheDisable(void)
{
    int i, pstate;


    for (i = 0; i < 8; i++)
    {
        FtOut32(0x3A200010 + i * 0x10000, 1);
    }

    for (i = 0; i < 8; i++)
    {
        do
        {
            pstate = FtIn32(0x3A200018 + i * 0x10000);
        }
        while ((pstate & 0xf) != (0x1 << 2));
    }
}


void FCacheL3CacheFlush(void)
{
    int i, pstate;

    for (i = 0; i < HNF_COUNT; i++)
    {
        FtOut64(HNF_PSTATE_REQ + i * HNF_STRIDE, HNF_PSTATE_SFONLY);
    }
    for (i = 0; i < HNF_COUNT; i++)
    {
        do
        {
            pstate = FtIn64(HNF_PSTATE_STAT + i * HNF_STRIDE);
        }
        while ((pstate & 0xf) != (HNF_PSTATE_SFONLY << 2));
    }

    for (i = 0; i < HNF_COUNT; i++)
    {
        FtOut64(HNF_PSTATE_REQ + i * HNF_STRIDE, HNF_PSTATE_FULL);
    }

    return ;
}


void FCacheL3CacheInvalidate(void)
{
    int i, pstate;

    for (i = 0; i < HNF_COUNT; i++)
    {
        FtOut64(HNF_PSTATE_REQ + i * HNF_STRIDE, HNF_PSTATE_SFONLY);
    }

    for (i = 0; i < HNF_COUNT; i++)
    {
        do
        {
            pstate = FtIn64(HNF_PSTATE_STAT + i * HNF_STRIDE);
        }
        while ((pstate & 0xf) != (HNF_PSTATE_SFONLY << 2));
    }

    for (i = 0; i < HNF_COUNT; i++)
    {
        FtOut64(HNF_PSTATE_REQ + i * HNF_STRIDE, HNF_PSTATE_FULL);
    }

    return ;
}


void FCacheL3CacheEnable(void)
{
    return ;
}