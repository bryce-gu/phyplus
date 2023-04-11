/*
 ******************************************************************************
 * Name:        user_dhub.h
 *
 * description: defines:
 * 				implementation of CATE device plug & unplug detection using pulling.
 *
 *  History:     Date;           Author;         Description
 *              18 Mar. 2016;   Chen, George;   file creation.
 ******************************************************************************
 */

#ifndef __PCBA_CFGS_H__
#define __PCBA_CFGS_H__

/*
 ******************************************************************************
 * Includes
 ******************************************************************************
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
 ******************************************************************************
 * defines
 ******************************************************************************
 */
/* CHIP */
// #define PCAB_CFGS_CFGS_CHIP_6230SC_SOP16_00C
// #define PCAB_CFGS_CFGS_CHIP_6230SC_SOP16_01C
// #define PCAB_CFGS_CFGS_CHIP_6230SC_SOP16_YJX
// #define PCAB_CFGS_CFGS_CHIP_6230SD_SOP24_XXX
#define PCAB_CFGS_CFGS_CHIP_6230SC_SOP16_03C

/* PCBA */
// #define PCAB_CFGS_CFGS_PCBA_6230SC_MOUSE_A
// #define PCAB_CFGS_CFGS_PCBA_6230SD_MOUSE_V1_1
// #define PCAB_CFGS_CFGS_PCBA_6230SD_MOUSE_V1_2
// #define PCAB_CFGS_CFGS_PCBA_6230SD_MOUSE_V1_3
// #define PCAB_CFGS_CFGS_PCBA_6230SC_MOUSE_YJX
// #define PCAB_CFGS_CFGS_PCBA_6230_MOUSE_BYKC_V1_1
#define PCAB_CFGS_CFGS_PCBA_6230SC_MOUSE_WK303

#if     defined(PCAB_CFGS_CFGS_CHIP_6230SC_SOP16_00C)
#define PCAB_CFGS_CFGS_SCLK_PINN    (GPIO_DUMMY)
#define PCAB_CFGS_CFGS_CHPS_PINN    (GPIO_DUMMY)
#define PCAB_CFGS_CFGS_MOSI_PINN    (GPIO_DUMMY)
#define PCAB_CFGS_CFGS_MISO_PINN    (GPIO_DUMMY)

#define PCAB_CFGS_CFGS_SCLK_FMUX    (FMUX_GPIO)
#define PCAB_CFGS_CFGS_CHPS_FMUX    (FMUX_GPIO)
#define PCAB_CFGS_CFGS_MOSI_FMUX    (FMUX_GPIO)
#define PCAB_CFGS_CFGS_MISO_FMUX    (FMUX_GPIO)

#elif   defined(PCAB_CFGS_CFGS_CHIP_6230SC_SOP16_01C)
#define PCAB_CFGS_CFGS_SCLK_PINN    (GPIO_P14)
#define PCAB_CFGS_CFGS_CHPS_PINN    (GPIO_DUMMY)
#define PCAB_CFGS_CFGS_MOSI_PINN    (GPIO_P15)
#define PCAB_CFGS_CFGS_MISO_PINN    (GPIO_P02)

#define PCAB_CFGS_CFGS_SCLK_FMUX    (FMUX_P14_SPI_CLK)
#define PCAB_CFGS_CFGS_CHPS_FMUX    (FMUX_GPIO)
#define PCAB_CFGS_CFGS_MOSI_FMUX    (FMUX_P15_SPI_TX)
#define PCAB_CFGS_CFGS_MISO_FMUX    (FMUX_P2_SPI_RX)

#elif   defined(PCAB_CFGS_CFGS_CHIP_6230SC_SOP16_YJX)
#define PCAB_CFGS_CFGS_SCLK_PINN    (GPIO_P14)
#define PCAB_CFGS_CFGS_CHPS_PINN    (GPIO_DUMMY)
#define PCAB_CFGS_CFGS_MOSI_PINN    (GPIO_P15)
#define PCAB_CFGS_CFGS_MISO_PINN    (GPIO_P02)

