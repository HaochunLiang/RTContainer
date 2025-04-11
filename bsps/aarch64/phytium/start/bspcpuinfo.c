/* SPDX-License-Identifier: BSD-2-Clause */

/**
 * @file
 *
 * @ingroup RTEMSBSPsAArch64Phytium
 *
 * @brief PSCI-based BSP CPU start.
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

#include <assert.h>
#include <rtems/score/smpimpl.h>

#include <bsp/start.h>
#include <bsp.h>
#include <bsp/irq.h>
#include <rtems/score/aarch64-system-registers.h>

#define REGISTER_PREFIX "x"

typedef struct {
    /* cpu affinity */
    uint64_t mpidr;
    /* cpu id physically */
    uint32_t phyiscal_id;
    /* gic offset */
    void *redist_base;
    void *sgi_ppi_base;
} phytium_cpu_core_info;

uint32_t bsp_get_processor_affinity(uint32_t cpu_index);
void bsp_get_processor_affinity_by_mask(uint32_t cpu_mask, uint32_t *cluster_id, uint32_t *target_list);
void *bsp_get_processor_redist(uint32_t cpu_index);
void *bsp_get_processor_sgi_ppi(uint32_t cpu_index);

#if defined(PHYTIUM_BSP_TYPE_E2000Q_DEMO) || \
    defined(PHYTIUM_BSP_TYPE_PHYTIUM_PI)
const phytium_cpu_core_info BSP_core_info[BSP_CORE_NUM] =
{
    { /* core-0 */
        .mpidr = BSP_CORE2_AFF, /* 0x200U */
        .phyiscal_id = 2,
        .redist_base = (void *)BSP_ARM_GIC_RD2_BASE,
        .sgi_ppi_base = (void *)BSP_ARM_GIC_SGI2_BASE
    },
    { /* core-1 */
        .mpidr = BSP_CORE3_AFF, /* 0x201U */
        .phyiscal_id = 3,
        .redist_base = (void *)BSP_ARM_GIC_RD3_BASE,
        .sgi_ppi_base = (void *)BSP_ARM_GIC_SGI3_BASE
    },
    { /* core-2 */
        .mpidr = BSP_CORE0_AFF, /* 0x000U */
        .phyiscal_id = 0,
        .redist_base = (void *)BSP_ARM_GIC_RD0_BASE,
        .sgi_ppi_base = (void *)BSP_ARM_GIC_SGI0_BASE
    },
    { /* core-3 */
        .mpidr = BSP_CORE1_AFF, /* 0x100U */
        .phyiscal_id = 1,
        .redist_base = (void *)BSP_ARM_GIC_RD1_BASE,
        .sgi_ppi_base = (void *)BSP_ARM_GIC_SGI1_BASE
    },
};

#elif defined(PHYTIUM_BSP_TYPE_E2000D_DEMO)
const phytium_cpu_core_info BSP_core_info[BSP_CORE_NUM] =
{
    {
        .mpidr = BSP_CORE0_AFF,
        .phyiscal_id = 0,
        .redist_base = (void *)BSP_ARM_GIC_RD0_BASE,
        .sgi_ppi_base = (void *)BSP_ARM_GIC_SGI0_BASE
    },
    {
        .mpidr = BSP_CORE1_AFF,
        .phyiscal_id = 1,
        .redist_base = (void *)BSP_ARM_GIC_RD1_BASE,
        .sgi_ppi_base = (void *)BSP_ARM_GIC_SGI1_BASE
    },
};

#elif defined(PHYTIUM_BSP_TYPE_D2000_TEST) || \
      defined(PHYTIUM_BSP_TYPE_PD2408_TESTA) || \
      defined(PHYTIUM_BSP_TYPE_PD2308_DEMO)
