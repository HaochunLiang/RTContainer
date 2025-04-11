/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * Date: 2024-10-30 13:33:28
 * LastEditTime: 2024-10-30 18:00:50
 * Description:  This file is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#ifndef PD2408_FPARAMETERS_H
#define PD2408_FPARAMETERS_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
#define SOC_TARGET_PD2408

#define CORE0_AFF                      0x000
#define CORE1_AFF                      0x100
#define CORE2_AFF                      0x200
#define CORE3_AFF                      0x300
#define CORE4_AFF                      0x10000
#define CORE5_AFF                      0x10100
#define CORE6_AFF                      0x10200
#define CORE7_AFF                      0x10300
#define CORE_AFF_MASK                  0xFFFFF
#define FCORE_NUM                      8

/* cache */
#define CACHE_LINE_ADDR_MASK           0x3FUL
#define CACHE_LINE                     64U

#define FCPU_CONFIG_ARM64_PA_BITS      44

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

#define GICV3_BASE_ADDR                0x26800000U
#define GICV3_DISTRIBUTOR_BASE_ADDR    (GICV3_BASE_ADDR + 0)
#define GICV3_RD_BASE_ADDR             (GICV3_BASE_ADDR + 0x60000U)
#define GICV3_RD_OFFSET                (2U << 16)
#define GICV3_RD_SIZE                  (16U << 16)

#define GICV3_ITS_BASE_ADDR            0x26840000U

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
#define FPCI_ECAM_MEM64_REG_LENGTH     0xfffffffff

/* SCMI and MHU */
#define FSCMI_MHU_BASE_ADDR            0x26fc0000
#define FSCMI_MHU_IRQ_ADDR             0x26fc0508
#define FSCMI_MHU_IRQ_NUM              (16U + 62U)
#define FSCMI_SHR_MEM_ADDR             0x26fc5400

#define FSCMI_MSG_SIZE                 128
#define FSCMI_MAX_STR_SIZE             16
#define FSCMI_MAX_NUM_SENSOR           16
#define FSCMI_MAX_CLOCK_RATES          16
#define FSCMI_MAX_PROTOCOLS_IMP        16
#define FSCMI_MAX_PERF_DOMAINS         8
#define FSCMI_MAX_OPPS                 14
#define FSCMI_MAX_POWER_DOMAINS        40
#define FSCMI_MAX_CLOCK_DOMAINS        32
#define FSCMI_MAX_RESET_DOMAINS        32

/* XMAC(V2) */
#define FXMAC_MSG_NUM                  1
#define FXMAC0_MSG_ID                  0
#define FXMAC0_MSG_SHARE_MEM_ADDR      0x26FEF000U
#define FXMAC0_MSG_REGFILE_ADDR        0x2701D000U
#define FXMAC0_MSG_QUEUE0_IRQ_NUM      (46U)
#define FXMAC0_MSG_QUEUE1_IRQ_NUM      (47U)
#define FXMAC0_MSG_QUEUE2_IRQ_NUM      (48U)
#define FXMAC0_MSG_QUEUE3_IRQ_NUM      (49U)
#define FXMAC_MSG_PHY_MAX_NUM          32U
#define FXMAC_MSG_CAPS                 0x00000003 /* use */

/* sata controller */
#define FSATA0_ID                      0
#define FSATA_NUM                      1
#define FSATA0_BASE_ADDR               0x26FD4000U

#define FSATA0_IRQ_NUM                 96U

/* DcDp */

#define FDC_ID0                        0
#define FDC_ID1                        1
#define FDC_INSTANCE_NUM               2

#define FDP_ID0                        0
#define FDP_ID1                        1
#define FDP_ID2                        2
#define FDP_INSTANCE_NUM               3

#define FDC0_BASE_OFFSET               0x26CA0000U
#define FDC1_BASE_OFFSET               0x26CA8000U

#define FDP0_BASE_OFFSET               0x26CB6000U
#define FDP1_BASE_OFFSET               0x26CB7000U
#define FDP2_BASE_OFFSET               0x26CB8000U

