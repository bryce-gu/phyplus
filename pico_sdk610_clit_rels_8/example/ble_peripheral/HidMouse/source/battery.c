/*****************************************************************************
Copyright:
File name:
Description:
Author:
Version:
Date:
History:
*****************************************************************************/
#include "rom_sym_def.h"
#include "battery.h"
#include "adc.h"
#include "pcba_cfgs.h"
#include "log.h"

#define BATTERY_ADC_VAL_MIN 2730
#define BATTERY_ADC_VAL_MAX 4095

uint8_t battery_get(void)
{
    return 100;
	uint32_t adc_val;
    uint8_t percent = 0;

	adc_val = hal_adc_sample(PCAB_CFGS_CFGS_BATT_PINN, ADC_RESOL_DIV3, 10);

    if (adc_val < BATTERY_ADC_VAL_MIN)
    {
        percent = 0;
    }
    else
    if (adc_val >= BATTERY_ADC_VAL_MAX)
    {
        percent = 100;
    }
    else
    {
        percent = 100 * (adc_val - BATTERY_ADC_VAL_MIN) / (BATTERY_ADC_VAL_MAX - BATTERY_ADC_VAL_MIN);
    }

    // LOG_ERROR("adcVal %d\n", adc_val);

    return percent;
}
