#include "esp32c3.h"
#include <esp_efuse_rtc_calib.h>
#include <esp_private/sar_periph_ctrl.h>
#include <hal/temperature_sensor_ll.h>

#define TAG __FILE_NAME__

void temperature_init(void)
{
    temperature_sensor_power_acquire();
    temp_sensor_sync_tsens_idx(2);
    temperature_sensor_ll_clk_sel(TEMPERATURE_SENSOR_CLK_SRC_DEFAULT);
    temperature_sensor_ll_set_clk_div(6);
    temperature_sensor_ll_set_range(15);
}

int temperature(void)
{
    static float deltaT = 0.0f;
    if (deltaT == 0.0f)
    {
        esp_efuse_rtc_calib_get_tsens_val(&deltaT);
    }
    return (int)(temp_sensor_get_raw_value(NULL) - deltaT / 10.0f);
}