#define FDP0_PHY_BASE_OFFSET           0x26CB9000U
#define FDP1_PHY_BASE_OFFSET           0x26CBA000U
#define FDP2_PHY_BASE_OFFSET           0x26CBB000U

#define FDC0_IRQ_NUM                   87
#define FDC1_IRQ_NUM                   90

#define FDP0_IRQ_NUM                   97
#define FDP1_IRQ_NUM                   98
#define FDP2_IRQ_NUM                   99

/* DDR */
/* GDMA */
#define FGDMA0_ID                      0U
#define FGDMA0_BASE_ADDR               0x26FD5000U
#define FGDMA0_CHANNEL0_IRQ_NUM        110U
#define FGDMA0_CHANNEL1_IRQ_NUM        111U
#define FGDMA_NUM_OF_CHAN              2
#define FGDMA_INSTANCE_NUM             1U
#define FGDMA0_CAPACITY                (1U << 1)


/* UART */
#define FUART_NUM                      1U
#define FUART_REG_LENGTH               0x18000U

#define FUART1_ID                      1U
#define FUART1_IRQ_NUM                 117U
#define FUART1_BASE_ADDR               0x18002000U
#define FUART1_CLK_FREQ_HZ             100000000U

/* UART(MSG) */
#define FUART_MSG_NUM                  3U
#define FUART0_MSG_ID                  0U
#define FUART0_MSG_IRQ_NUM             116U
#define FUART0_MSG_REGFILE_BASE        0x27011000U
#define FUART0_MSG_SHARE_MEM_ADDR      0x26fe4000U
#define FUART0_MSG_CLK_FREQ_HZ         100000000U

#define FUART1_MSG_ID                  1U
#define FUART1_MSG_IRQ_NUM             117U
#define FUART1_MSG_REGFILE_BASE        0x27012000U
#define FUART1_MSG_SHARE_MEM_ADDR      0x26fe5000U
#define FUART1_MSG_CLK_FREQ_HZ         100000000U

#define FUART2_MSG_ID                  2U
#define FUART2_MSG_IRQ_NUM             118U
#define FUART2_MSG_REGFILE_BASE        0x27013000U
#define FUART2_MSG_SHARE_MEM_ADDR      0x26fe6000U
#define FUART2_MSG_CLK_FREQ_HZ         100000000U

/* SD */
#define FSDIF_NUM                      1U
#define FSDIF0_ID                      0U
#define FSDIF0_BASE_ADDR               0x18005000U
#define FSDIF0_IRQ_NUM                 121U
#define FSDIF_CLK_FREQ_HZ              (1200000000UL) /* 1.2GHz */

/* SD v2 */
#define FSDIF0_MSG_SHARE_MEM_ADDR      0x26fea000U
#define FSDIF0_MSG_REGFILE_ADDR        0x27018000U

/* QSPI */
#define FQSPI_CS_0                     0
#define FQSPI_CS_1                     1
#define FQSPI_CS_2                     2
#define FQSPI_CS_3                     3
#define FQSPI_CS_NUM                   4

#define FQSPI_NUM                      1U
#define FQSPI0_ID                      0U
#define FQSPI_BASE_ADDR                0x18006000U

#define FQSPI_CS_NUM                   4
#define FQSPI_MEM_START_ADDR           0x0U

#define FQSPI_CAP_FLASH_NUM_MASK       GENMASK(17, 16)
#define FQSPI_CAP_FLASH_NUM(data)      ((data) << 16) /* Flash number */

/* I2S */
#define FI2S_CLK_FREQ_HZ               1200000000 /* 1200MHz */

#define FI2S_MSG_NUM                   1
#define FI2S0_MSG_ID                   0
#define FI2S0_MSG_IRQ_NUM              115U

#define FI2S0_MSG_SHARE_MEM_ADDR       0x26FE3000U
#define FI2S0_MSG_REGFILE_ADDR         0x27010000U