const phytium_cpu_core_info BSP_core_info[BSP_CORE_NUM] =
{
    {
        .mpidr = BSP_CORE0_AFF,
        .phyiscal_id = 0,
        .redist_base = (void *)BSP_ARM_GIC_RD0_BASE,
        .sgi_ppi_base = (void *)BSP_ARM_GIC_SGI0_BASE
    },
    {
        .mpidr = BSP_CORE1_AFF,
        .phyiscal_id = 1,
        .redist_base = (void *)BSP_ARM_GIC_RD1_BASE,
        .sgi_ppi_base = (void *)BSP_ARM_GIC_SGI1_BASE
    },
    {
        .mpidr = BSP_CORE2_AFF,
        .phyiscal_id = 2,
        .redist_base = (void *)BSP_ARM_GIC_RD2_BASE,
        .sgi_ppi_base = (void *)BSP_ARM_GIC_SGI2_BASE
    },
    {
        .mpidr = BSP_CORE3_AFF,
        .phyiscal_id = 3,
        .redist_base = (void *)BSP_ARM_GIC_RD3_BASE,
        .sgi_ppi_base = (void *)BSP_ARM_GIC_SGI3_BASE
    },    
    {
        .mpidr = BSP_CORE4_AFF,
        .phyiscal_id = 4,
        .redist_base = (void *)BSP_ARM_GIC_RD4_BASE,
        .sgi_ppi_base = (void *)BSP_ARM_GIC_SGI4_BASE
    },
    {
        .mpidr = BSP_CORE5_AFF,
        .phyiscal_id = 5,
        .redist_base = (void *)BSP_ARM_GIC_RD5_BASE,
        .sgi_ppi_base = (void *)BSP_ARM_GIC_SGI5_BASE
    },
    {
        .mpidr = BSP_CORE6_AFF,
        .phyiscal_id = 6,
        .redist_base = (void *)BSP_ARM_GIC_RD6_BASE,
        .sgi_ppi_base = (void *)BSP_ARM_GIC_SGI6_BASE
    },
    {
        .mpidr = BSP_CORE7_AFF,
        .phyiscal_id = 7,
        .redist_base = (void *)BSP_ARM_GIC_RD7_BASE,
        .sgi_ppi_base = (void *)BSP_ARM_GIC_SGI7_BASE
    },
};
#elif defined(PHYTIUM_BSP_TYPE_FT2004_DSK)
const phytium_cpu_core_info BSP_core_info[BSP_CORE_NUM] =
{
    {
        .mpidr = BSP_CORE0_AFF,
        .phyiscal_id = 0,
        .redist_base = (void *)BSP_ARM_GIC_RD0_BASE,
        .sgi_ppi_base = (void *)BSP_ARM_GIC_SGI0_BASE
    },
    {
        .mpidr = BSP_CORE1_AFF,
        .phyiscal_id = 1,
        .redist_base = (void *)BSP_ARM_GIC_RD1_BASE,
        .sgi_ppi_base = (void *)BSP_ARM_GIC_SGI1_BASE
    },
    {
        .mpidr = BSP_CORE2_AFF,
        .phyiscal_id = 2,
        .redist_base = (void *)BSP_ARM_GIC_RD2_BASE,
        .sgi_ppi_base = (void *)BSP_ARM_GIC_SGI2_BASE
    },
    {
        .mpidr = BSP_CORE3_AFF,
        .phyiscal_id = 3,
        .redist_base = (void *)BSP_ARM_GIC_RD3_BASE,
        .sgi_ppi_base = (void *)BSP_ARM_GIC_SGI3_BASE
    },
};

#else
#error "SMP affinitiy map undefined !!!"
#endif

uint32_t bsp_get_processor_affinity(uint32_t cpu_index)
{
    assert(cpu_index < BSP_CORE_NUM);
    return BSP_core_info[cpu_index].mpidr;
}

#if defined(PHYTIUM_BSP_TYPE_PD2408_TESTA)
const int cluster_ids[BSP_CORE_NUM] = {
    BSP_CORE0_AFF,
    BSP_CORE1_AFF,
    BSP_CORE2_AFF,
    BSP_CORE3_AFF,
    BSP_CORE4_AFF,
    BSP_CORE5_AFF,
    BSP_CORE6_AFF,
    BSP_CORE7_AFF
};

