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
 * FilePath: fparameters_comm.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 18:01:11
 * Description:  This file is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#ifndef E2000_FPARAMETERS_COMMON_H
#define E2000_FPARAMETERS_COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

#define SOC_TARGET_PE220X
/************************** Constant Definitions *****************************/
/* CACHE */
#define CACHE_LINE_ADDR_MASK           0x3FUL
#define CACHE_LINE                     64U

/* DEVICE Register Address */
#define FT_DEV_BASE_ADDR               0x28000000U
#define FT_DEV_END_ADDR                0x2FFFFFFFU

/* PCIE */

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
#define FPCIEC_INSTANCE_NUM            6

#define FPCIEC_INSTANCE0_INDEX         0 /* psu.c0 */
#define FPCIEC_INSTANCE1_INDEX         1 /* psu.c1 */
#define FPCIEC_INSTANCE2_INDEX         2 /* peu.c0 */
#define FPCIEC_INSTANCE3_INDEX         3 /* peu.c1 */
#define FPCIEC_INSTANCE4_INDEX         4 /* peu.c2 */
#define FPCIEC_INSTANCE5_INDEX         5 /* peu.c3 */

#define FPCIEC_INSTANCE0_MISC_IRQ_NUM  42

#define FPCIEC_INSTANCE3_MISC_IRQ_NUM  40

#define FPCIEC_INSTANCE0_NUM           0 /* psu.c0 */
#define FPCIEC_INSTANCE1_NUM           1 /* psu.c1 */
#define FPCIEC_INSTANCE2_NUM           0 /* peu.c0 */
#define FPCIEC_INSTANCE3_NUM           1 /* peu.c1 */
#define FPCIEC_INSTANCE4_NUM           2 /* peu.c2 */
#define FPCIEC_INSTANCE5_NUM           3 /* peu.c3 */

#define FPCIEC_INSTANCE0_CONTROL_BASE  0x31000000U
#define FPCIEC_INSTANCE1_CONTROL_BASE  0x31020000U
#define FPCIEC_INSTANCE2_CONTROL_BASE  0x31040000U
#define FPCIEC_INSTANCE3_CONTROL_BASE  0x31060000U
#define FPCIEC_INSTANCE4_CONTROL_BASE  0x31080000U
#define FPCIEC_INSTANCE5_CONTROL_BASE  0x310a0000U

#define FPCIEC_INSTANCE0_CONFIG_BASE   0x31100000U
#define FPCIEC_INSTANCE1_CONFIG_BASE   0x31100000U
#define FPCIEC_INSTANCE2_CONFIG_BASE   0x31101000U
#define FPCIEC_INSTANCE3_CONFIG_BASE   0x31101000U
#define FPCIEC_INSTANCE4_CONFIG_BASE   0x31101000U
#define FPCIEC_INSTANCE5_CONFIG_BASE   0x31101000U

#define FPCIEC_INSTANCE0_DMA_NUM       2
#define FPCIEC_INSTANCE1_DMA_NUM       0
#define FPCIEC_INSTANCE2_DMA_NUM       2
#define FPCIEC_INSTANCE3_DMA_NUM       0
#define FPCIEC_INSTANCE4_DMA_NUM       0
#define FPCIEC_INSTANCE5_DMA_NUM       0

#define FPCIEC_INSTANCE0_DMABASE       (0x31000000U + 0x400U)
#define FPCIEC_INSTANCE1_DMABASE       (0U)
#define FPCIEC_INSTANCE2_DMABASE       (0x31040000U + 0x400U)
#define FPCIEC_INSTANCE3_DMABASE       (0U)
#define FPCIEC_INSTANCE4_DMABASE       (0U)
#define FPCIEC_INSTANCE5_DMABASE       (0U)

#define FPCIEC_MAX_OUTBOUND_NUM        8


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