/*CODEC FOR MSG*/
#define FCODEC_MSG_NUM                 1
#define FCODEC0_MSG_ID                 0
#define FCODEC0_MSG_SHARE_MEM_ADDR     0x26FE3100
#define FCODEC0_MSG_REGFILE_ADDR       0x27010800
/* DDMA ONLY FOR I2S*/
#define FDDMA_CHANNEL_REG_OFFSET       0x00

#define FDDMA_MIN_SLAVE_ID             0U
#define FDDMA_MAX_SLAVE_ID             31U

#define FDDMA_INSTANCE_NUM             1U
#define FDDMA0_ID                      0U
#define FDDMA0_BASE_ADDR               0x18000000U
#define FDDMA0_IRQ_NUM                 142U
#define FDDMA0_CAPACITY                (1U << 0)

/* I2C */
#define FI2C0_MSG_IRQ_NUM              124U
#define FI2C1_MSG_IRQ_NUM              125U
#define FI2C2_MSG_IRQ_NUM              122U
#define FI2C3_MSG_IRQ_NUM              123U

#define FI2C_CLK_FREQ_HZ               100000000 /* 100MHz */

#define FI2C_MSG_NUM                   4
#define FI2C0_MSG_ID                   0
#define FI2C1_MSG_ID                   1
#define FI2C2_MSG_ID                   2
#define FI2C3_MSG_ID                   3

#define FI2C0_MSG_SHARE_MEM_ADDR       0x26FEB000U
#define FI2C1_MSG_SHARE_MEM_ADDR       0x26FEC000U
#define FI2C2_MSG_SHARE_MEM_ADDR       0x26FED000U
#define FI2C3_MSG_SHARE_MEM_ADDR       0x26FEE000U
#define FI2C0_MSG_REGFILE_ADDR         0x27019000U
#define FI2C1_MSG_REGFILE_ADDR         0x2701A000U
#define FI2C2_MSG_REGFILE_ADDR         0x2701B000U
#define FI2C3_MSG_REGFILE_ADDR         0x2701C000U

/* platform ahci host */
#define PLAT_AHCI_HOST_MAX_COUNT       5
#define AHCI_BASE_0                    0
#define AHCI_BASE_1                    0
#define AHCI_BASE_2                    0
#define AHCI_BASE_3                    0
#define AHCI_BASE_4                    0

#define AHCI_IRQ_0                     0
#define AHCI_IRQ_1                     0
#define AHCI_IRQ_2                     0
#define AHCI_IRQ_3                     0
#define AHCI_IRQ_4                     0

/*  PWM */
#define FPWM_NUM                       1U
#define FPWM0_ID                       0U
#define FPWM_BASE_ADDR                 0x1800E000U
#define FPWM0_IRQ_NUM                  178U
#define FPWM_CLK_FREQ_HZ               50000000U /* 50MHz */
#define FPWM_CHANNEL_0                 0
#define FPWM_CHANNEL_1                 1
#define FPWM_CHANNEL_NUM               2
#define FPWM_CYCLE_PROCESSING(reg_val) ((reg_val)-0x1)

/* LSD Config */
#define FLSD_CONFIG_BASE               0x18016000U
#define FLSD_PWM_HADDR                 0x10

/* TIMER and TACHO */
#define FTIMER_NUM                     1U
#define FTIMER_TACHO_BASE_ADDR(n)      (0x1800F000U + 0x1000U * (n))
#define FTIMER_TACHO_IRQ_NUM(n)        (179U + (n))
#define FTIMER_CLK_FREQ_HZ             50000000ULL /* 50MHz */
#define FTIMER_TICK_PERIOD_NS          20U         /* 20ns */

#if !defined(__ASSEMBLER__)
enum
{
    FTACHO0_ID = 0,
    FTACHO_NUM
};
#endif


#define FSPI0_MSG_IRQ_NUM             119U
#define FSPI1_MSG_IRQ_NUM             120U
#define FSPI_CLK_FREQ_HZ              50000000U

