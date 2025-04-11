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
 * Date: 2022-02-11 13:33:28
 * LastEditTime: 2022-02-17 18:00:50
 * Description:  This file is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#ifndef E2000Q_FPARAMETERS_H
#define E2000Q_FPARAMETERS_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "fparameters_comm.h"

/************************** Constant Definitions *****************************/
#define SOC_TARGET_PE2204

#define CORE0_AFF                      0x000U
#define CORE1_AFF                      0x100U
#define CORE2_AFF                      0x200U
#define CORE3_AFF                      0x201U
#define CORE_AFF_MASK                  0xFFF
#define FCORE_NUM                      4

#define FCPU_CONFIG_ARM64_PA_BITS      44

/* PWM */
#define FPWM0_ID                       0
#define FPWM1_ID                       1
#define FPWM_NUM                       2

#define FPWM_CHANNEL_0                 0
#define FPWM_CHANNEL_1                 1
#define FPWM_CHANNEL_NUM               2

#define FPWM_BASE_ADDR                 0x2804A000U

#define FPWM_CLK_FREQ_HZ               50000000U /* 50MHz */

#define FPWM0_IRQ_NUM                  205U
#define FPWM1_IRQ_NUM                  206U
#define FPWM2_IRQ_NUM                  207U
#define FPWM3_IRQ_NUM                  208U

#define FPWM_CYCLE_PROCESSING(reg_val) ((reg_val)-0x1)

/* I2S */

#define FI2S0_ID                       0U
#define FI2S1_ID                       1U
#define FI2S2_ID                       2U
#define FI2S_NUM                       3U

#define FI2S0_BASE_ADDR                0x28009000
#define FI2S0_IRQ_NUM                  112

#define FI2S1_BASE_ADDR                0x32009000 /*i2s for dp0*/
#define FI2S1_IRQ_NUM                  77

#define FI2S2_BASE_ADDR                0x3200B000 /*i2s for dp1*/
#define FI2S2_IRQ_NUM                  78

#define FI2S_CLK_FREQ_HZ               600000000 /* 600MHz */

/* sata controller */
#define FSATA0_BASE_ADDR               0x31A40000U
#define FSATA1_BASE_ADDR               0x32014000U

#define FSATA0_IRQ_NUM                 74
#define FSATA1_IRQ_NUM                 75

#define FSATA0_ID                      0
#define FSATA1_ID                      1
#define FSATA_NUM                      2

/* CANFD */
#define FCAN_CLK_FREQ_HZ               200000000U

#define FCAN0_BASE_ADDR                0x2800A000U
#define FCAN1_BASE_ADDR                0x2800B000U

#define FCAN0_IRQ_NUM                  113U
#define FCAN1_IRQ_NUM                  114U

#define FCAN0_ID                       0
#define FCAN1_ID                       1
#define FCAN_NUM                       2

/* can capacity */
#define FCAN_FD_CAPACITY               BIT(0) /* Whether canfd is supported */
#define FCAN_CAPACITY                  FCAN_FD_CAPACITY

/* TACH */
#define FTACHO0_ID                     0
#define FTACHO1_ID                     1
#define FTACHO2_ID                     2
#define FTACHO3_ID                     3
#define FTACHO_NUM                     4

/* MIO */
#define FMIO0_ID                       0
#define FMIO1_ID                       1
#define FMIO2_ID                       2
#define FMIO3_ID                       3
#define FMIO4_ID                       4
#define FMIO5_ID                       5
#define FMIO6_ID                       6
#define FMIO7_ID                       7
#define FMIO8_ID                       8
#define FMIO9_ID                       9
#define FMIO10_ID                      10
#define FMIO11_ID                      11
#define FMIO12_ID                      12
#define FMIO13_ID                      13
#define FMIO14_ID                      14
#define FMIO15_ID                      15
#define FMIO_NUM                       16

/* SDIF */
#define FSDIF0_ID                      0
#define FSDIF1_ID                      1
#define FSDIF_NUM                      2

#define FSDIF0_BASE_ADDR               0x28000000U
#define FSDIF1_BASE_ADDR               0x28001000U

#define FSDIF0_IRQ_NUM                 104U
#define FSDIF1_IRQ_NUM                 105U

#define FSDIF_CLK_FREQ_HZ              (1200000000UL) /* 1.2GHz */