/* SCMI and MHU */
#define FSCMI_MHU_BASE_ADDR            0x32a00000
#define FSCMI_MHU_IRQ_ADDR             0x32a00508
#define FSCMI_MHU_IRQ_NUM              (22U + 32U)
#define FSCMI_SHR_MEM_ADDR             0x32a11400
#define FSCMI_MEM_TX_OFSET             0x1400
#define FSCMI_MEM_RX_OFSET             0x1000
#define FSCMI_SHR_MEM_SIZE             0x400

#define FSCMI_MSG_SIZE                 128
#define FSCMI_MAX_STR_SIZE             16
#define FSCMI_MAX_CLOCK_RATES          16
#define FSCMI_MAX_NUM_SENSOR           16
#define FSCMI_MAX_PROTOCOLS_IMP        16
#define FSCMI_MAX_PERF_DOMAINS         3
#define FSCMI_MAX_CLOCK_DOMAINS        4
#define FSCMI_MAX_RESET_DOMAINS        1
#define FSCMI_MAX_OPPS                 4
#define FSCMI_MAX_POWER_DOMAINS        11

/* UART */
#define FUART_NUM                      4U
#define FUART_REG_LENGTH               0x18000U

#define FUART0_ID                      0U
#define FUART0_IRQ_NUM                 (85 + 30)
#define FUART0_BASE_ADDR               0x2800c000U
#define FUART0_CLK_FREQ_HZ             100000000U

#define FUART1_ID                      1U
#define FUART1_IRQ_NUM                 (86 + 30)
#define FUART1_BASE_ADDR               0x2800d000U
#define FUART1_CLK_FREQ_HZ             100000000U

#define FUART2_ID                      2U
#define FUART2_IRQ_NUM                 (87 + 30)
#define FUART2_BASE_ADDR               0x2800e000U
#define FUART2_CLK_FREQ_HZ             100000000U

#define FUART3_BASE_ADDR               0x2800f000U
#define FUART3_ID                      3U
#define FUART3_IRQ_NUM                 (88 + 30)
#define FUART3_CLK_FREQ_HZ             100000000U

#define FT_STDOUT_BASE_ADDR            FUART1_BASE_ADDR
#define FT_STDIN_BASE_ADDR             FUART1_BASE_ADDR

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

#define GICV3_BASE_ADDR                0x30800000U
#define GICV3_ITS_BASE_ADDR            (GICV3_BASE_ADDR + 0x20000U)
#define GICV3_DISTRIBUTOR_BASE_ADDR    (GICV3_BASE_ADDR + 0)
#define GICV3_RD_BASE_ADDR             (GICV3_BASE_ADDR + 0x80000U)
#define GICV3_RD_OFFSET                (2U << 16)
#define GICV3_RD_SIZE                  (8U << 16)

/* GPIO */
#define FGPIO0_BASE_ADDR               0x28034000U
#define FGPIO1_BASE_ADDR               0x28035000U
#define FGPIO2_BASE_ADDR               0x28036000U
#define FGPIO3_BASE_ADDR               0x28037000U
#define FGPIO4_BASE_ADDR               0x28038000U
#define FGPIO5_BASE_ADDR               0x28039000U

#define FGPIO_CTRL_0                   0
#define FGPIO_CTRL_1                   1
#define FGPIO_CTRL_2                   2
#define FGPIO_CTRL_3                   3
#define FGPIO_CTRL_4                   4
#define FGPIO_CTRL_5                   5
#define FGPIO_CTRL_NUM                 6U

#define FGPIO_PORT_A                   0U
#define FGPIO_PORT_NUM                 1U

#define FGPIO_PIN_0                    0U
#define FGPIO_PIN_1                    1U
#define FGPIO_PIN_2                    2U
#define FGPIO_PIN_3                    3U
#define FGPIO_PIN_4                    4U
#define FGPIO_PIN_5                    5U
#define FGPIO_PIN_6                    6U
#define FGPIO_PIN_7                    7U
#define FGPIO_PIN_8                    8U
#define FGPIO_PIN_9                    9U
#define FGPIO_PIN_10                   10U
#define FGPIO_PIN_11                   11U
#define FGPIO_PIN_12                   12U
#define FGPIO_PIN_13                   13U
#define FGPIO_PIN_14                   14U
#define FGPIO_PIN_15                   15U
#define FGPIO_PIN_NUM                  16U