/* SPI for MSG */
#define FSPI_MSG_NUM                  2

#define FSPI0_MSG_ID                  0
#define FSPI0_MSG_SHARE_MEM_BASE_ADDR 0x26fe7000U
#define FSPI0_MSG_REGFILE_BASE_ADDR   0x27014000U

#define FSPI1_MSG_ID                  1
#define FSPI1_MSG_SHARE_MEM_BASR_ADDR 0x26fe8000U
#define FSPI1_MSG_REGFILE_BASE_ADDR   0x27015000U


/* WDT */
#define FWDT0_ID                      0U
#define FWDT_NUM                      1U

#define FWDT0_REFRESH_BASE_ADDR       0x18012000U
#define FWDT1_REFRESH_BASE_ADDR       0x18014000U
#define FWDT_CONTROL_BASE_ADDR(x)     ((x) + 0x1000)

#define FWDT0_IRQ_NUM                 176U
#define FWDT1_IRQ_NUM                 177U

#define FWDT_CLK_FREQ_HZ              50000000U /* 50MHz */

/* GPIO */
#define FGPIO0_BASE_ADDR              0x18007000U
#define FGPIO1_BASE_ADDR              0x18008000U

#define FGPIO_CTRL_0                  0
#define FGPIO_CTRL_1                  1
#define FGPIO_CTRL_NUM                2U

#define FGPIO_PORT_A                  0U
#define FGPIO_PORT_NUM                1U

#define FGPIO_PIN_0                   0U
#define FGPIO_PIN_1                   1U
#define FGPIO_PIN_2                   2U
#define FGPIO_PIN_3                   3U
#define FGPIO_PIN_4                   4U
#define FGPIO_PIN_5                   5U
#define FGPIO_PIN_6                   6U
#define FGPIO_PIN_7                   7U
#define FGPIO_PIN_8                   8U
#define FGPIO_PIN_9                   9U
#define FGPIO_PIN_10                  10U
#define FGPIO_PIN_11                  11U
#define FGPIO_PIN_12                  12U
#define FGPIO_PIN_13                  13U
#define FGPIO_PIN_14                  14U
#define FGPIO_PIN_15                  15U
#define FGPIO_PIN_NUM                 16U

#define FGPIO_NUM                     (FGPIO_CTRL_NUM * FGPIO_PORT_NUM * FGPIO_PIN_NUM)

#define FGPIO_CAP_IRQ_BY_PIN          (1 << 0) /* 支持外部中断，每个引脚有单独上报的中断 */
#define FGPIO_CAP_IRQ_BY_CTRL         (1 << 1) /* 支持外部中断，引脚中断统一上报 */
#define FGPIO_CAP_IRQ_NONE            (1 << 2) /* 不支持外部中断 */

#define FGPIO_ID(ctrl, pin) \
    (((ctrl)*FGPIO_PORT_NUM * FGPIO_PIN_NUM) + ((FGPIO_PORT_A)*FGPIO_PIN_NUM) + (pin))

#define FGPIO_CLK_FREQ_HZ        (50000000UL) /* 50MHz */


/* IOPAD */

#define FIOPAD0_ID               0
#define FIOPAD_NUM               2

/* generic timer */
/* non-secure physical timer int id */
#define GENERIC_TIMER_NS_IRQ_NUM 30U

/* virtual timer int id */
#define GENERIC_VTIMER_IRQ_NUM   27U

#define GENERIC_TIMER_ID0        0 /* non-secure physical timer */
#define GENERIC_TIMER_ID1        1 /* virtual timer */
#define GENERIC_TIMER_NUM        2


/* USB3 (OTG)，支持 USB Host/Device 模式 */
#define FUSB3_OTG_3X2_GEN2_NUM   1U
#define FUSB_ID_0                0U

#define FUSB_0_BASE_ADDR         0x26f00000U
#define FUSB_0_HC_DC_IRQ_NUM     52U
#define FUSB_0_OTG_IRQ_NUM       180U

