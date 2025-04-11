/* SPDX-License-Identifier: BSD-2-Clause */

/**
 * @file
 *
 * @ingroup RTEMSBSPsAArch64Phytium
 *
 * @brief This source file contains the
 *   _AArch64_Get_current_processor_for_system_start() default implementation
 *   using the MPIDR_EL1.
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

#include <bsp.h>
#include <rtems/asm.h>

FUNCTION_ENTRY(_AArch64_Get_current_processor_for_system_start)

    mrs x0, MPIDR_EL1
    and x0, x0, #BSP_CORE_AFF_MASK

#if defined(PHYTIUM_BSP_TYPE_E2000Q_DEMO) || \
    defined(PHYTIUM_BSP_TYPE_PHYTIUM_PI)
    ldr x1, =BSP_CORE2_AFF
    cmp x0, x1
    beq core0

    ldr x1, =BSP_CORE3_AFF
    cmp x0, x1
    beq core1

    ldr x1, =BSP_CORE0_AFF
    cmp x0, x1
    beq core2

    ldr x1, =BSP_CORE1_AFF
    cmp x0, x1
    beq core3
    /* PHYTIUM_BSP_TYPE_E2000Q_DEMO */
    /* PHYTIUM_BSP_TYPE_PHYTIUM_PI */
#elif defined(PHYTIUM_BSP_TYPE_E2000D_DEMO)
    ldr x1, =BSP_CORE0_AFF
    cmp x0, x1
    beq core0

    ldr x1, =BSP_CORE1_AFF
    cmp x0, x1
    beq core1
    /* PHYTIUM_BSP_TYPE_E2000D_DEMO */
#elif defined(PHYTIUM_BSP_TYPE_D2000_TEST) || \
      defined(PHYTIUM_BSP_TYPE_PD2408_TESTA) || \
      defined(PHYTIUM_BSP_TYPE_PD2308_DEMO)
    ldr x1, =BSP_CORE0_AFF
    cmp x0, x1
    beq core0

    ldr x1, =BSP_CORE1_AFF
    cmp x0, x1
    beq core1

    ldr x1, =BSP_CORE2_AFF
    cmp x0, x1
    beq core2

    ldr x1, =BSP_CORE3_AFF
    cmp x0, x1
    beq core3

    ldr x1, =BSP_CORE4_AFF
    cmp x0, x1
    beq core4

    ldr x1, =BSP_CORE5_AFF
    cmp x0, x1
    beq core5

    ldr x1, =BSP_CORE6_AFF
    cmp x0, x1
    beq core6

    ldr x1, =BSP_CORE7_AFF
    cmp x0, x1
    beq core7
    /* PHYTIUM_BSP_TYPE_D2000_TEST */
    /* PHYTIUM_BSP_TYPE_PD2408_TESTA */
    /* PHYTIUM_BSP_TYPE_PD2308_DEMO */
#elif defined(PHYTIUM_BSP_TYPE_FT2004_DSK)
    ldr x1, =BSP_CORE0_AFF
    cmp x0, x1
    beq core0

    ldr x1, =BSP_CORE1_AFF
    cmp x0, x1
    beq core1

    ldr x1, =BSP_CORE2_AFF
    cmp x0, x1
    beq core2

    ldr x1, =BSP_CORE3_AFF
    cmp x0, x1
    beq core3
    /* PHYTIUM_BSP_TYPE_FT2004_DSK */
#endif

    b default

core0:
    mov x0, #0
    b return

core1:
    mov x0, #1
    b return

core2:
    mov x0, #2
    b return

core3:
    mov x0, #3
    b return

core4:
    mov x0, #4
    b return

core5:
    mov x0, #5
    b return

core6:
    mov x0, #6
    b return

core7:
    mov x0, #7
    b return

default:
    and x0, x0, #15

return:
    RET

FUNCTION_END(_AArch64_Get_current_processor_for_system_start)