#define FGPIO_NUM                      (FGPIO_CTRL_NUM * FGPIO_PORT_NUM * FGPIO_PIN_NUM)

#define FGPIO_CAP_IRQ_BY_PIN           (1 << 0) /* 支持外部中断，每个引脚有单独上报的中断 */
#define FGPIO_CAP_IRQ_BY_CTRL          (1 << 1) /* 支持外部中断，引脚中断统一上报 */
#define FGPIO_CAP_IRQ_NONE             (1 << 2) /* 不支持外部中断 */

#define FGPIO_ID(ctrl, pin) \
    (((ctrl)*FGPIO_PORT_NUM * FGPIO_PIN_NUM) + ((FGPIO_PORT_A)*FGPIO_PIN_NUM) + (pin))

/* SPI */
#define FSPI0_BASE_ADDR               0x2803A000U
#define FSPI1_BASE_ADDR               0x2803B000U
#define FSPI2_BASE_ADDR               0x2803C000U
#define FSPI3_BASE_ADDR               0x2803D000U
#define FSPI0_ID                      0U
#define FSPI1_ID                      1U
#define FSPI2_ID                      2U
#define FSPI3_ID                      3U

#define FSPI0_IRQ_NUM                 191U
#define FSPI1_IRQ_NUM                 192U
#define FSPI2_IRQ_NUM                 193U
#define FSPI3_IRQ_NUM                 194U

#define FSPI_CLK_FREQ_HZ              50000000U
#define FSPI_DEFAULT_SCLK             5000000U
#define FSPI_NUM                      4U

#define FSPI_DMA_CAPACITY             BIT(0)
#define FSPIM0_DMA_CAPACITY           FSPI_DMA_CAPACITY
#define FSPIM1_DMA_CAPACITY           FSPI_DMA_CAPACITY
#define FSPIM2_DMA_CAPACITY           FSPI_DMA_CAPACITY
#define FSPIM3_DMA_CAPACITY           FSPI_DMA_CAPACITY

/* XMAC */
#define FXMAC_NUM                     4U

#define FXMAC0_ID                     0U
#define FXMAC1_ID                     1U
#define FXMAC2_ID                     2U
#define FXMAC3_ID                     3U

#define FXMAC0_BASE_ADDR              0x3200C000U
#define FXMAC1_BASE_ADDR              0x3200E000U
#define FXMAC2_BASE_ADDR              0x32010000U
#define FXMAC3_BASE_ADDR              0x32012000U

#define FXMAC0_MODE_SEL_BASE_ADDR     0x3200DC00U
#define FXMAC0_LOOPBACK_SEL_BASE_ADDR 0x3200DC04U
#define FXMAC1_MODE_SEL_BASE_ADDR     0x3200FC00U
#define FXMAC1_LOOPBACK_SEL_BASE_ADDR 0x3200FC04U
#define FXMAC2_MODE_SEL_BASE_ADDR     0x32011C00U
#define FXMAC2_LOOPBACK_SEL_BASE_ADDR 0x32011C04U
#define FXMAC3_MODE_SEL_BASE_ADDR     0x32013C00U
#define FXMAC3_LOOPBACK_SEL_BASE_ADDR 0x32013C04U

#define FXMAC0_PCLK                   50000000U
#define FXMAC1_PCLK                   50000000U
#define FXMAC2_PCLK                   50000000U
#define FXMAC3_PCLK                   50000000U
#define FXMAC0_HOTPLUG_IRQ_NUM        (53U + 30U)
#define FXMAC1_HOTPLUG_IRQ_NUM        (54U + 30U)
#define FXMAC2_HOTPLUG_IRQ_NUM        (55U + 30U)
#define FXMAC3_HOTPLUG_IRQ_NUM        (56U + 30U)

#define FXMAC_QUEUE_MAX_NUM           16U