#define FUSB_OTG_REG_OFF         0x0U
#define FUSB_OTG_DC_REG_OFF      0x4000U
#define FUSB_OTG_HC_REG_OFFSET   0x8000U

/* USB3, USB3.2 Gen2 Host 控制器, 10Gbps */
#define FUSB3_3X2_GEN2_NUM       1U
#define FUSB_ID_1                1U

#define FUSB_1_BASE_ADDR         0x26f20000U
#define FUSB_1_IRQ_NUM           53U

/* USB3, USB3.2 Gen1 Host 控制器, 5Gbps */
#define FUSB3_3X2_GEN1_NUM       2U
#define FUSB_ID_2                2U
#define FUSB_ID_3                3U

#define FUSB_2_BASE_ADDR         0x26f60000U
#define FUSB_3_BASE_ADDR         0x26f40000U
#define FUSB_2_IRQ_NUM           55U
#define FUSB_3_IRQ_NUM           54U

/* USB2, USB2.x Host 控制器，480Mbps */
#define FUSB2_2X0_NUM            2U
#define FUSB_ID_4                4U
#define FUSB_ID_5                5U

#define FUSB_4_BASE_ADDR         0x26f80000U
#define FUSB_5_BASE_ADDR         0x26fa0000U

#define FUSB_4_IRQ_NUM           56U
#define FUSB_5_IRQ_NUM           57U

/* PGU USB, USB 3.2 Gen2 Host 控制器，不兼容 USB 2.x 协议 */
#define FUSB3_PGU_3X2_GEN2_NUM   1U
#define FUSB_ID_6                6U

#define FUSB_6_BASE_ADDR         0x26ee0000U
#define FUSB_6_IRQ_NUM           51U

#define FUSB_TOT_NUM \
    (FUSB3_OTG_3X2_GEN2_NUM + FUSB3_3X2_GEN2_NUM + FUSB3_3X2_GEN1_NUM + FUSB2_2X0_NUM + FUSB3_PGU_3X2_GEN2_NUM)

/*****************************************************************************/
/* register offset of iopad function / pull / driver strength */

#define FIOPAD_BASE_ADDR        0x26FD2000U
#define FIOPAD_REG1_BASE_ADDR   0x26FD3000U

#define FIOPAD_REG0_BEG_OFFSET  FIOPAD_C7_REG0_OFFSET
#define FIOPAD_REG0_END_OFFSET  FIOPAD_T2_REG0_OFFSET

