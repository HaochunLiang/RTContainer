/* SPDX-License-Identifier: BSD-2-Clause */

/**
 * @file
 *
 * @ingroup RTEMSBSPsAArch64Phytium
 *
 * @brief This source file contains the
 *   _pre_start() default implementation to flush cache before start
 */

/*
 * Copyright (C) 2025 Phytium Technology Co., Ltd.
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

#include <rtems/asm.h>
#include <bspopts.h>

  .globl _bsp_start
  .section ".bsp_pre_start_text", "ax"

/* PreStart entry */

_bsp_start:

#ifdef AARCH64_FLUSH_CACHE_BOOT_UP
  /*
   * We do dcache flush only for the primary core startup
  */
#ifdef RTEMS_SMP
  bl _AArch64_Get_current_processor_for_system_start
  cmp x0, #0
  bne _skip_flush_start
#endif /* RTEMS_SMP */
  bl rtems_cache_flush_entire_data
_skip_flush_start:
#endif /* AARCH64_FLUSH_CACHE_BOOT_UP */
  /* access generic timer registers */
  mov x0, 0x3
  msr CNTHCTL_EL2, x0

  mrs x0, HCR_EL2
  /* bit(31) */
  orr x0, x0, (1 << 31)
  /* bit(27) */
  bic x0, x0, (1 << 27)
  /* bit(5) */
  bic x0, x0, (1 << 5)
  /* bit(4) */
  bic x0, x0, (1 << 4)
  /* bit(3) */
  bic x0, x0, (1 << 3)
  msr HCR_EL2, x0

  bl _start

#include "../shared/start/start.S"