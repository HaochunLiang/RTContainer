/*
 * Copyright (C) 2022, Phytium Technology Co., Ltd.   All Rights Reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 * FilePath: fparameters.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:58:51
 * Description:  This file is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#ifndef PD2308_FPARAMETERS_H
#define PD2308_FPARAMETERS_H

#ifdef __cplusplus
extern "C"
{
#endif

#define SOC_TARGET_PD2308

#define CORE0_AFF                      0x000
#define CORE1_AFF                      0x100
#define CORE2_AFF                      0x200
#define CORE3_AFF                      0x300
#define CORE4_AFF                      0x10000
#define CORE5_AFF                      0x10100
#define CORE6_AFF                      0x10200
#define CORE7_AFF                      0x10300
#define FCORE_NUM                      8
#define CORE_AFF_MASK                  0xFFFFF

#define FCPU_CONFIG_ARM64_PA_BITS      44

/* cache */
#define CACHE_LINE_ADDR_MASK           0x3FUL
#define CACHE_LINE                     64U


/****** GIC v3  *****/
#define FT_GICV3_INSTANCES_NUM         1U
#define GICV3_REG_LENGTH               0x00009000U

/*
 * The maximum priority value that can be used in the GIC.
 */
#define GICV3_MAX_INTR_PRIO_VAL        240U
#define GICV3_INTR_PRIO_MASK           0x000000f0U

#define ARM_GIC_NR_IRQS                270U
#define ARM_GIC_IRQ_START              0U
#define FGIC_NUM                       1U

#define ARM_GIC_IPI_COUNT              16U /* MPCore IPI count         */
#define SGI_INT_MAX                    16U
#define SPI_START_INT_NUM              32U   /* SPI start at ID32        */
#define PPI_START_INT_NUM              16U   /* PPI start at ID16        */
#define GIC_INT_MAX_NUM                1020U /* GIC max interrupts count */

#define GICV3_BASE_ADDR                0x36800000U
#define GICV3_ITS_BASE_ADDR            0x36840000U
#define GICV3_DISTRIBUTOR_BASE_ADDR    (GICV3_BASE_ADDR + 0)
#define GICV3_RD_BASE_ADDR             (GICV3_BASE_ADDR + 0x60000U)
#define GICV3_RD_OFFSET                (2U << 16)
#define GICV3_RD_SIZE                  (16U << 16)


/* PCIE ECAM */

/* Pci express  */
#define FPCIE_ECAM_INSTANCE_NUM        1
#define FPCIE_ECAM_INSTANCE0           0

#define FPCIE_ECAM_MAX_OUTBOUND_NUM    8

/* Bus, Device and Function */
#define FPCIE_ECAM_CFG_MAX_NUM_OF_BUS  256
#define FPCIE_ECAM_CFG_MAX_NUM_OF_DEV  32
#define FPCIE_ECAM_CFG_MAX_NUM_OF_FUN  8

#define FPCIE_ECAM_INTA_IRQ_NUM        36
#define FPCIE_ECAM_INTB_IRQ_NUM        37
#define FPCIE_ECAM_INTC_IRQ_NUM        38
#define FPCIE_ECAM_INTD_IRQ_NUM        39

/*  max scan*/
#define FPCIE_MAX_SCAN_NUMBER          128

/* memory space */
#define FPCI_ECAM_CONFIG_BASE_ADDR     0x40000000 /* ecam */
#define FPCI_ECAM_CONFIG_REG_LENGTH    0x10000000

#define FPCI_ECAM_IO_CONFIG_BASE_ADDR  0x50000000 /* io address space */
#define FPCI_ECAM_IO_CONFIG_REG_LENGTH 0x08000000

#define FPCI_ECAM_MEM32_BASE_ADDR      0x58000000 /* mmio 32 */
#define FPCI_ECAM_MEM32_REG_LENGTH     0x27ffffff

#define FPCI_ECAM_MEM64_BASE_ADDR      0x1000000000 /* mmio 64 */
#define FPCI_ECAM_MEM64_REG_LENGTH     0x1fffffffff