#define FIOPAD_C7_REG0_OFFSET   0x0018U
#define FIOPAD_B4_REG0_OFFSET   0x0024U
#define FIOPAD_B6_REG0_OFFSET   0x0028U
#define FIOPAD_A5_REG0_OFFSET   0x002CU
#define FIOPAD_C3_REG0_OFFSET   0x0030U
#define FIOPAD_B8_REG0_OFFSET   0x0034U
#define FIOPAD_E1_REG0_OFFSET   0x0038U
#define FIOPAD_R3_REG0_OFFSET   0x003CU
#define FIOPAD_P2_REG0_OFFSET   0x0040U
#define FIOPAD_A9_REG0_OFFSET   0x0044U
#define FIOPAD_C9_REG0_OFFSET   0x0048U
#define FIOPAD_F10_REG0_OFFSET  0x004CU
#define FIOPAD_AD6_REG0_OFFSET  0x0050U
#define FIOPAD_AK6_REG0_OFFSET  0x0054U
#define FIOPAD_Y8_REG0_OFFSET   0x0058U
#define FIOPAD_AJ5_REG0_OFFSET  0x005CU
#define FIOPAD_AH6_REG0_OFFSET  0x0060U
#define FIOPAD_AC5_REG0_OFFSET  0x0064U
#define FIOPAD_AB6_REG0_OFFSET  0x0068U
#define FIOPAD_AF6_REG0_OFFSET  0x006CU
#define FIOPAD_AB8_REG0_OFFSET  0x0070U
#define FIOPAD_AA5_REG0_OFFSET  0x0074U
#define FIOPAD_V6_REG0_OFFSET   0x0078U
#define FIOPAD_W5_REG0_OFFSET   0x007CU
#define FIOPAD_Y6_REG0_OFFSET   0x0080U
#define FIOPAD_W3_REG0_OFFSET   0x0084U
#define FIOPAD_U5_REG0_OFFSET   0x0088U
#define FIOPAD_T8_REG0_OFFSET   0x008CU
#define FIOPAD_V8_REG0_OFFSET   0x0090U
#define FIOPAD_P8_REG0_OFFSET   0x0094U
#define FIOPAD_M8_REG0_OFFSET   0x0098U
#define FIOPAD_AE5_REG0_OFFSET  0x009CU
#define FIOPAD_AG5_REG0_OFFSET  0x00A0U
#define FIOPAD_J13_REG0_OFFSET  0x00A4U
#define FIOPAD_L13_REG0_OFFSET  0x00A8U
#define FIOPAD_D10_REG0_OFFSET  0x00ACU
#define FIOPAD_E9_REG0_OFFSET   0x00B0U
#define FIOPAD_BL19_REG0_OFFSET 0x00B4U
#define FIOPAD_BK18_REG0_OFFSET 0x00B8U
#define FIOPAD_BL15_REG0_OFFSET 0x00BCU
#define FIOPAD_BK16_REG0_OFFSET 0x00C0U
#define FIOPAD_BL13_REG0_OFFSET 0x00C4U
#define FIOPAD_BK14_REG0_OFFSET 0x00C8U
#define FIOPAD_J1_REG0_OFFSET   0x00CCU
#define FIOPAD_H2_REG0_OFFSET   0x00D0U
#define FIOPAD_E3_REG0_OFFSET   0x00D4U
#define FIOPAD_F2_REG0_OFFSET   0x00D8U
#define FIOPAD_G3_REG0_OFFSET   0x00DCU
#define FIOPAD_E5_REG0_OFFSET   0x00E0U
#define FIOPAD_F6_REG0_OFFSET   0x00E4U
#define FIOPAD_E7_REG0_OFFSET   0x00E8U
#define FIOPAD_F8_REG0_OFFSET   0x00ECU
#define FIOPAD_N3_REG0_OFFSET   0x00F0U
#define FIOPAD_N1_REG0_OFFSET   0x00F4U
#define FIOPAD_M2_REG0_OFFSET   0x00F8U
#define FIOPAD_K2_REG0_OFFSET   0x00FCU
#define FIOPAD_K8_REG0_OFFSET   0x0100U
#define FIOPAD_M6_REG0_OFFSET   0x0104U
#define FIOPAD_H6_REG0_OFFSET   0x0108U
#define FIOPAD_K6_REG0_OFFSET   0x010CU
#define FIOPAD_J5_REG0_OFFSET   0x0110U
#define FIOPAD_J3_REG0_OFFSET   0x0114U
#define FIOPAD_G5_REG0_OFFSET   0x0118U
#define FIOPAD_L5_REG0_OFFSET   0x011CU
#define FIOPAD_L3_REG0_OFFSET   0x0120U
#define FIOPAD_AP4_REG0_OFFSET  0x0124U
#define FIOPAD_AE1_REG0_OFFSET  0x0128U
#define FIOPAD_AG3_REG0_OFFSET  0x012CU
#define FIOPAD_AJ3_REG0_OFFSET  0x0130U
#define FIOPAD_AD2_REG0_OFFSET  0x0134U
#define FIOPAD_AM4_REG0_OFFSET  0x0138U
#define FIOPAD_AR5_REG0_OFFSET  0x013CU
#define FIOPAD_AT6_REG0_OFFSET  0x0140U
#define FIOPAD_AC3_REG0_OFFSET  0x0144U
#define FIOPAD_AE3_REG0_OFFSET  0x0148U
#define FIOPAD_AF2_REG0_OFFSET  0x014CU
#define FIOPAD_AH2_REG0_OFFSET  0x0150U
#define FIOPAD_AJ1_REG0_OFFSET  0x0154U
#define FIOPAD_AK2_REG0_OFFSET  0x0158U
#define FIOPAD_BM12_REG0_OFFSET 0x015CU
#define FIOPAD_BM10_REG0_OFFSET 0x0160U
#define FIOPAD_BK12_REG0_OFFSET 0x0164U
#define FIOPAD_BL11_REG0_OFFSET 0x0168U
#define FIOPAD_BK20_REG0_OFFSET 0x016CU
#define FIOPAD_BL21_REG0_OFFSET 0x0170U
#define FIOPAD_BK22_REG0_OFFSET 0x0174U
#define FIOPAD_BL23_REG0_OFFSET 0x0178U
#define FIOPAD_BK24_REG0_OFFSET 0x017CU
#define FIOPAD_BL25_REG0_OFFSET 0x0180U
#define FIOPAD_AA3_REG0_OFFSET  0x0184U
#define FIOPAD_Y2_REG0_OFFSET   0x0188U
#define FIOPAD_V2_REG0_OFFSET   0x018CU
#define FIOPAD_AA1_REG0_OFFSET  0x0190U
#define FIOPAD_AB2_REG0_OFFSET  0x0194U
#define FIOPAD_T2_REG0_OFFSET   0x0198U