#define FXMAC0_QUEUE0_IRQ_NUM         (57U + 30U)
#define FXMAC0_QUEUE1_IRQ_NUM         (58U + 30U)
#define FXMAC0_QUEUE2_IRQ_NUM         (59U + 30U)
#define FXMAC0_QUEUE3_IRQ_NUM         (60U + 30U)
#define FXMAC0_QUEUE4_IRQ_NUM         (30U + 30U)
#define FXMAC0_QUEUE5_IRQ_NUM         (31U + 30U)
#define FXMAC0_QUEUE6_IRQ_NUM         (32U + 30U)
#define FXMAC0_QUEUE7_IRQ_NUM         (33U + 30U)

#define FXMAC1_QUEUE0_IRQ_NUM         (61U + 30U)
#define FXMAC1_QUEUE1_IRQ_NUM         (62U + 30U)
#define FXMAC1_QUEUE2_IRQ_NUM         (63U + 30U)
#define FXMAC1_QUEUE3_IRQ_NUM         (64U + 30U)

#define FXMAC2_QUEUE0_IRQ_NUM         (66U + 30U)
#define FXMAC2_QUEUE1_IRQ_NUM         (67U + 30U)
#define FXMAC2_QUEUE2_IRQ_NUM         (68U + 30U)
#define FXMAC2_QUEUE3_IRQ_NUM         (69U + 30U)

#define FXMAC3_QUEUE0_IRQ_NUM         (70U + 30U)
#define FXMAC3_QUEUE1_IRQ_NUM         (71U + 30U)
#define FXMAC3_QUEUE2_IRQ_NUM         (72U + 30U)
#define FXMAC3_QUEUE3_IRQ_NUM         (73U + 30U)

#define FXMAC_PHY_MAX_NUM             32U

#define FXMAC_CLK_TYPE_0

/* IOPAD */
#define FIOPAD0_ID                0
#define FIOPAD_NUM                1

/* QSPI */
#define FQSPI0_ID                 0
#define FQSPI_NUM                 1

/* FQSPI cs 0_3, chip number */

#define FQSPI_CS_0                0
#define FQSPI_CS_1                1
#define FQSPI_CS_2                2
#define FQSPI_CS_3                3
#define FQSPI_CS_NUM              4
#define FQSPI_BASE_ADDR           0x028008000U

#define FQSPI_MEM_START_ADDR      0x0U
#define FQSPI_MEM_END_ADDR        0x0FFFFFFFU /* 256MB */
#define FQSPI_MEM_START_ADDR_64   0x100000000U
#define FQSPI_MEM_END_ADDR_64     0x17FFFFFFFU /* 2GB */
#define FQSPI_CAP_FLASH_NUM_MASK  GENMASK(4, 3)
#define FQSPI_CAP_FLASH_NUM(data) ((data) << 3) /* Flash number */

/* TIMER and TACHO */
#define FTIMER_NUM                38U
#define FTIMER_CLK_FREQ_HZ        50000000ULL /* 50MHz */
#define FTIMER_TACHO_IRQ_NUM(n)   (226U + (n))
#define FTIMER_TACHO_BASE_ADDR(n) (0x28054000U + 0x1000U * (n))

/* GDMA */
#define FGDMA0_ID                 0U
#define FGDMA0_BASE_ADDR          0x32B34000U
#define FGDMA0_CHANNEL0_IRQ_NUM   266U
#define FGDMA_NUM_OF_CHAN         16
#define FGDMA_INSTANCE_NUM        1U
#define FGDMA0_CAPACITY           (1U << 0)

/* WDT */

#define FWDT0_ID                  0
#define FWDT1_ID                  1
#define FWDT_NUM                  2

#define FWDT0_REFRESH_BASE_ADDR   0x28040000U
#define FWDT1_REFRESH_BASE_ADDR   0x28042000U

#define FWDT_CONTROL_BASE_ADDR(x) ((x) + 0x1000)

#define FWDT0_IRQ_NUM             196U
#define FWDT1_IRQ_NUM             197U