/* FPCIE Controler */
#define FPCIEC_INSTANCE_NUM            3

#define FPCIEC_INSTANCE0_INDEX         0 /* peu1.c3 */
#define FPCIEC_INSTANCE1_INDEX         1 /* peu1.c4 */
#define FPCIEC_INSTANCE2_INDEX         2 /* peu1.c5 */

#define FPCIEC_INSTANCE0_MISC_IRQ_NUM  40 /* Override all controllers */
#define FPCIEC_INSTANCE3_MISC_IRQ_NUM  40 /* Override all controllers */

/* for hbp */
#define FPCIEC_INSTANCE0_NUM           2 /* peu1.c3 */
#define FPCIEC_INSTANCE1_NUM           3 /* peu1.c4 */
#define FPCIEC_INSTANCE2_NUM           4 /* peu1.c5 */

#define FPCIEC_INSTANCE0_CONFIG_BASE   0x34060000U
#define FPCIEC_INSTANCE1_CONFIG_BASE   0x34080000U
#define FPCIEC_INSTANCE2_CONFIG_BASE   0x310a0000U

#define FPCIEC_INSTANCE0_CONTROL_BASE  0x34200000U
#define FPCIEC_INSTANCE1_CONTROL_BASE  0x34200000U
#define FPCIEC_INSTANCE2_CONTROL_BASE  0x34200000U

#define FPCIEC_INSTANCE0_DMA_NUM       0
#define FPCIEC_INSTANCE1_DMA_NUM       0
#define FPCIEC_INSTANCE2_DMA_NUM       0


#define FPCIEC_INSTANCE0_DMABASE       (0U)
#define FPCIEC_INSTANCE1_DMABASE       (0U)
#define FPCIEC_INSTANCE2_DMABASE       (0U)

#define FPCIEC_MAX_OUTBOUND_NUM        8

/* XMAC */
#define FXMAC_NUM                      2U

#define FXMAC0_ID                      0U
#define FXMAC1_ID                      1U

#define FXMAC0_BASE_ADDR               0x36CE0000U
#define FXMAC1_BASE_ADDR               0x36CE2000U

#define FXMAC0_PCLK                    250000000U
#define FXMAC1_PCLK                    250000000U
#define FXMAC2_PCLK                    250000000U
#define FXMAC3_PCLK                    250000000U

#define FXMAC0_HOTPLUG_IRQ_NUM         (72U)
#define FXMAC1_HOTPLUG_IRQ_NUM         (73U)

#define FXMAC_QUEUE_MAX_NUM            16U

#define FXMAC0_QUEUE0_IRQ_NUM          (64)
#define FXMAC0_QUEUE1_IRQ_NUM          (65)
#define FXMAC0_QUEUE2_IRQ_NUM          (66)
#define FXMAC0_QUEUE3_IRQ_NUM          (67)

#define FXMAC1_QUEUE0_IRQ_NUM          (68)
#define FXMAC1_QUEUE1_IRQ_NUM          (69)
#define FXMAC1_QUEUE2_IRQ_NUM          (70)
#define FXMAC1_QUEUE3_IRQ_NUM          (71)

#define FXMAC_PHY_MAX_NUM              32U

#define FXMAC_CAPS                     0x00000002 /* use  */

#define FXMAC_CLK_TYPE_2

/* sata controller */
#define FSATA0_BASE_ADDR        0x36CE5000U

#define FSATA0_IRQ_NUM          63

#define FSATA0_ID               0
#define FSATA_NUM               1

/* GDMA */
#define FGDMA0_ID               0U
#define FGDMA0_BASE_ADDR        0x36CE7000U
#define FGDMA0_CHANNEL0_IRQ_NUM 61U
#define FGDMA0_CHANNEL1_IRQ_NUM 62U
#define FGDMA_NUM_OF_CHAN       2
#define FGDMA_INSTANCE_NUM      1U
#define FGDMA0_CAPACITY         (1U << 1)

