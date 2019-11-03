#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/pcnt.h"

#define PCNT_TEST_UNIT      PCNT_UNIT_0
#define PCNT_H_LIM_VAL      32767
#define PCNT_L_LIM_VAL      0
#define PCNT_INPUT_SIG_IO   34 // A2
#define PCNT_INPUT_CTRL_IO  5
#define PCNT_THRESH1_VAL    2000
#define PCNT_THRESH0_VAL    1000

static xQueueHandle pcnt_event_queue;

typedef struct {
    int unit;  // the PCNT unit that originated an interrupt
    uint32_t status; // information on the event type that caused the interrupt
} pcnt_evt_t;

static void IRAM_ATTR pcnt_isr(void *arg)
{
    uint32_t intr_status = PCNT.int_st.val;
    int i;
    pcnt_evt_t evt;
    portBASE_TYPE HPTaskAwoken = pdFALSE;

    for (i = 0; i < PCNT_UNIT_MAX; i++) {
        if (intr_status & (BIT(i))) {
            evt.unit = i;
            evt.status = PCNT.status_unit[i].val;
            PCNT.int_clr.val = BIT(i);
            xQueueSendFromISR(pcnt_event_queue, &evt, &HPTaskAwoken);
            if (HPTaskAwoken == pdTRUE) {
                portYIELD_FROM_ISR();
            }
        }
    }
}

void pulsecounter_init()
{
    pcnt_config_t pc_conf = {
        .pulse_gpio_num = PCNT_INPUT_SIG_IO,
        .ctrl_gpio_num = PCNT_INPUT_CTRL_IO,
        .channel = PCNT_CHANNEL_0,
        .unit = PCNT_TEST_UNIT,
        // What to do on the positive / negative edge of pulse input?
        .pos_mode = PCNT_COUNT_INC,   // Count up on the positive edge
        .neg_mode = PCNT_COUNT_DIS,   // Keep the counter value on the negative edge
        // What to do when control input is low or high?
        .lctrl_mode = PCNT_MODE_REVERSE, // Reverse counting direction if low
        .hctrl_mode = PCNT_MODE_KEEP,    // Keep the primary counter mode if high
        // Set the maximum and minimum limit values to watch
        .counter_h_lim = PCNT_H_LIM_VAL,
        .counter_l_lim = PCNT_L_LIM_VAL,
    };

    pcnt_unit_config(&pc_conf);
    pcnt_set_filter_value(PCNT_TEST_UNIT, 100);
    pcnt_filter_enable(PCNT_TEST_UNIT);

    pcnt_set_event_value(PCNT_TEST_UNIT,PCNT_EVT_THRES_1, PCNT_THRESH1_VAL);
    pcnt_event_enable(PCNT_TEST_UNIT, PCNT_EVT_THRES_1);

    pcnt_set_event_value(PCNT_TEST_UNIT,PCNT_EVT_THRES_0, PCNT_THRESH0_VAL);
    pcnt_event_enable(PCNT_TEST_UNIT, PCNT_EVT_THRES_0);

    pcnt_event_enable(PCNT_TEST_UNIT, PCNT_EVT_ZERO);
    pcnt_event_enable(PCNT_TEST_UNIT, PCNT_EVT_H_LIM);
    pcnt_event_enable(PCNT_TEST_UNIT, PCNT_EVT_L_LIM);

    pcnt_counter_pause(PCNT_TEST_UNIT);
    pcnt_counter_clear(PCNT_TEST_UNIT);

    pcnt_isr_register(pcnt_isr, NULL, 0, NULL);
    pcnt_intr_enable(PCNT_TEST_UNIT);

    pcnt_counter_resume(PCNT_TEST_UNIT);

    pcnt_event_queue = xQueueCreate(10, sizeof(pcnt_evt_t));
}

static int16_t pulse_count = 0;

static void pulsecounter_task()
{
    pcnt_evt_t evt;
    while (1)
    {
        xQueueReceive(pcnt_event_queue, &evt, 1000/portTICK_PERIOD_MS);
        pcnt_get_counter_value(PCNT_TEST_UNIT, &pulse_count);
    }
}

static TaskHandle_t counter_task = NULL;

void pulsecounter_start()
{
    if (counter_task == NULL)
    {
        xTaskCreate(pulsecounter_task, "counter_task", 4096, NULL, configMAX_PRIORITIES-1, &counter_task);
    }
}

void pulsecounter_reset()
{
    pulse_count = 0;
}

void pulsecounter_stop()
{
    if (counter_task != NULL)
    {
        vTaskDelete(counter_task);
        counter_task = NULL;
    }
}

int16_t pulsecounter_get_count()
{
    return pulse_count;
}