/* register offset of iopad delay */

#define FIOPAD_C7_REG1_OFFSET   0x1018U
#define FIOPAD_B4_REG1_OFFSET   0x1024U
#define FIOPAD_B6_REG1_OFFSET   0x1028U
#define FIOPAD_D10_REG1_OFFSET  0x10ACU
#define FIOPAD_E9_REG1_OFFSET   0x10B0U
#define FIOPAD_J1_REG1_OFFSET   0x10CCU
#define FIOPAD_H2_REG1_OFFSET   0x10D0U
#define FIOPAD_E3_REG1_OFFSET   0x10D4U
#define FIOPAD_F2_REG1_OFFSET   0x10D8U
#define FIOPAD_G3_REG1_OFFSET   0x10DCU
#define FIOPAD_E5_REG1_OFFSET   0x10E0U
#define FIOPAD_F6_REG1_OFFSET   0x10E4U
#define FIOPAD_E7_REG1_OFFSET   0x10E8U
#define FIOPAD_K8_REG1_OFFSET   0x1100U
#define FIOPAD_M6_REG1_OFFSET   0x1104U
#define FIOPAD_H6_REG1_OFFSET   0x1108U
#define FIOPAD_K6_REG1_OFFSET   0x110CU
#define FIOPAD_J5_REG1_OFFSET   0x1110U
#define FIOPAD_J3_REG1_OFFSET   0x1114U
#define FIOPAD_G5_REG1_OFFSET   0x1118U
#define FIOPAD_L5_REG1_OFFSET   0x111CU
#define FIOPAD_L3_REG1_OFFSET   0x1120U
#define FIOPAD_AP4_REG1_OFFSET  0x1124U
#define FIOPAD_AE1_REG1_OFFSET  0x1128U
#define FIOPAD_AG3_REG1_OFFSET  0x112CU
#define FIOPAD_AJ3_REG1_OFFSET  0x1130U
#define FIOPAD_AD2_REG1_OFFSET  0x1134U
#define FIOPAD_AM4_REG1_OFFSET  0x1138U
#define FIOPAD_AE3_REG1_OFFSET  0x1148U
#define FIOPAD_AF2_REG1_OFFSET  0x114CU
#define FIOPAD_AH2_REG1_OFFSET  0x1150U
#define FIOPAD_AJ1_REG1_OFFSET  0x1154U


#define FIOPAD_REG1_BEG_OFFSET  FIOPAD_C7_REG1_OFFSET
#define FIOPAD_REG1_END_OFFSET  FIOPAD_AJ1_REG1_OFFSET

/* PMU */
#define FPMU_IRQ_NUM            23

#ifdef __cplusplus
}

#endif

#endif