/* WDT */
#if !defined(__ASSEMBLER__)
enum
{
    FWDT0_ID = 0,
    FWDT1_ID,

    FWDT_NUM
};
#endif

#define FWDT0_REFRESH_BASE_ADDR   0x28014000U
#define FWDT1_REFRESH_BASE_ADDR   0x28016000U

#define FWDT_CONTROL_BASE_ADDR(x) ((x) + 0x1000)

#define FWDT0_IRQ_NUM             112
#define FWDT1_IRQ_NUM             113

#define FWDT_CLK_FREQ_HZ          48000000U /* 48MHz */


/* QSPI */
#if !defined(__ASSEMBLER__)
enum
{
    FQSPI0_ID = 0,

    FQSPI_NUM
};

#define FQSPI_BASE_ADDR 0x28004000U

/* FQSPI cs 0_3, chip number */
enum
{
    FQSPI_CS_0 = 0,
    FQSPI_CS_1 = 1,
    FQSPI_CS_2 = 2,
    FQSPI_CS_3 = 3,
    FQSPI_CS_NUM
};

#endif

#define FQSPI_MEM_START_ADDR      0x0U
#define FQSPI_MEM_END_ADDR        0x0FFFFFFFU /* 256MB */
#define FQSPI_CAP_FLASH_NUM_MASK  GENMASK(4, 3)
#define FQSPI_CAP_FLASH_NUM(data) ((data) << 3) /* Flash number */

/* SPI */
#define FSPI0_BASE_ADDR           0x28012000U
#define FSPI1_BASE_ADDR           0x28013000U

#define FSPI0_ID                  0U
#define FSPI1_ID                  1U
#define FSPI_NUM                  2U

#define FSPI_DMA_CAPACITY         BIT(0)
#define FSPIM0_DMA_CAPACITY       0
#define FSPIM1_DMA_CAPACITY       0

#define FSPI0_IRQ_NUM             110U
#define FSPI1_IRQ_NUM             111U

#define FSPI_CLK_FREQ_HZ          50000000U
#define FSPI_DEFAULT_SCLK         5000000U

#define PLAT_AHCI_HOST_MAX_COUNT  5
#define AHCI_BASE_0               0
#define AHCI_BASE_1               0
#define AHCI_BASE_2               0
#define AHCI_BASE_3               0
#define AHCI_BASE_4               0

#define AHCI_IRQ_0                0
#define AHCI_IRQ_1                0
#define AHCI_IRQ_2                0
#define AHCI_IRQ_3                0
#define AHCI_IRQ_4                0

/* SDIO */
#if !defined(__ASSEMBLER__)
enum
{
    FSDIF0_ID = 0,

    FSDIF_NUM
};
#endif

#define FSDIF0_BASE_ADDR  0x28000000U

#define FSDIF0_IRQ_NUM    96U

#define FSDIF_CLK_FREQ_HZ (1200000000UL) /* 1.2GHz */


/* CANFD */
#define FCAN_CLK_FREQ_HZ  300000000U

#define FCAN0_BASE_ADDR   0x28006000U
#define FCAN1_BASE_ADDR   0x28007000U

#define FCAN0_IRQ_NUM     101U
#define FCAN1_IRQ_NUM     102U

#if !defined(__ASSEMBLER__)
enum
{
    FCAN0_ID = 0,
    FCAN1_ID = 1,

    FCAN_NUM
};
#endif

/* can capacity */
#define FCAN_FD_CAPACITY    BIT(0) /* Whether canfd is supported */
#define FCAN_CAPACITY       FCAN_FD_CAPACITY

/* UART */
#define FUART_NUM           4U
#define FUART_REG_LENGTH    0x18000U

#define FUART0_ID           0U
#define FUART0_IRQ_NUM      103
#define FUART0_BASE_ADDR    0x28008000U
#define FUART0_CLK_FREQ_HZ  100000000U