#define PCAB_CFGS_CFGS_SCLK_FMUX    (FMUX_P14_SPI_CLK)
#define PCAB_CFGS_CFGS_CHPS_FMUX    (FMUX_GPIO)
#define PCAB_CFGS_CFGS_MOSI_FMUX    (FMUX_P15_SPI_TX)
#define PCAB_CFGS_CFGS_MISO_FMUX    (FMUX_P2_SPI_RX)

#elif   defined(PCAB_CFGS_CFGS_CHIP_6230SC_SOP16_03C)
#define PCAB_CFGS_CFGS_SCLK_PINN    (GPIO_P16)
#define PCAB_CFGS_CFGS_CHPS_PINN    (GPIO_DUMMY)
#define PCAB_CFGS_CFGS_MOSI_PINN    (GPIO_P15)
#define PCAB_CFGS_CFGS_MISO_PINN    (GPIO_P02)

#define PCAB_CFGS_CFGS_SCLK_FMUX    (FMUX_P16_SPI_CLK)
#define PCAB_CFGS_CFGS_CHPS_FMUX    (FMUX_GPIO)
#define PCAB_CFGS_CFGS_MOSI_FMUX    (FMUX_P15_SPI_TX)
#define PCAB_CFGS_CFGS_MISO_FMUX    (FMUX_P2_SPI_RX)

#elif   defined(PCAB_CFGS_CFGS_CHIP_6230SD_SOP24_XXX)
#define PCAB_CFGS_CFGS_SCLK_PINN    (GPIO_P16)
#define PCAB_CFGS_CFGS_CHPS_PINN    (GPIO_DUMMY)
#define PCAB_CFGS_CFGS_MOSI_PINN    (GPIO_P17)
#define PCAB_CFGS_CFGS_MISO_PINN    (GPIO_P18)

#define PCAB_CFGS_CFGS_SCLK_FMUX    (FMUX_P16_SPI_CLK)
#define PCAB_CFGS_CFGS_CHPS_FMUX    (FMUX_GPIO)
#define PCAB_CFGS_CFGS_MOSI_FMUX    (FMUX_P17_SPI_TX)
#define PCAB_CFGS_CFGS_MISO_FMUX    (FMUX_P18_SPI_RX)

#else
#error "!! NONE CHIP !!"
#endif

#if     defined(PCAB_CFGS_CFGS_PCBA_6230SC_MOUSE_A)
#define PCAB_CFGS_CFGS_MODE_PINN        (GPIO_P01)
// #define PCAB_CFGS_CFGS_MOTI_PINN        (GPIO_P13)
#define PCAB_CFGS_CFGS_KEYL_PINN        (GPIO_P00)// key left
#define PCAB_CFGS_CFGS_KEYR_PINN        (GPIO_P17)// key right
#define PCAB_CFGS_CFGS_KEYM_PINN        (GPIO_P03)// key middle
#define PCAB_CFGS_CFGS_WHLA_PINN        (GPIO_P05)// wheel ZA
#define PCAB_CFGS_CFGS_WHLB_PINN        (GPIO_P04)// wheel ZB
#define PCAB_CFGS_CFGS_DPI_PINN         (GPIO_P13)
#define PCAB_CFGS_CFGS_4KEY_PINN        PCAB_CFGS_CFGS_KEYM_PINN
#define PCAB_CFGS_CFGS_5KEY_PINN        PCAB_CFGS_CFGS_KEYR_PINN

#define PCAB_CFGS_CFGS_BATT_PINN        (ADC_CH7P_P13)

#define PCAB_CFGS_CFGS_FLIP_X           (0)
#define PCAB_CFGS_CFGS_FLIP_Y           (1)
#define PCAB_CFGS_CFGS_SWAP_XY          (0)