void bsp_get_processor_affinity_by_mask(uint32_t cpu_mask, uint32_t *cluster_id, uint32_t *target_list)
{
    assert(cluster_id && target_list);
    const phytium_cpu_core_info *core_info;
    uint32_t cpu_index = __builtin_ctz(cpu_mask);
    assert(cpu_index < BSP_CORE_NUM);
    *target_list = 0;
    *cluster_id = 0;

    core_info = &BSP_core_info[cpu_index];

    *target_list = 1;
    *cluster_id = core_info->mpidr;
}
#else
void bsp_get_processor_affinity_by_mask(uint32_t cpu_mask, uint32_t *cluster_id, uint32_t *target_list)
{
    assert(cluster_id && target_list);
  
    const phytium_cpu_core_info *core_info;
    uint32_t cpu_index = __builtin_ctz(cpu_mask);
    assert(cpu_index < BSP_CORE_NUM);
    *target_list = 0;
    *cluster_id = 0;
  
    core_info = &BSP_core_info[cpu_index];
  
    *target_list = ((core_info->mpidr & 0xf) + 1);
    *cluster_id = (core_info->mpidr & 0xff0);
}
#endif

void *bsp_get_processor_redist(uint32_t cpu_index)
{
    assert(cpu_index < BSP_CORE_NUM);
    return BSP_core_info[cpu_index].redist_base;
}

void *bsp_get_processor_sgi_ppi(uint32_t cpu_index)
{
    assert(cpu_index < BSP_CORE_NUM);
    return BSP_core_info[cpu_index].sgi_ppi_base;
}

#ifdef RTEMS_SMP
void _bsp_start(void);

RTEMS_WEAK bool _CPU_SMP_Start_processor( uint32_t cpu_index )
{
  uint32_t PSCI_FN_SYSTEM_CPU_ON = 0xC4000003;
  uint64_t ret = 0;
  uint64_t target_cpu = bsp_get_processor_affinity(cpu_index);

  _AARCH64_Data_synchronization_barrier();
  _AARCH64_Instruction_synchronization_barrier();

  __asm__ volatile (
    "mov " REGISTER_PREFIX "0, %1\n"
    "mov " REGISTER_PREFIX "1, %2\n"
    "mov " REGISTER_PREFIX "2, %3\n"
    "mov " REGISTER_PREFIX "3, #0\n"
#ifdef BSP_CPU_ON_USES_SMC
    "smc #0\n"
#else
    "hvc #0\n"
#endif
    "mov %0, " REGISTER_PREFIX "0\n"
    : "=r" ( ret ) : "r" ( PSCI_FN_SYSTEM_CPU_ON ), "r" ( target_cpu ),
    "r" ( _bsp_start )
    : REGISTER_PREFIX "0", REGISTER_PREFIX "1", REGISTER_PREFIX "2",
    REGISTER_PREFIX "3"
  );

  return ret == 0;
}

RTEMS_WEAK bool _CPU_SMP_Reset_system( void )
{
    uint32_t PSCI_FN_SYSTEM_RESET = 0x84000009;
    uint64_t ret = 0;

    _AARCH64_Data_synchronization_barrier();
    _AARCH64_Instruction_synchronization_barrier();

    __asm__ volatile (
      "mov " REGISTER_PREFIX "0, %1\n"
      "mov " REGISTER_PREFIX "1, %2\n"
      "mov " REGISTER_PREFIX "2, %3\n"
      "mov " REGISTER_PREFIX "3, #0\n"
  #ifdef BSP_CPU_ON_USES_SMC
      "smc #0\n"
  #else
      "hvc #0\n"
  #endif
      "mov %0, " REGISTER_PREFIX "0\n"
      : "=r" ( ret ) : "r" ( PSCI_FN_SYSTEM_RESET ), "r" ( 0 ),
      "r" ( 0 )
      : REGISTER_PREFIX "0", REGISTER_PREFIX "1", REGISTER_PREFIX "2",
      REGISTER_PREFIX "3"
    );

    return ret == 0;
}
#endif /* RTEMS_SMP */