#define FUART1_ID           1U
#define FUART1_IRQ_NUM      104
#define FUART1_BASE_ADDR    0x28009000U
#define FUART1_CLK_FREQ_HZ  100000000U

#define FUART2_ID           2U
#define FUART2_IRQ_NUM      105
#define FUART2_BASE_ADDR    0x2800A000U
#define FUART2_CLK_FREQ_HZ  100000000U

#define FUART3_BASE_ADDR    0x2800B000U
#define FUART3_ID           3U
#define FUART3_IRQ_NUM      106
#define FUART3_CLK_FREQ_HZ  100000000U

#define FT_STDOUT_BASE_ADDR FUART1_BASE_ADDR
#define FT_STDIN_BASE_ADDR  FUART1_BASE_ADDR


#if !defined(__ASSEMBLER__)
/*I2C0 -> PMBUS0
* I2C1 -> PMBUS1
* I2C2 -> SMBUS0
*/

#define FI2C0_ID 0
#define FI2C1_ID 1
#define FI2C2_ID 2
#define FI2C_NUM 3

#endif

#define FI2C0_BASE_ADDR  0x2800c000
#define FI2C1_BASE_ADDR  0x2800d000
#define FI2C2_BASE_ADDR  0x28021000

#define FI2C0_IRQ_NUM    108
#define FI2C1_IRQ_NUM    109
#define FI2C2_IRQ_NUM    124

#define FI2C_CLK_FREQ_HZ 50000000 /* 50MHz */

#if !defined(__ASSEMBLER__)
enum
{
    FI3C0_ID = 0,
    FI3C1_ID,

    FI3C_NUM
};
#endif

#define FI3C0_BASE_ADDR  0x28018000
#define FI3C1_BASE_ADDR  0x28019000

#define FI3C0_IRQ_NUM    114
#define FI3C1_IRQ_NUM    115

#define FI3C_CLK_FREQ_HZ 100000000

/* PWM */
#if !defined(__ASSEMBLER__)
enum
{
    FPWM0_ID = 0,
    FPWM1_ID = 1,

    FPWM_NUM
};

typedef enum
{
    FPWM_CHANNEL_0 = 0,
    FPWM_CHANNEL_1,

    FPWM_CHANNEL_NUM
} FPwmChannel;
#endif

#define FPWM_BASE_ADDR                 0x2801A000U

#define FPWM_CLK_FREQ_HZ               50000000U /* 50MHz */

#define FPWM0_IRQ_NUM                  118U
#define FPWM1_IRQ_NUM                  119U

/* LSD Config */
#define FLSD_CONFIG_BASE               0x28020000U
#define FLSD_PWM_HADDR                 0x10

#define FPWM_CYCLE_PROCESSING(reg_val) ((reg_val)-0x1)

/* TIMER and TACHO */
#define FTIMER_NUM                     4U
#define FTIMER_CLK_FREQ_HZ             50000000ULL /* 50MHz */
#define FTIMER_TACHO_IRQ_NUM(n)        (120U + (n))
#define FTIMER_TACHO_BASE_ADDR(n)      (0x2801C000U + 0x1000U * (n))

#if !defined(__ASSEMBLER__)
enum
{
    FTACHO0_ID = 0,
    FTACHO1_ID = 1,
    FTACHO2_ID,
    FTACHO3_ID,


    FTACHO_NUM
};
#endif

/* iopad */
#define FIOPAD_BASE_ADDR 0x36CEA000U

#if !defined(__ASSEMBLER__)
/* IOPAD */
enum
{
    FIOPAD0_ID = 0,

    FIOPAD_NUM
};
#endif


/* GPIO */
#define FGPIO0_BASE_ADDR      0x2800E000U
#define FGPIO1_BASE_ADDR      0x2800F000U
#define FGPIO2_BASE_ADDR      0x28010000U
#define FGPIO3_BASE_ADDR      0x28011000U

#define FGPIO_CTRL_0          0
#define FGPIO_CTRL_1          1
#define FGPIO_CTRL_2          2
#define FGPIO_CTRL_3          3
#define FGPIO_CTRL_NUM        4U