#elif     defined(PCAB_CFGS_CFGS_PCBA_6230SC_MOUSE_YJX)
#define PCAB_CFGS_CFGS_MODE_PINN        (GPIO_P06)
/* SHOULD NOT "#define" FOR CLIT: YJX, MOTION PINN N/A */
// #define PCAB_CFGS_CFGS_MOTI_PINN        (/* GPIO_P13 */GPIO_DUMMY)
#define PCAB_CFGS_CFGS_KEYL_PINN        (GPIO_P01)// key left
#define PCAB_CFGS_CFGS_KEYR_PINN        (GPIO_P17)// key right
#define PCAB_CFGS_CFGS_KEYM_PINN        (GPIO_P13)// key middle
#define PCAB_CFGS_CFGS_WHLA_PINN        (/* GPIO_DUMMY */GPIO_P05)// wheel ZA
#define PCAB_CFGS_CFGS_WHLB_PINN        (/* GPIO_DUMMY */GPIO_P04)// wheel ZB
#define PCAB_CFGS_CFGS_DPI_PINN         PCAB_CFGS_CFGS_KEYL_PINN
// #define PCAB_CFGS_CFGS_4KEY_PINN        PCAB_CFGS_CFGS_KEYR_PINN
// #define PCAB_CFGS_CFGS_5KEY_PINN        PCAB_CFGS_CFGS_KEYM_PINN
#define PCAB_CFGS_CFGS_MULTI_KEY_PINN   (GPIO_P00)


#define PCAB_CFGS_CFGS_BATT_PINN        (ADC_CH7P_P13)

#define PCAB_CFGS_CFGS_FLIP_X           (1) // cfgs for YongJiaXin
#define PCAB_CFGS_CFGS_FLIP_Y           (1)
#define PCAB_CFGS_CFGS_SWAP_XY          (1)

// #define PCAB_CFGS_CFGS_MODE_PINN        (GPIO_P06)
// #define PCAB_CFGS_CFGS_MOTI_PINN        (/* GPIO_P13 */GPIO_DUMMY)
// #define PCAB_CFGS_CFGS_KEYL_PINN        (GPIO_P12)// key left
// #define PCAB_CFGS_CFGS_KEYR_PINN        (GPIO_P14)// key right
// #define PCAB_CFGS_CFGS_KEYM_PINN        (GPIO_P13)// key middle
// #define PCAB_CFGS_CFGS_WHLA_PINN        (/* GPIO_DUMMY */GPIO_P10)// wheel ZA
// #define PCAB_CFGS_CFGS_WHLB_PINN        (/* GPIO_DUMMY */GPIO_P11)// wheel ZB
// #define PCAB_CFGS_CFGS_DPI_PINN         PCAB_CFGS_CFGS_KEYL_PINN
// #define PCAB_CFGS_CFGS_4KEY_PINN        PCAB_CFGS_CFGS_KEYR_PINN
// #define PCAB_CFGS_CFGS_5KEY_PINN        PCAB_CFGS_CFGS_KEYM_PINN
// #define PCAB_CFGS_CFGS_MULTI_KEY_PINN   GPIO_P05

// #define PCAB_CFGS_CFGS_BATT_PINN        (ADC_CH7P_P13)

