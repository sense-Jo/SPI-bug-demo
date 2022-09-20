#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>

#define LED_ON   GPIO_ACTIVE_LOW
#define LED_OFF  GPIO_ACTIVE_HIGH

static const struct gpio_dt_spec ledGreen = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

const struct device *lis2dw12 = NULL;
struct sensor_value accel[3];

void handleError(int r, char* msg);
int initPeripherals();
int fetchEnvironmentData();
int fetchMotionData();


void blockStop2()
{
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, 3);
}

void main(void)
{
    // k_msleep(1000);
    // blockStop2();  // locking STOP2 before it is ever accessed, removes the issue

    handleError(initPeripherals(), "init peripherals");

    while (1)
    {
        k_msleep(2000);

        gpio_pin_set_dt(&ledGreen, LED_ON);
        handleError(fetchMotionData(), "fetch motion data");
        float accelF[] = { sensor_value_to_double(&accel[0]),
                           sensor_value_to_double(&accel[1]),
                           sensor_value_to_double(&accel[2]) };
        printk("Measured: [%i, %i, %i]\n",
               (int) accelF[0] * 100,
               (int) accelF[1] * 100,
               (int) accelF[2] * 100);
        gpio_pin_set_dt(&ledGreen, LED_OFF);
    }
}


void handleError(int r, char* msg)
{
    if (r != 0)
    {
        printk("Error %d: %s\n", r, msg);
        while (1);
    }
    else
    {
        printk("Success: %s\n", msg);
    }
}

int initPeripherals()
{
    gpio_pin_configure_dt(&ledGreen, GPIO_OUTPUT_INACTIVE);
    gpio_pin_set_dt(&ledGreen, LED_ON);

#ifdef CONFIG_LIS2DW12
    lis2dw12 = DEVICE_DT_GET_ONE(st_lis2dw12);

    if (lis2dw12 == NULL) {
      printk("Could not get LIS2DW12 device\n");
      return -1;
    }

    /* set LIS2DW12 accel sampling frequency to 1.6 Hz */

    struct sensor_value odr_attr;
    odr_attr.val1 = 1;
    odr_attr.val2 = 600000;

    if (sensor_attr_set(lis2dw12, SENSOR_CHAN_ACCEL_XYZ,
                        SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr) < 0)
    {
        printk("Cannot set sampling frequency for LIS2DW12 accel\n");
        return -2;
    }
#endif

    gpio_pin_set_dt(&ledGreen, LED_OFF);
    return 0;
}

int fetchMotionData()
{
    if (lis2dw12 != NULL)
    {
        int r = sensor_sample_fetch(lis2dw12);
        if (r == 0)
        {
            r = sensor_channel_get(lis2dw12, SENSOR_CHAN_ACCEL_XYZ, accel);
        }
        if (r != 0) {
            printk("LIS2DW12 failed: %d\n", r);
            return -1;
        }
    }
    else
    {
        printk("Skipping LIS2DW12\n");
    }

    return 0;
}