#define FGPIO_PORT_A          0U
#define FGPIO_PORT_NUM        1U

#define FGPIO_PIN_0           0U
#define FGPIO_PIN_1           1U
#define FGPIO_PIN_2           2U
#define FGPIO_PIN_3           3U
#define FGPIO_PIN_4           4U
#define FGPIO_PIN_5           5U
#define FGPIO_PIN_6           6U
#define FGPIO_PIN_7           7U
#define FGPIO_PIN_8           8U
#define FGPIO_PIN_9           9U
#define FGPIO_PIN_10          10U
#define FGPIO_PIN_11          11U
#define FGPIO_PIN_12          12U
#define FGPIO_PIN_13          13U
#define FGPIO_PIN_14          14U
#define FGPIO_PIN_15          15U
#define FGPIO_PIN_NUM         16U

#define FGPIO_NUM             (FGPIO_CTRL_NUM * FGPIO_PORT_NUM * FGPIO_PIN_NUM)

#define FGPIO_CAP_IRQ_BY_PIN  (1 << 0) /* 支持外部中断，每个引脚有单独上报的中断 */
#define FGPIO_CAP_IRQ_BY_CTRL (1 << 1) /* 支持外部中断，引脚中断统一上报 */
#define FGPIO_CAP_IRQ_NONE    (1 << 2) /* 不支持外部中断 */

#define FGPIO_ID(ctrl, pin) \
    (((ctrl)*FGPIO_PORT_NUM * FGPIO_PIN_NUM) + ((FGPIO_PORT_A)*FGPIO_PIN_NUM) + (pin))

#define FGPIO_CLK_FREQ_HZ        (50000000UL) /* 50MHz */

/* generic timer */
/* non-secure physical timer int id */
#define GENERIC_TIMER_NS_IRQ_NUM 30U

/* virtual timer int id */
#define GENERIC_VTIMER_IRQ_NUM   27U

#if !defined(__ASSEMBLER__)
enum
{
    GENERIC_TIMER_ID0 = 0, /* non-secure physical timer */
    GENERIC_TIMER_ID1 = 1, /* virtual timer */

    GENERIC_TIMER_NUM
};
#endif


/* register offset of iopad function / pull / driver strength */
#define FIOPAD_C5_REG0_OFFSET   0x0018U
#define FIOPAD_E5_REG0_OFFSET   0x001CU

#define FIOPAD_E3_REG0_OFFSET   0x0020U
#define FIOPAD_L5_REG0_OFFSET   0x0024U
#define FIOPAD_J5_REG0_OFFSET   0x0028U
#define FIOPAD_L3_REG0_OFFSET   0x002CU

#define FIOPAD_N1_REG0_OFFSET   0x0030U
#define FIOPAD_L1_REG0_OFFSET   0x0034U
#define FIOPAD_J1_REG0_OFFSET   0x0038U
#define FIOPAD_N5_REG0_OFFSET   0x003CU

#define FIOPAD_CH19_REG0_OFFSET 0x0040U
#define FIOPAD_CF19_REG0_OFFSET 0x0044U
#define FIOPAD_G3_REG0_OFFSET   0x0048U
#define FIOPAD_G1_REG0_OFFSET   0x004CU

#define FIOPAD_G5_REG0_OFFSET   0x0050U
#define FIOPAD_CB19_REG0_OFFSET 0x0054U
#define FIOPAD_CB21_REG0_OFFSET 0x0058U
#define FIOPAD_BY19_REG0_OFFSET 0x005CU

#define FIOPAD_U13_REG0_OFFSET  0x0060U
#define FIOPAD_U15_REG0_OFFSET  0x0064U
#define FIOPAD_AA7_REG0_OFFSET  0x0068U
#define FIOPAD_AA9_REG0_OFFSET  0x006CU

#define FIOPAD_AA13_REG0_OFFSET 0x0070U
#define FIOPAD_AA11_REG0_OFFSET 0x0074U
#define FIOPAD_AA15_REG0_OFFSET 0x0078U
#define FIOPAD_W11_REG0_OFFSET  0x007CU

