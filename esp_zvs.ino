#include "driver/mcpwm_prelude.h"
#include "esp_log.h"

#define PIN_PWM_A   18  // First alternating pin
#define PIN_PWM_B   19  // Second alternating pin (inverted)

void setup()
{
    // 1. Initialize the MCPWM Timer
    mcpwm_timer_handle_t timer = NULL;
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000,                // 1 MHz resolution (1 tick = 1 microsecond)
        .period_ticks = 20000,                   // 20,000 ticks = 20ms period (50 Hz desired frequency)
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer));

    // 2. Initialize the MCPWM Operator
    mcpwm_oper_handle_t oper = NULL;
    mcpwm_operator_config_t oper_config = {
        .group_id = 0, // Must belong to the same group as the timer
    };
    ESP_ERROR_CHECK(mcpwm_new_operator(&oper_config, &oper));
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper, timer));

    // 3. Initialize Comparators to set the duty cycle
    mcpwm_cmpr_handle_t comparator = NULL;
    mcpwm_comparator_config_t cmpr_config = {
        .flags.update_cmp_on_tez = true,
    };
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &cmpr_config, &comparator));
    // Set 25% duty cycle (5000 ticks out of 20000)
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, 5000)); 

    // 4. Initialize Generators linked to physical GPIOs
    mcpwm_gen_handle_t gen_a = NULL;
    mcpwm_generator_config_t gen_a_config = { .gen_gpio_num = PIN_PWM_A };
    ESP_ERROR_CHECK(mcpwm_new_generator(oper, &gen_a_config, &gen_a));

    mcpwm_gen_handle_t gen_b = NULL;
    mcpwm_generator_config_t gen_b_config = { .gen_gpio_num = PIN_PWM_B };
    ESP_ERROR_CHECK(mcpwm_new_generator(oper, &gen_b_config, &gen_b));

    // 5. Configure Generator Actions for alternating (Complementary) outputs
    // Generator A Logic (Active High Example)
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(gen_a, 
                    MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(gen_a, 
                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator, MCPWM_GEN_ACTION_LOW)));

    // Generator B Logic (Inverted relative to Generator A)
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(gen_b, 
                    MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_LOW)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(gen_b, 
                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator, MCPWM_GEN_ACTION_HIGH)));

    // 6. Enable and Start the Timer
    ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));
}

void loop(){
}