#define FWDT_CLK_FREQ_HZ          48000000U /* 48MHz */

/*MIO*/
#define FMIO_BASE_ADDR(n)         (0x28014000 + 0x2000 * (n))
#define FMIO_CONF_ADDR(n)         FMIO_BASE_ADDR(n) + 0x1000
#define FMIO_IRQ_NUM(n)           (124 + n)
#define FMIO_CLK_FREQ_HZ          50000000 /* 50MHz */

/* NAND */
#define FNAND_NUM                 1U
#define FNAND_INSTANCE0           0U
#define FNAND_BASE_ADDR           0x28002000U
#define FNAND_IRQ_NUM             (106U)
#define FNAND_CONNECT_MAX_NUM     1U

#define FIOPAD_BASE_ADDR          0x32B30000U

/* DDMA */
#define FDDMA0_ID                 0U
#define FDDMA0_BASE_ADDR          0x28003000U
#define FDDMA0_IRQ_NUM            107U
#define FDDMA0_CAPACITY           (1U << 1)

#define FDDMA1_ID                 1U
#define FDDMA1_BASE_ADDR          0x28004000U
#define FDDMA1_IRQ_NUM            108U
#define FDDMA1_CAPACITY           (1U << 1)

#define FDDMA2_I2S_ID             2U
#define FDDMA2_BASE_ADDR          0x28005000U
#define FDDMA2_IRQ_NUM            109U
#define FDDMA2_CAPACITY           (1U << 0)

#define FDDMA3_DP0_I2S_ID         3U
#define FDDMA3_BASE_ADDR          0x32008000U
#define FDDMA3_IRQ_NUM            79U
#define FDDMA3_CAPACITY           (1U << 0)

#define FDDMA4_DP1_I2S_ID         4U
#define FDDMA4_BASE_ADDR          0x3200A000U
#define FDDMA4_IRQ_NUM            80U
#define FDDMA4_CAPACITY           (1U << 0)

#define FDDMA_INSTANCE_NUM        5U

#define FDDMA0_CAN0_TX_SLAVE_ID   0U /* can0 tx slave-id */
#define FDDMA0_CAN1_TX_SLAVE_ID   1U /* can1 tx slave-id */
#define FDDMA0_UART0_TX_SLAVE_ID  2U /* uart0 tx slave-id */
#define FDDMA0_UART1_TX_SLAVE_ID  3U /* uart1 tx slave-id */
#define FDDMA0_UART2_TX_SLAVE_ID  4U /* uart2 tx slave-id */
#define FDDMA0_UART3_TX_SLAVE_ID  5U /* uart3 tx slave-id */

#define FDDMA0_SPIM0_TX_SLAVE_ID  6U /* spi0 tx slave-id */
#define FDDMA0_SPIM1_TX_SLAVE_ID  7U /* spi1 tx slave-id */
#define FDDMA0_SPIM2_TX_SLAVE_ID  8U /* spi2 tx slave-id */
#define FDDMA0_SPIM3_TX_SLAVE_ID  9U /* spi3 tx slave-id */

#define FDDMA0_CAN0_RX_SLAVE_ID   13U /* can0 rx slave-id */
#define FDDMA0_CAN1_RX_SLAVE_ID   14U /* can1 rx slave-id */
#define FDDMA0_UART0_RX_SLAVE_ID  15U /* uart0 rx slave-id */
#define FDDMA0_UART1_RX_SLAVE_ID  16U /* uart1 rx slave-id */
#define FDDMA0_UART2_RX_SLAVE_ID  17U /* uart2 rx slave-id */
#define FDDMA0_UART3_RX_SLAVE_ID  18U /* uart3 rx slave-id */

#define FDDMA0_SPIM0_RX_SLAVE_ID  19U /* spi0 rx slave-id */
#define FDDMA0_SPIM1_RX_SLAVE_ID  20U /* spi1 rx slave-id */
#define FDDMA0_SPIM2_RX_SLAVE_ID  21U /* spi2 rx slave-id */
#define FDDMA0_SPIM3_RX_SLAVE_ID  22U /* spi3 rx slave-id */