// #define PCAB_CFGS_CFGS_FLIP_X           (1) // cfgs for YongJiaXin
// #define PCAB_CFGS_CFGS_FLIP_Y           (1)
// #define PCAB_CFGS_CFGS_SWAP_XY          (1)
#elif   defined(PCAB_CFGS_CFGS_PCBA_6230SD_MOUSE_V1_1)
#define PCAB_CFGS_CFGS_MODE_PINN        (GPIO_P06)
// #define PCAB_CFGS_CFGS_MOTI_PINN        (GPIO_P03)
#define PCAB_CFGS_CFGS_KEYL_PINN        (GPIO_P12) // cfgs for xinziyuan //(GPIO_P12)  // key left   // cfgs for YongJiaXin
#define PCAB_CFGS_CFGS_KEYR_PINN        (GPIO_P02)                       //(GPIO_P14)  // key right
#define PCAB_CFGS_CFGS_KEYM_PINN        (GPIO_P14)                       //(GPIO_P13)  // key middle
#define PCAB_CFGS_CFGS_WHLA_PINN        (GPIO_P10) //(GPIO_P10)  // wheel ZA
#define PCAB_CFGS_CFGS_WHLB_PINN        (GPIO_P09) //(GPIO_P11)  // wheel ZB
#define PCAB_CFGS_CFGS_DPI_PINN         (GPIO_P05)
#define PCAB_CFGS_CFGS_4KEY_PINN        (GPIO_P00)
#define PCAB_CFGS_CFGS_5KEY_PINN        (GPIO_P02)

#define PCAB_CFGS_CFGS_BATT_PINN        (ADC_CH7P_P13)

#define PCAB_CFGS_CFGS_USES_GDKS        (0)

#define PCAB_CFGS_CFGS_FLIP_X           (0) // cfgs for xinziyuan
#define PCAB_CFGS_CFGS_FLIP_Y           (1)
#define PCAB_CFGS_CFGS_SWAP_XY          (0)

#elif   defined(PCAB_CFGS_CFGS_PCBA_6230SD_MOUSE_V1_2)
#define PCAB_CFGS_CFGS_MODE_PINN        (GPIO_P06)
// #define PCAB_CFGS_CFGS_MOTI_PINN        (GPIO_P03)
#define PCAB_CFGS_CFGS_KEYL_PINN        (GPIO_P12)
#define PCAB_CFGS_CFGS_KEYR_PINN        (GPIO_P03)
#define PCAB_CFGS_CFGS_KEYM_PINN        (GPIO_P08)
#define PCAB_CFGS_CFGS_WHLA_PINN        (GPIO_P10) //(GPIO_P10)  // wheel ZA
#define PCAB_CFGS_CFGS_WHLB_PINN        (GPIO_P09) //(GPIO_P11)  // wheel ZB
#define PCAB_CFGS_CFGS_DPI_PINN         (GPIO_P05)
#define PCAB_CFGS_CFGS_KEYA_PINN        (GPIO_P11)
// #define PCAB_CFGS_CFGS_5KEY_PINN        (GPIO_P02)

#define PCAB_CFGS_CFGS_BATT_PINN        (ADC_CH7P_P13)

#define PCAB_CFGS_CFGS_USES_GDKS        (0)

#define PCAB_CFGS_CFGS_FLIP_X           (0) // cfgs for xinziyuan
#define PCAB_CFGS_CFGS_FLIP_Y           (1)
#define PCAB_CFGS_CFGS_SWAP_XY          (0)

#elif   defined(PCAB_CFGS_CFGS_PCBA_6230SD_MOUSE_V1_3)
#define PCAB_CFGS_CFGS_MODE_PINN        (GPIO_P06)
// #define PCAB_CFGS_CFGS_MOTI_PINN        (GPIO_P03)
#define PCAB_CFGS_CFGS_DPI_PINN         (GPIO_P05)

#define PCAB_CFGS_CFGS_KEYL_PINN        (GPIO_P03) // key left
#define PCAB_CFGS_CFGS_KEYR_PINN        (GPIO_P08) // key rightGPIO_P08
#define PCAB_CFGS_CFGS_KEYM_PINN        (GPIO_P09) // key middle
#define PCAB_CFGS_CFGS_KEY0_PINN        (GPIO_P06)

#define PCAB_CFGS_CFGS_WHLA_PINN        (GPIO_P13) // wheel ZA
#define PCAB_CFGS_CFGS_WHLB_PINN        (GPIO_P02) // wheel ZB