/*****************************************************************************/
/* register offset of iopad function / pull / driver strength */
#define FIOPAD_AN59_REG0_OFFSET        0x0000U
#define FIOPAD_AW47_REG0_OFFSET        0x0004U
#define FIOPAD_AR55_REG0_OFFSET        0x0020U
#define FIOPAD_AJ55_REG0_OFFSET        0x0024U
#define FIOPAD_AL55_REG0_OFFSET        0x0028U
#define FIOPAD_AL53_REG0_OFFSET        0x002CU
#define FIOPAD_AN51_REG0_OFFSET        0x0030U
#define FIOPAD_AR51_REG0_OFFSET        0x0034U
#define FIOPAD_BA57_REG0_OFFSET        0x0038U
#define FIOPAD_BA59_REG0_OFFSET        0x003CU
#define FIOPAD_AW57_REG0_OFFSET        0x0040U
#define FIOPAD_AW59_REG0_OFFSET        0x0044U
#define FIOPAD_AU55_REG0_OFFSET        0x0048U
#define FIOPAD_AN57_REG0_OFFSET        0x004CU
#define FIOPAD_AL59_REG0_OFFSET        0x0050U
#define FIOPAD_AJ59_REG0_OFFSET        0x0054U
#define FIOPAD_AJ57_REG0_OFFSET        0x0058U
#define FIOPAD_AG59_REG0_OFFSET        0x005CU
#define FIOPAD_AG57_REG0_OFFSET        0x0060U
#define FIOPAD_AE59_REG0_OFFSET        0x0064U
#define FIOPAD_AC59_REG0_OFFSET        0x0068U
#define FIOPAD_AC57_REG0_OFFSET        0x006CU
#define FIOPAD_AR49_REG0_OFFSET        0x0070U
#define FIOPAD_BA55_REG0_OFFSET        0x0074U
#define FIOPAD_BA53_REG0_OFFSET        0x0078U
#define FIOPAD_AR59_REG0_OFFSET        0x007CU
#define FIOPAD_AU59_REG0_OFFSET        0x0080U
#define FIOPAD_AR57_REG0_OFFSET        0x0084U
#define FIOPAD_BA49_REG0_OFFSET        0x0088U
#define FIOPAD_AW55_REG0_OFFSET        0x008CU
#define FIOPAD_A35_REG0_OFFSET         0x0090U
#define FIOPAD_R57_REG0_OFFSET         0x0094U
#define FIOPAD_R59_REG0_OFFSET         0x0098U
#define FIOPAD_U59_REG0_OFFSET         0x009CU
#define FIOPAD_W59_REG0_OFFSET         0x00A0U
#define FIOPAD_U57_REG0_OFFSET         0x00A4U
#define FIOPAD_AA57_REG0_OFFSET        0x00A8U
#define FIOPAD_AA59_REG0_OFFSET        0x00ACU
#define FIOPAD_AW51_REG0_OFFSET        0x00B0U
#define FIOPAD_AU51_REG0_OFFSET        0x00B4U
#define FIOPAD_A39_REG0_OFFSET         0x00B8U
#define FIOPAD_C39_REG0_OFFSET         0x00BCU
#define FIOPAD_C37_REG0_OFFSET         0x00C0U
#define FIOPAD_A37_REG0_OFFSET         0x00C4U
#define FIOPAD_A41_REG0_OFFSET         0x00C8U
#define FIOPAD_A43_REG0_OFFSET         0x00CCU
#define FIOPAD_A45_REG0_OFFSET         0x00D0U
#define FIOPAD_C45_REG0_OFFSET         0x00D4U
#define FIOPAD_A47_REG0_OFFSET         0x00D8U
#define FIOPAD_A49_REG0_OFFSET         0x00DCU
#define FIOPAD_C49_REG0_OFFSET         0x00E0U
#define FIOPAD_A51_REG0_OFFSET         0x00E4U
#define FIOPAD_A33_REG0_OFFSET         0x00E8U
#define FIOPAD_C33_REG0_OFFSET         0x00ECU
#define FIOPAD_C31_REG0_OFFSET         0x00F0U
#define FIOPAD_A31_REG0_OFFSET         0x00F4U
#define FIOPAD_AJ53_REG0_OFFSET        0x00F8U
#define FIOPAD_AL49_REG0_OFFSET        0x00FCU
#define FIOPAD_AL47_REG0_OFFSET        0x0100U
#define FIOPAD_AN49_REG0_OFFSET        0x0104U
#define FIOPAD_AG51_REG0_OFFSET        0x0108U
#define FIOPAD_AJ51_REG0_OFFSET        0x010CU
#define FIOPAD_AG49_REG0_OFFSET        0x0110U
#define FIOPAD_AE55_REG0_OFFSET        0x0114U
#define FIOPAD_AE53_REG0_OFFSET        0x0118U
#define FIOPAD_AG55_REG0_OFFSET        0x011CU
#define FIOPAD_AJ49_REG0_OFFSET        0x0120U
#define FIOPAD_AC55_REG0_OFFSET        0x0124U
#define FIOPAD_AC53_REG0_OFFSET        0x0128U
#define FIOPAD_AE51_REG0_OFFSET        0x012CU
#define FIOPAD_W51_REG0_OFFSET         0x0130U
#define FIOPAD_W55_REG0_OFFSET         0x0134U
#define FIOPAD_W53_REG0_OFFSET         0x0138U
#define FIOPAD_U55_REG0_OFFSET         0x013CU
#define FIOPAD_U53_REG0_OFFSET         0x0140U
#define FIOPAD_AE49_REG0_OFFSET        0x0144U
#define FIOPAD_AC49_REG0_OFFSET        0x0148U
#define FIOPAD_AE47_REG0_OFFSET        0x014CU
#define FIOPAD_AA47_REG0_OFFSET        0x0150U
#define FIOPAD_AA49_REG0_OFFSET        0x0154U
#define FIOPAD_W49_REG0_OFFSET         0x0158U
#define FIOPAD_AA51_REG0_OFFSET        0x015CU
#define FIOPAD_U49_REG0_OFFSET         0x0160U
#define FIOPAD_G59_REG0_OFFSET         0x0164U
#define FIOPAD_J59_REG0_OFFSET         0x0168U
#define FIOPAD_L57_REG0_OFFSET         0x016CU
#define FIOPAD_C59_REG0_OFFSET         0x0170U
#define FIOPAD_E59_REG0_OFFSET         0x0174U
#define FIOPAD_J57_REG0_OFFSET         0x0178U
#define FIOPAD_L59_REG0_OFFSET         0x017CU
#define FIOPAD_N59_REG0_OFFSET         0x0180U
#define FIOPAD_C57_REG0_OFFSET         0x0184U
#define FIOPAD_E57_REG0_OFFSET         0x0188U
#define FIOPAD_E31_REG0_OFFSET         0x018CU
#define FIOPAD_G31_REG0_OFFSET         0x0190U
#define FIOPAD_N41_REG0_OFFSET         0x0194U
#define FIOPAD_N39_REG0_OFFSET         0x0198U
#define FIOPAD_J33_REG0_OFFSET         0x019CU
#define FIOPAD_N33_REG0_OFFSET         0x01A0U
#define FIOPAD_L33_REG0_OFFSET         0x01A4U
#define FIOPAD_N45_REG0_OFFSET         0x01A8U
#define FIOPAD_N43_REG0_OFFSET         0x01ACU
#define FIOPAD_L31_REG0_OFFSET         0x01B0U
#define FIOPAD_J31_REG0_OFFSET         0x01B4U
#define FIOPAD_J29_REG0_OFFSET         0x01B8U
#define FIOPAD_E29_REG0_OFFSET         0x01BCU
#define FIOPAD_G29_REG0_OFFSET         0x01C0U
#define FIOPAD_N27_REG0_OFFSET         0x01C4U
#define FIOPAD_L29_REG0_OFFSET         0x01C8U
#define FIOPAD_J37_REG0_OFFSET         0x01CCU
#define FIOPAD_J39_REG0_OFFSET         0x01D0U
#define FIOPAD_G41_REG0_OFFSET         0x01D4U
#define FIOPAD_E43_REG0_OFFSET         0x01D8U
#define FIOPAD_L43_REG0_OFFSET         0x01DCU
#define FIOPAD_C43_REG0_OFFSET         0x01E0U
#define FIOPAD_E41_REG0_OFFSET         0x01E4U
#define FIOPAD_L45_REG0_OFFSET         0x01E8U
#define FIOPAD_J43_REG0_OFFSET         0x01ECU
#define FIOPAD_J41_REG0_OFFSET         0x01F0U
#define FIOPAD_L39_REG0_OFFSET         0x01F4U
#define FIOPAD_E37_REG0_OFFSET         0x01F8U
#define FIOPAD_E35_REG0_OFFSET         0x01FCU
#define FIOPAD_G35_REG0_OFFSET         0x0200U
#define FIOPAD_J35_REG0_OFFSET         0x0204U
#define FIOPAD_L37_REG0_OFFSET         0x0208U
#define FIOPAD_N35_REG0_OFFSET         0x020CU
#define FIOPAD_R51_REG0_OFFSET         0x0210U
#define FIOPAD_R49_REG0_OFFSET         0x0214U
#define FIOPAD_N51_REG0_OFFSET         0x0218U
#define FIOPAD_N55_REG0_OFFSET         0x021CU
#define FIOPAD_L55_REG0_OFFSET         0x0220U
#define FIOPAD_J55_REG0_OFFSET         0x0224U
#define FIOPAD_J45_REG0_OFFSET         0x0228U
#define FIOPAD_E47_REG0_OFFSET         0x022CU
#define FIOPAD_G47_REG0_OFFSET         0x0230U
#define FIOPAD_J47_REG0_OFFSET         0x0234U
#define FIOPAD_J49_REG0_OFFSET         0x0238U
#define FIOPAD_N49_REG0_OFFSET         0x023CU
#define FIOPAD_L51_REG0_OFFSET         0x0240U
#define FIOPAD_L49_REG0_OFFSET         0x0244U
#define FIOPAD_N53_REG0_OFFSET         0x0248U
#define FIOPAD_J53_REG0_OFFSET         0x024CU