#define FIOPAD_W7_REG0_OFFSET   0x0080U
#define FIOPAD_U11_REG0_OFFSET  0x0084U
#define FIOPAD_U9_REG0_OFFSET   0x0088U
#define FIOPAD_U3_REG0_OFFSET   0x008CU

#define FIOPAD_U1_REG0_OFFSET   0x0090U
#define FIOPAD_CM43_REG0_OFFSET 0x0094U
#define FIOPAD_CP45_REG0_OFFSET 0x0098U
#define FIOPAD_CH21_REG0_OFFSET 0x009CU

#define FIOPAD_CK19_REG0_OFFSET 0x00A0U
#define FIOPAD_BT67_REG0_OFFSET 0x00A4U
#define FIOPAD_BP67_REG0_OFFSET 0x00A8U
#define FIOPAD_BV63_REG0_OFFSET 0x00ACU

#define FIOPAD_BT63_REG0_OFFSET 0x00B0U
#define FIOPAD_AN17_REG0_OFFSET 0x00B4U
#define FIOPAD_AR17_REG0_OFFSET 0x00B8U
#define FIOPAD_AR15_REG0_OFFSET 0x00BCU

#define FIOPAD_AN15_REG0_OFFSET 0x00C0U
#define FIOPAD_AJ19_REG0_OFFSET 0x00C4U
#define FIOPAD_AN19_REG0_OFFSET 0x00C8U
#define FIOPAD_AJ15_REG0_OFFSET 0x00CCU

#define FIOPAD_AG21_REG0_OFFSET 0x00D0U
#define FIOPAD_AJ13_REG0_OFFSET 0x00D4U
#define FIOPAD_AJ11_REG0_OFFSET 0x00D8U
#define FIOPAD_AN21_REG0_OFFSET 0x00DCU

#define FIOPAD_AC5_REG0_OFFSET  0x00E0U
#define FIOPAD_AE1_REG0_OFFSET  0x00E4U
#define FIOPAD_AE3_REG0_OFFSET  0x00E8U
#define FIOPAD_AC3_REG0_OFFSET  0x00ECU

#define FIOPAD_AR19_REG0_OFFSET 0x00F0U
#define FIOPAD_BM21_REG0_OFFSET 0x00F4U
#define FIOPAD_BK19_REG0_OFFSET 0x00F8U
#define FIOPAD_BK21_REG0_OFFSET 0x00FCU

#define FIOPAD_BM19_REG0_OFFSET 0x0100U
#define FIOPAD_BP21_REG0_OFFSET 0x0104U
#define FIOPAD_BP19_REG0_OFFSET 0x0108U
#define FIOPAD_BV19_REG0_OFFSET 0x010CU

#define FIOPAD_BV12_REG0_OFFSET 0x0110U
#define FIOPAD_CF67_REG0_OFFSET 0x0114U
#define FIOPAD_CD67_REG0_OFFSET 0x0118U
#define FIOPAD_BY65_REG0_OFFSET 0x011CU

#define FIOPAD_BY67_REG0_OFFSET 0x0120U
#define FIOPAD_CB67_REG0_OFFSET 0x0124U
#define FIOPAD_BV65_REG0_OFFSET 0x0128U
#define FIOPAD_BV67_REG0_OFFSET 0x012CU

#define FIOPAD_BY63_REG0_OFFSET 0x0130U
#define FIOPAD_CB63_REG0_OFFSET 0x0134U
#define FIOPAD_CD63_REG0_OFFSET 0x0138U
#define FIOPAD_W5_REG0_OFFSET   0x013CU

#define FIOPAD_U5_REG0_OFFSET   0x0140U
#define FIOPAD_W3_REG0_OFFSET   0x0144U
#define FIOPAD_AC7_REG0_OFFSET  0x0148U
#define FIOPAD_W1_REG0_OFFSET   0x014CU