/* FDDMA1_ID */
#define FDDMA1_MIO0_TX_SLAVE_ID   0U  /* mio0 rx slave-id */
#define FDDMA1_MIO1_TX_SLAVE_ID   1U  /* mio1 rx slave-id */
#define FDDMA1_MIO2_TX_SLAVE_ID   2U  /* mio2 rx slave-id */
#define FDDMA1_MIO3_TX_SLAVE_ID   3U  /* mio3 rx slave-id */
#define FDDMA1_MIO4_TX_SLAVE_ID   4U  /* mio4 rx slave-id */
#define FDDMA1_MIO5_TX_SLAVE_ID   5U  /* mio5 rx slave-id */
#define FDDMA1_MIO6_TX_SLAVE_ID   6U  /* mio6 rx slave-id */
#define FDDMA1_MIO7_TX_SLAVE_ID   7U  /* mio7 rx slave-id */
#define FDDMA1_MIO8_TX_SLAVE_ID   8U  /* mio8 rx slave-id */
#define FDDMA1_MIO9_TX_SLAVE_ID   9U  /* mio9 rx slave-id */
#define FDDMA1_MIO10_TX_SLAVE_ID  10U /* mio10 rx slave-id */
#define FDDMA1_MIO11_TX_SLAVE_ID  11U /* mio11 rx slave-id */
#define FDDMA1_MIO12_TX_SLAVE_ID  12U /* mio12 rx slave-id */
#define FDDMA1_MIO13_TX_SLAVE_ID  13U /* mio13 rx slave-id */
#define FDDMA1_MIO14_TX_SLAVE_ID  14U /* mio14 rx slave-id */
#define FDDMA1_MIO15_TX_SLAVE_ID  15U /* mio15 rx slave-id */

#define FDDMA1_MIO0_RX_SLAVE_ID   16U /* mio0 tx slave-id */
#define FDDMA1_MIO1_RX_SLAVE_ID   17U /* mio1 tx slave-id */
#define FDDMA1_MIO2_RX_SLAVE_ID   18U /* mio2 tx slave-id */
#define FDDMA1_MIO3_RX_SLAVE_ID   19U /* mio3 tx slave-id */
#define FDDMA1_MIO4_RX_SLAVE_ID   20U /* mio4 tx slave-id */
#define FDDMA1_MIO5_RX_SLAVE_ID   21U /* mio5 tx slave-id */
#define FDDMA1_MIO6_RX_SLAVE_ID   22U /* mio6 tx slave-id */
#define FDDMA1_MIO7_RX_SLAVE_ID   23U /* mio7 tx slave-id */
#define FDDMA1_MIO8_RX_SLAVE_ID   24U /* mio8 tx slave-id */
#define FDDMA1_MIO9_RX_SLAVE_ID   25U /* mio9 tx slave-id */
#define FDDMA1_MIO10_RX_SLAVE_ID  26U /* mio10 tx slave-id */
#define FDDMA1_MIO11_RX_SLAVE_ID  27U /* mio11 tx slave-id */
#define FDDMA1_MIO12_RX_SLAVE_ID  28U /* mio12 tx slave-id */
#define FDDMA1_MIO13_RX_SLAVE_ID  29U /* mio13 tx slave-id */
#define FDDMA1_MIO14_RX_SLAVE_ID  30U /* mio14 tx slave-id */
#define FDDMA1_MIO15_RX_SLAVE_ID  31U /* mio15 tx slave-id */

#define FDDMA_MIN_SLAVE_ID        0U
#define FDDMA_MAX_SLAVE_ID        31U

/* Semaphore */
#define FSEMA0_ID                 0U
#define FSEMA0_BASE_ADDR          0x32B36000U
#define FSEMA_INSTANCE_NUM        1U

/* LSD Config */
#define FLSD_CONFIG_BASE          0x2807E000U
#define FLSD_NAND_MMCSD_HADDR     0xC0U
#define FLSD_PWM_HADDR            0x20U

