#include <bsp/irq-default.h>
/* SPDX-License-Identifier: BSD-2-Clause */

/**
 * @file
 *
 * @ingroup RTEMSBSPsAArch64Phytium
 *
 * @brief BSP IRQ definitions
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

#ifndef LIBBSP_AARCH64_PHYTIUM_IRQ_H
#define LIBBSP_AARCH64_PHYTIUM_IRQ_H

#ifndef ASM

#include <rtems.h>
#include <dev/irq/arm-gic-irq.h>

#if defined(PHYTIUM_BSP_TYPE_PHYTIUM_PI)
#include <soc/phytiumpi/fparameters.h>
#elif defined(PHYTIUM_BSP_TYPE_E2000D_DEMO)
#include <soc/e2000d/fparameters.h>
#elif defined(PHYTIUM_BSP_TYPE_E2000Q_DEMO)
#include <soc/e2000q/fparameters.h>
#elif defined(PHYTIUM_BSP_TYPE_D2000_TEST)
#include <soc/d2000/fparameters.h>
#elif defined(PHYTIUM_BSP_TYPE_FT2004_DSK)
#include <soc/ft2004/fparameters.h>
#elif defined(PHYTIUM_BSP_TYPE_PD2308_DEMO)
#include <soc/pd2308/fparameters.h>
#elif defined(PHYTIUM_BSP_TYPE_PD2408_TESTA)
#include <soc/pd2408/fparameters.h>
#else
#error "Target unselected !!!"
#endif

/**
 * @defgroup phytium_interrupt Interrrupt Support
 *
 * @ingroup RTEMSBSPsARMPhytium
 *
 * @brief Interrupt support.
 */

#ifdef  BSP_INTERRUPT_VECTOR_COUNT
#undef  BSP_INTERRUPT_VECTOR_COUNT
#endif

#define BSP_INTERRUPT_VECTOR_COUNT  270

/* Interrupts vectors */
#define BSP_TIMER_VIRT_PPI          GENERIC_VTIMER_IRQ_NUM
#define BSP_TIMER_PHYS_NS_PPI       GENERIC_TIMER_NS_IRQ_NUM

#if defined(FXMAC0_QUEUE0_IRQ_NUM)
#define BSP_PHYTIUM_XMAC0_IRQ       FXMAC0_QUEUE0_IRQ_NUM
#endif

#if defined(FXMAC1_QUEUE0_IRQ_NUM)
#define BSP_PHYTIUM_XMAC1_IRQ       FXMAC1_QUEUE0_IRQ_NUM
#endif

#if defined(FPCIE_ECAM_INTA_IRQ_NUM)
#define BSP_PHYTIUM_INTA_IRQ        FPCIE_ECAM_INTA_IRQ_NUM
#endif

#endif /* ASM */
#endif /* LIBBSP_AARCH64_PHYTIUM_IRQ_H */