#define FIOPAD_AA5_REG0_OFFSET  0x0150U
#define FIOPAD_AA1_REG0_OFFSET  0x0154U
#define FIOPAD_AC1_REG0_OFFSET  0x0158U
#define FIOPAD_BM63_REG0_OFFSET 0x015CU

#define FIOPAD_BP63_REG0_OFFSET 0x0160U
#define FIOPAD_CM47_REG0_OFFSET 0x0164U
#define FIOPAD_CK47_REG0_OFFSET 0x0168U
#define FIOPAD_AG13_REG0_OFFSET 0x016CU

#define FIOPAD_AG11_REG0_OFFSET 0x0170U
#define FIOPAD_AL15_REG0_OFFSET 0x0174U
#define FIOPAD_AJ17_REG0_OFFSET 0x0178U
#define FIOPAD_AC15_REG0_OFFSET 0x017CU

#define FIOPAD_AC9_REG0_OFFSET  0x0180U
#define FIOPAD_AC11_REG0_OFFSET 0x0184U
#define FIOPAD_AE13_REG0_OFFSET 0x0188U
#define FIOPAD_AE11_REG0_OFFSET 0x018CU

#define FIOPAD_CD65_REG0_OFFSET 0x0190U
#define FIOPAD_CF63_REG0_OFFSET 0x0194U
#define FIOPAD_CH63_REG0_OFFSET 0x0198U
#define FIOPAD_CH65_REG0_OFFSET 0x019CU

#define FIOPAD_CH67_REG0_OFFSET 0x01A0U
#define FIOPAD_AE17_REG0_OFFSET 0x01A4U
#define FIOPAD_CF65_REG0_OFFSET 0x01A8U
#define FIOPAD_AA19_REG0_OFFSET 0x01ACU

#define FIOPAD_CE61_REG0_OFFSET 0x01B0U
#define FIOPAD_AC19_REG0_OFFSET 0x01B4U
#define FIOPAD_AA17_REG0_OFFSET 0x01B8U
#define FIOPAD_W13_REG0_OFFSET  0x01BCU

#define FIOPAD_AG15_REG0_OFFSET 0x01C0U
#define FIOPAD_AE21_REG0_OFFSET 0x01C4U
#define FIOPAD_CM45_REG0_OFFSET 0x01C8U
#define FIOPAD_CK45_REG0_OFFSET 0x01CCU

#define FIOPAD_BM65_REG0_OFFSET 0x01D0U
#define FIOPAD_BP65_REG0_OFFSET 0x01D4U

#define FIOPAD_REG0_BEG_OFFSET  FIOPAD_C5_REG0_OFFSET
#define FIOPAD_REG0_END_OFFSET  FIOPAD_BP65_REG0_OFFSET

/* register offset of iopad delay */
#define FIOPAD_CK19_REG1_OFFSET 0x10A0U
#define FIOPAD_BT67_REG1_OFFSET 0x10A4U
#define FIOPAD_BP67_REG1_OFFSET 0x10A8U
#define FIOPAD_BV63_REG1_OFFSET 0x10ACU

#define FIOPAD_BT63_REG1_OFFSET 0x10B0U
#define FIOPAD_AN17_REG1_OFFSET 0x10B4U
#define FIOPAD_AR17_REG1_OFFSET 0x10B8U
#define FIOPAD_AR15_REG1_OFFSET 0x10BCU

#define FIOPAD_AN15_REG1_OFFSET 0x10C0U
#define FIOPAD_AJ19_REG1_OFFSET 0x10C4U
#define FIOPAD_AN19_REG1_OFFSET 0x10C8U
#define FIOPAD_AJ15_REG1_OFFSET 0x10CCU

#define FIOPAD_AG21_REG1_OFFSET 0x10D0U
#define FIOPAD_AJ13_REG1_OFFSET 0x10D4U
#define FIOPAD_AJ11_REG1_OFFSET 0x10D8U
#define FIOPAD_AN21_REG1_OFFSET 0x10DCU