/* USB3 */
#define FUSB3_ID_0                0U
#define FUSB3_ID_1                1U
#define FUSB3_NUM                 2U
#define FUSB3_XHCI_OFFSET         0x8000U
#define FUSB3_0_BASE_ADDR         0x31A00000U
#define FUSB3_1_BASE_ADDR         0x31A20000U
#define FUSB3_0_IRQ_NUM           48U
#define FUSB3_1_IRQ_NUM           49U

/* DcDp */


#define FDCDP_ID0                 0
#define FDCDP_ID1                 1

#define FDP_INSTANCE_NUM          2
#define FDC_INSTANCE_NUM          2

#define FDC_CTRL_BASE_OFFSET      0x32000000U

#define FDC0_CHANNEL_BASE_OFFSET  0x32000000U
#define FDC1_CHANNEL_BASE_OFFSET  (FDC0_CHANNEL_BASE_OFFSET + 0x1000U)

#define FDP0_CHANNEL_BASE_OFFSET  0x32004000U
#define FDP1_CHANNEL_BASE_OFFSET  (FDP0_CHANNEL_BASE_OFFSET + 0x1000U)

#define FDP0_PHY_BASE_OFFSET      0x32300000U
#define FDP1_PHY_BASE_OFFSET      (FDP0_PHY_BASE_OFFSET + 0x100000U)

#define FDCDP_IRQ_NUM             76

/* generic timer */
/* non-secure physical timer int id */
#define GENERIC_TIMER_NS_IRQ_NUM  30U

/* virtual timer int id */
#define GENERIC_VTIMER_IRQ_NUM    27U


#define GENERIC_TIMER_ID0         0 /* non-secure physical timer */
#define GENERIC_TIMER_ID1         1 /* virtual timer */
#define GENERIC_TIMER_NUM         2


/* PMU */
#define FPMU_IRQ_NUM              23

/* USB2 OTG */
#if !defined(__ASSEMBLER__)

enum
{
    FUSB2_ID_VHUB_0 = 0,
    FUSB2_ID_1,
    FUSB2_ID_2,
    FUSB2_ID_VHUB_1,
    FUSB2_ID_VHUB_2,

    FUSB2_INSTANCE_NUM
};

#endif

#define FUSB2_0_VHUB_BASE_ADDR      0x31800000U
#define FUSB2_1_VHUB_BASE_ADDR      0x31880000U
#define FUSB2_2_VHUB_BASE_ADDR      0x31900000U

#define FUSB2_VHUB_CFG_BASE_ADDR    0x319C0000U
#define FUSB2_DEV_1_CFG_BASE_ADDR   0x31990000U
#define FUSB2_DEV_2_CFG_BASE_ADDR   0x319A0000U
#define FUSB2_DEV_3_CFG_BASE_ADDR   0x319B0000U

#define FUSB2_0_VHUB_IRQ_NUM        64U
#define FUSB2_1_VHUB_IRQ_NUM        65U
#define FUSB2_2_VHUB_IRQ_NUM        66U

#define FUSB2_0_VHUB_WAKEUP_IRQ_NUM 67U
#define FUSB2_1_VHUB_WAKEUP_IRQ_NUM 68U
#define FUSB2_2_VHUB_WAKEUP_IRQ_NUM 69U

#define FUSB2_1_BASE_ADDR           0x32800000U
#define FUSB2_2_BASE_ADDR           0x32840000U
#define FUSB2_1_UIB_BASE_ADDR       0x32880000U
#define FUSB2_2_UIB_BASE_ADDR       0x328C0000U
#define FUSB2_MMIO_SIZE             0x00200000U /* 2MB */

#define FUSB2_1_IRQ_NUM             46U
#define FUSB2_2_IRQ_NUM             47U

#define FUSB2_1_WAKEUP_IRQ_NUM      50U
#define FUSB2_2_WAKEUP_IRQ_NUM      51U


/*****************************************************************************/

#ifdef __cplusplus
}

#endif

#endif