#elif     defined(PCAB_CFGS_CFGS_PCBA_6230_MOUSE_BYKC_V1_1)
#define PCAB_CFGS_CFGS_MODE_PINN        (GPIO_P10)
// #define PCAB_CFGS_CFGS_MOTI_PINN        (GPIO_P06)
#define PCAB_CFGS_CFGS_KEYL_PINN        (GPIO_P14)// key left
#define PCAB_CFGS_CFGS_KEYR_PINN        (GPIO_P01)// key right
#define PCAB_CFGS_CFGS_KEYM_PINN        (GPIO_P02)// key middle
#define PCAB_CFGS_CFGS_WHLA_PINN        (/* GPIO_DUMMY */GPIO_P11)// wheel ZA
#define PCAB_CFGS_CFGS_WHLB_PINN        (/* GPIO_DUMMY */GPIO_P12)// wheel ZB
#define PCAB_CFGS_CFGS_DPI_PINN         (GPIO_P03)
// #define PCAB_CFGS_CFGS_4KEY_PINN        PCAB_CFGS_CFGS_KEYM_PINN
// #define PCAB_CFGS_CFGS_5KEY_PINN        PCAB_CFGS_CFGS_KEYR_PINN

#define PCAB_CFGS_CFGS_LED0_PINN        (GPIO_P00)
#define PCAB_CFGS_CFGS_LED1_PINN        (GPIO_P09)

#define PCAB_CFGS_CFGS_BATT_PINN        (ADC_CH7P_P13)

#define PCAB_CFGS_CFGS_FLIP_X           (0)
#define PCAB_CFGS_CFGS_FLIP_Y           (1)
#define PCAB_CFGS_CFGS_SWAP_XY          (0)

#elif     defined(PCAB_CFGS_CFGS_PCBA_6230SC_MOUSE_WK303)
#define PCAB_CFGS_CFGS_MODE_PINN        (GPIO_P06)
/* SHOULD NOT "#define" FOR CLIT: YJX, MOTION PINN N/A */
// #define PCAB_CFGS_CFGS_MOTI_PINN        (/* GPIO_P13 */GPIO_DUMMY)
#define PCAB_CFGS_CFGS_KEYL_PINN        (GPIO_P14)// key left
#define PCAB_CFGS_CFGS_KEYR_PINN        (GPIO_P18)// key right
#define PCAB_CFGS_CFGS_KEYM_PINN        (GPIO_P17)// key middle
#define PCAB_CFGS_CFGS_WHLA_PINN        (GPIO_P04)// wheel ZA
#define PCAB_CFGS_CFGS_WHLB_PINN        (GPIO_P01)// wheel ZB
// #define PCAB_CFGS_CFGS_DPI_PINN         PCAB_CFGS_CFGS_KEYL_PINN
// #define PCAB_CFGS_CFGS_4KEY_PINN        PCAB_CFGS_CFGS_KEYM_PINN
// #define PCAB_CFGS_CFGS_5KEY_PINN        PCAB_CFGS_CFGS_KEYR_PINN
#define PCAB_CFGS_CFGS_MULTI_KEY_PINN   (GPIO_P19)
#define PCAB_CFGS_CFGS_BOND_KEY1_PINN   (GPIO_P00)
#define PCAB_CFGS_CFGS_BOND_KEY2_PINN   (GPIO_P03)

#define PCAB_CFGS_CFGS_BATT_PINN        (ADC_CH7P_P13)

#define PCAB_CFGS_CFGS_FLIP_X           (1) // cfgs for YongJiaXin
#define PCAB_CFGS_CFGS_FLIP_Y           (1)
#define PCAB_CFGS_CFGS_SWAP_XY          (1)

#else

#error "!! NONE PCBA !!"

#endif

/*
 ******************************************************************************
 * public variables
 ******************************************************************************
 */

/*
 ******************************************************************************
 * public functions
 ******************************************************************************
 */

#ifdef __cplusplus
}
#endif

#endif /* __PCBA_CFGS_H__ */
