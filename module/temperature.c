#include "esp32c3.h"
#include <esp_efuse_rtc_calib.h>
#include <esp_private/sar_periph_ctrl.h>
#include <hal/temperature_sensor_hal.h>
#include <hal/temperature_sensor_ll.h>

#define TAG __FILE_NAME__

void temperature_init(void)
{
    temperature_sensor_power_acquire();
    temperature_sensor_hal_sync_tsens_idx(2);
    temperature_sensor_ll_clk_sel(TEMPERATURE_SENSOR_CLK_SRC_DEFAULT);
    temperature_sensor_ll_set_clk_div(6);
    temperature_sensor_ll_set_range(15);
}

int temperature(void)
{
    static float deltaT = 0.0f;
    if (deltaT == 0.0f)
    {
        deltaT = temperature_sensor_ll_load_calib_param();
    }
    return (int)(temperature_sensor_hal_get_degree(NULL) - deltaT / 10.0f);
}