#define FIOPAD_REG0_BEG_OFFSET         FIOPAD_AN59_REG0_OFFSET
#define FIOPAD_REG0_END_OFFSET         FIOPAD_J53_REG0_OFFSET

/* register offset of iopad delay */
#define FIOPAD_AJ55_REG1_OFFSET        0x1024U
#define FIOPAD_AL55_REG1_OFFSET        0x1028U
#define FIOPAD_AL53_REG1_OFFSET        0x102CU
#define FIOPAD_AN51_REG1_OFFSET        0x1030U
#define FIOPAD_AR51_REG1_OFFSET        0x1034U
#define FIOPAD_AJ57_REG1_OFFSET        0x1058U
#define FIOPAD_AG59_REG1_OFFSET        0x105CU
#define FIOPAD_AG57_REG1_OFFSET        0x1060U
#define FIOPAD_AE59_REG1_OFFSET        0x1064U
#define FIOPAD_BA55_REG1_OFFSET        0x1074U
#define FIOPAD_BA53_REG1_OFFSET        0x1078U
#define FIOPAD_AR59_REG1_OFFSET        0x107CU
#define FIOPAD_AU59_REG1_OFFSET        0x1080U
#define FIOPAD_A45_REG1_OFFSET         0x10D0U
#define FIOPAD_C45_REG1_OFFSET         0x10D4U
#define FIOPAD_A47_REG1_OFFSET         0x10D8U
#define FIOPAD_A49_REG1_OFFSET         0x10DCU
#define FIOPAD_C49_REG1_OFFSET         0x10E0U
#define FIOPAD_A51_REG1_OFFSET         0x10E4U
#define FIOPAD_A33_REG1_OFFSET         0x10E8U
#define FIOPAD_C33_REG1_OFFSET         0x10ECU
#define FIOPAD_C31_REG1_OFFSET         0x10F0U
#define FIOPAD_A31_REG1_OFFSET         0x10F4U
#define FIOPAD_AJ53_REG1_OFFSET        0x10F8U
#define FIOPAD_AL49_REG1_OFFSET        0x10FCU
#define FIOPAD_AL47_REG1_OFFSET        0x1100U
#define FIOPAD_AN49_REG1_OFFSET        0x1104U
#define FIOPAD_AG51_REG1_OFFSET        0x1108U
#define FIOPAD_AJ51_REG1_OFFSET        0x110CU
#define FIOPAD_AG49_REG1_OFFSET        0x1110U
#define FIOPAD_AE55_REG1_OFFSET        0x1114U
#define FIOPAD_AE53_REG1_OFFSET        0x1118U
#define FIOPAD_AG55_REG1_OFFSET        0x111CU
#define FIOPAD_AJ49_REG1_OFFSET        0x1120U
#define FIOPAD_AC55_REG1_OFFSET        0x1124U
#define FIOPAD_AC53_REG1_OFFSET        0x1128U
#define FIOPAD_AE51_REG1_OFFSET        0x112CU
#define FIOPAD_W51_REG1_OFFSET         0x1130U
#define FIOPAD_W53_REG1_OFFSET         0x1138U
#define FIOPAD_U55_REG1_OFFSET         0x113CU
#define FIOPAD_U53_REG1_OFFSET         0x1140U
#define FIOPAD_AE49_REG1_OFFSET        0x1144U
#define FIOPAD_AC49_REG1_OFFSET        0x1148U
#define FIOPAD_AE47_REG1_OFFSET        0x114CU
#define FIOPAD_AA47_REG1_OFFSET        0x1150U
#define FIOPAD_AA49_REG1_OFFSET        0x1154U
#define FIOPAD_W49_REG1_OFFSET         0x1158U
#define FIOPAD_AA51_REG1_OFFSET        0x115CU
#define FIOPAD_U49_REG1_OFFSET         0x1160U
#define FIOPAD_J59_REG1_OFFSET         0x1168U
#define FIOPAD_L57_REG1_OFFSET         0x116CU
#define FIOPAD_C59_REG1_OFFSET         0x1170U
#define FIOPAD_E59_REG1_OFFSET         0x1174U
#define FIOPAD_J57_REG1_OFFSET         0x1178U
#define FIOPAD_L59_REG1_OFFSET         0x117CU
#define FIOPAD_N59_REG1_OFFSET         0x1180U
#define FIOPAD_E31_REG1_OFFSET         0x118CU
#define FIOPAD_G31_REG1_OFFSET         0x1190U
#define FIOPAD_N41_REG1_OFFSET         0x1194U
#define FIOPAD_N39_REG1_OFFSET         0x1198U
#define FIOPAD_J33_REG1_OFFSET         0x119CU
#define FIOPAD_N33_REG1_OFFSET         0x11A0U
#define FIOPAD_L33_REG1_OFFSET         0x11A4U
#define FIOPAD_N45_REG1_OFFSET         0x11A8U
#define FIOPAD_N43_REG1_OFFSET         0x11ACU
#define FIOPAD_L31_REG1_OFFSET         0x11B0U
#define FIOPAD_J31_REG1_OFFSET         0x11B4U
#define FIOPAD_J29_REG1_OFFSET         0x11B8U
#define FIOPAD_E29_REG1_OFFSET         0x11BCU
#define FIOPAD_G29_REG1_OFFSET         0x11C0U
#define FIOPAD_J37_REG1_OFFSET         0x11CCU
#define FIOPAD_J39_REG1_OFFSET         0x11D0U
#define FIOPAD_G41_REG1_OFFSET         0x11D4U
#define FIOPAD_E43_REG1_OFFSET         0x11D8U
#define FIOPAD_L43_REG1_OFFSET         0x11DCU
#define FIOPAD_C43_REG1_OFFSET         0x11E0U
#define FIOPAD_E41_REG1_OFFSET         0x11E4U
#define FIOPAD_L45_REG1_OFFSET         0x11E8U
#define FIOPAD_J43_REG1_OFFSET         0x11ECU
#define FIOPAD_J41_REG1_OFFSET         0x11F0U
#define FIOPAD_L39_REG1_OFFSET         0x11F4U
#define FIOPAD_E37_REG1_OFFSET         0x11F8U
#define FIOPAD_E35_REG1_OFFSET         0x11FCU
#define FIOPAD_G35_REG1_OFFSET         0x1200U
#define FIOPAD_L55_REG1_OFFSET         0x1220U
#define FIOPAD_J55_REG1_OFFSET         0x1224U
#define FIOPAD_J45_REG1_OFFSET         0x1228U
#define FIOPAD_E47_REG1_OFFSET         0x122CU
#define FIOPAD_G47_REG1_OFFSET         0x1230U
#define FIOPAD_J47_REG1_OFFSET         0x1234U
#define FIOPAD_J49_REG1_OFFSET         0x1238U
#define FIOPAD_N49_REG1_OFFSET         0x123CU
#define FIOPAD_L51_REG1_OFFSET         0x1240U
#define FIOPAD_L49_REG1_OFFSET         0x1244U
#define FIOPAD_N53_REG1_OFFSET         0x1248U
#define FIOPAD_J53_REG1_OFFSET         0x124CU

#define FIOPAD_REG1_BEG_OFFSET         FIOPAD_AJ55_REG1_OFFSET
#define FIOPAD_REG1_END_OFFSET         FIOPAD_J53_REG1_OFFSET

#ifdef __cplusplus
}

#endif

#endif