#define FIOPAD_AE1_REG1_OFFSET  0x10E4U
#define FIOPAD_AE3_REG1_OFFSET  0x10E8U
#define FIOPAD_AC3_REG1_OFFSET  0x10ECU

#define FIOPAD_AR19_REG1_OFFSET 0x10F0U
#define FIOPAD_BM21_REG1_OFFSET 0x10F4U
#define FIOPAD_BK19_REG1_OFFSET 0x10F8U
#define FIOPAD_BK21_REG1_OFFSET 0x10FCU

#define FIOPAD_BM19_REG1_OFFSET 0x1100U
#define FIOPAD_BP21_REG1_OFFSET 0x1104U
#define FIOPAD_BP19_REG1_OFFSET 0x1108U
#define FIOPAD_BV19_REG1_OFFSET 0x110CU

#define FIOPAD_BV21_REG1_OFFSET 0x1110U
#define FIOPAD_CF67_REG1_OFFSET 0x1114U
#define FIOPAD_CD67_REG1_OFFSET 0x1118U
#define FIOPAD_BY65_REG1_OFFSET 0x111CU

#define FIOPAD_BY67_REG1_OFFSET 0x1120U
#define FIOPAD_CB67_REG1_OFFSET 0x1124U
#define FIOPAD_BV65_REG1_OFFSET 0x1128U

#define FIOPAD_CD65_REG1_OFFSET 0x1190U
#define FIOPAD_CF63_REG1_OFFSET 0x1194U
#define FIOPAD_CH63_REG1_OFFSET 0x1198U
#define FIOPAD_CH65_REG1_OFFSET 0x119CU

#define FIOPAD_REG1_BEG_OFFSET  FIOPAD_CK19_REG1_OFFSET
#define FIOPAD_REG1_END_OFFSET  FIOPAD_CH65_REG1_OFFSET

/* DDMA */

#define FDDMA_MIN_SLAVE_ID      0U
#define FDDMA_MAX_SLAVE_ID      31U

#define FDDMA_INSTANCE_NUM      6U
#define FDDMA5_I2S_ID           5U
#define FDDMA5_BASE_ADDR        0x28001000U
#define FDDMA5_IRQ_NUM          97U
#define FDDMA5_CAPACITY         (1U << 0)

/* I2S */

#define FI2S_NUM                4U
#define FI2S3_ID                3U
#define FI2S3_BASE_ADDR         0x28005000
#define FI2S3_GPIO_BASE_ADDR    0x28005D00
#define FI2S3_IRQ_NUM           100

#define FI2S_CLK_FREQ_HZ        1200000000 /* 1200MHz */

/* ACPI */
#define FACPI_CPU_MAX_FREQ_MHZ  2500U
#define FACPI_CPU_MIN_FREQ_MHZ  625U

/* SCMI and MHU */
#define FSCMI_MHU_BASE_ADDR     0x36d00000
#define FSCMI_MHU_IRQ_ADDR      0x36d00508
#define FSCMI_MHU_IRQ_NUM       (16U + 32U)
#define FSCMI_SHR_MEM_ADDR      0x36d05400
#define FSCMI_MEM_TX_OFSET      0x1400
#define FSCMI_MEM_RX_OFSET      0x1000
#define FSCMI_SHR_MEM_SIZE      0x400

#define FSCMI_MSG_SIZE          128
#define FSCMI_MAX_STR_SIZE      16
#define FSCMI_MAX_CLOCK_RATES   16
#define FSCMI_MAX_NUM_SENSOR    16
#define FSCMI_MAX_PROTOCOLS_IMP 16
#define FSCMI_MAX_PERF_DOMAINS  8
#define FSCMI_MAX_CLOCK_DOMAINS 16
#define FSCMI_MAX_RESET_DOMAINS 16

#define FSCMI_MAX_OPPS          9
#define FSCMI_MAX_POWER_DOMAINS 45

/* PMU */
#define FPMU_IRQ_NUM            23

#ifdef __cplusplus
}
#endif

#endif // !