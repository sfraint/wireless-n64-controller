#include "ESP32-BLE-Gamepad/BleGamepad.h"
#include "ble_gamepad_main.h"
#include "input_poll.h"
#include "storage.h"

#include <stdio.h>
#include <sys/time.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/adc.h"
#include "driver/gpio.h"



// LED mode and misc related status indicators
uint32_t led_mode = 0;
bool bt_connected = false;
// Controller state-machine
#define STATE_STARTUP      0
#define STATE_CALIBRATION  1
#define STATE_RUNNING      2

uint32_t current_state = STATE_STARTUP;
bool startup_routine_running = true;

BleGamepad bleGamepad;
xQueueHandle gpio_evt_queue = NULL; // Button-press event queue
// When an edge change is detected used to count movement
int countx = 0;
int county = 0;
// Setup Var to hold Factor to move joystick to positive range
uint16_t factor_x = 0;
uint16_t factor_y = 0;
// X Interrupt Hook 
static void IRAM_ATTR gpiox_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    if(gpio_get_level((gpio_num_t) SIXPIN_ANALOG_X) == gpio_get_level((gpio_num_t) SIXPIN_ANALOG_XQ)){ 
                        countx++;
                    }
                    else{
                        countx--;
                    }    
                     
}
// Y Interrupt Hook
static void IRAM_ATTR gpioy_isr_handler(void* arg)
{
       uint32_t gpio_num = (uint32_t) arg;
    if(gpio_get_level((gpio_num_t) SIXPIN_ANALOG_Y) == gpio_get_level((gpio_num_t) SIXPIN_ANALOG_YQ)){ 
                        county--;
                    }
                    else{
                        county++;
                    } 
}


//Return countx and county values 0 = countx
uint16_t get_sixpin_count(int32_t type) {
    if(type == 0){
        //printf("X: %d\n",countx);
        return countx+abs(min_x);
    }
    else{
        //printf("X: %d\n",county);
        return county+abs(min_y);
    }
     
}
// Setup specified pin number as pulled-down, input pin
void setup_input_pin(uint32_t pin) {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = ((1ULL << pin));
    io_conf.pull_down_en = (gpio_pulldown_t) 1;
    io_conf.pull_up_en = (gpio_pullup_t) 0;
    gpio_config(&io_conf);
}

// Setup GPIO
void setup_gpio()
{
    // Standard gamepad buttons
    for (uint32_t i = 0 ; i < NUM_OF_BUTTONS ; i++)
    {
        setup_input_pin(buttonPins[i]);
        previousButtonStates[i] = 0;
        currentButtonStates[i] =  0;
    }
    // D-pad
    for (uint32_t i = 0; i < 4; i++) {
        setup_input_pin(dpadPins[i]);
        previousDpadStates[i] = 0;
        currentDpadStates[i] =  0;
    }

    // Battery Monitor, Pin also used with 6 pin Joystick, therefore only availble with 4 pin joysticks
    if(!SIXPIN_ENABLED){
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ANALOG_BAT, ADC_ATTEN_DB_11);
    }   

    // SIXPIN Joystick setup
    if(SIXPIN_ENABLED){
        //zero-initialize the config structure.
        gpio_config_t io_conf = {};
        //disable pull-down mode
        io_conf.pull_down_en = (gpio_pulldown_t) 0;
        //interrupt of any edge
        io_conf.intr_type = GPIO_INTR_ANYEDGE;
        //bit mask of the pins
        io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
        //set as input mode
        io_conf.mode = GPIO_MODE_INPUT;
        //disable pull-up mode
        io_conf.pull_up_en = (gpio_pullup_t) 0;
        gpio_config(&io_conf);
        
        //SIXPIN Joystick Interrupts and Handler setup
        //install gpio isr service
        gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
        //hook isr handler for specific gpio pin
        gpio_isr_handler_add((gpio_num_t) SIXPIN_ANALOG_X, gpiox_isr_handler, (void*) SIXPIN_ANALOG_X);
        //hook isr handler for specific gpio pin
        gpio_isr_handler_add((gpio_num_t) SIXPIN_ANALOG_Y, gpioy_isr_handler, (void*) SIXPIN_ANALOG_Y);

        //Adjust max ADC reading for SIXPIN joystick potentiometers
        max_x = SIXPIN_ANALOG_MAX; 
        max_y = SIXPIN_ANALOG_MAX;
        center_x = SIXPIN_ANALOG_CENTER;
        center_y = SIXPIN_ANALOG_CENTER;
        factor_x = 40;
        factor_y = 40;
    }
    else{
    adc1_config_channel_atten(ANALOG_X, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ANALOG_Y, ADC_ATTEN_DB_11);
    }
    // Other IO
    gpio_config_t io_conf;
    // Button feedback LED output
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = ((1ULL << LED_IO_PIN));
    io_conf.pull_down_en = (gpio_pulldown_t) 0;
    io_conf.pull_up_en = (gpio_pullup_t) 0;
    gpio_config(&io_conf);
    // Management button
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = ((1ULL << MGMT_IO_PIN));
    io_conf.pull_down_en = (gpio_pulldown_t) 0;
    io_conf.pull_up_en = (gpio_pullup_t) 0;
    gpio_config(&io_conf);
}

void setup_bt() {
    uint32_t count = NUM_OF_BUTTONS + NUM_OF_SOFT_BUTTONS;
    uint32_t hat_count = 0;
    if (ENABLE_HAT_DPAD) {
        hat_count = 2;
    }
    printf("Setting up BT controller w/ %d buttons (hat dpad %d, hat count %d)", count, ENABLE_HAT_DPAD, hat_count);
    bleGamepad.begin(count, hat_count, true, true, false, false, false, false, false, false, false, false, false, false, false);
    bleGamepad.setAutoReport(false);
}


static void led_toggle_helper(uint32_t val) {
    gpio_set_level((gpio_num_t) LED_IO_PIN, val);
}


// Control LED output
static void led_handler(void* arg)
{
    const TickType_t xDelay = 50 / portTICK_PERIOD_MS;  // 50ms
    uint32_t count = 0;
    for(;;) {
        const uint32_t on = 1;
        const uint32_t off = 0;
        uint32_t very_slow = (count % 32) == 1;
        uint32_t slow = (count % 16) == 1;
        uint32_t medium = (count % 8) == 1;
        uint32_t fast = (count % 4) == 1;
        uint32_t very_fast = (count % 2) == 1;
        bt_connected = bleGamepad.isConnected();
        if (current_state == STATE_STARTUP) {
            // Starting up
            led_toggle_helper(on);
        } else if (current_state == STATE_CALIBRATION) {
            led_toggle_helper(very_fast);
        } else if (!bt_connected) {
            // Waiting for BT connection
            led_toggle_helper(medium);
        } else {
            // Connected, running like normal
            led_toggle_helper(very_slow);
        }
        // TODO re-incorporate or remove `led_mode`
        /*} else if (led_mode == LED_OFF) {
            led_toggle_helper(off);
        } else if (led_mode == LED_ON) {
            led_toggle_helper(on);
        } else if (led_mode == LED_BLINK_SLOW) {
            led_toggle_helper(slow);
        } else if (led_mode == LED_BLINK_MED) {
            led_toggle_helper(medium);
        } else if (led_mode == LED_BLINK_FAST) {
            led_toggle_helper(fast);
        } else if (led_mode == LED_BLINK_VERY_FAST) {
            led_toggle_helper(very_fast);
        }*/
        vTaskDelay(xDelay);
        count += 1;
        if (count > 31) count = 0;
    }
}


// Handle button press events
static void gpio_button_handler(void* arg)
{
    const TickType_t xDelay = 500 / portTICK_PERIOD_MS;  // 500ms
    uint32_t button_press_event;
    uint32_t last_event = NONE;
    //gpio_set_level((gpio_num_t) GPIO_OUTPUT_IO_1, 1);
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &button_press_event, portMAX_DELAY)) {

            // Button UP events
            if (button_press_event == BUTTON_UNPRESS) {
                printf("    (unpress received after %d)\n", last_event);
                //gpio_set_level((gpio_num_t) GPIO_OUTPUT_IO_0, 0);
                
                // Shutdown
                // https://randomnerdtutorials.com/esp32-external-wake-up-deep-sleep/
                // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/sleep_modes.html
                // https://github.com/espressif/esp-idf/blob/master/examples/system/deep_sleep/main/deep_sleep_example_main.c
                if (last_event == BUTTON_VERY_LONG_PRESS) {
                    // This is where you could do a shutdown procedure, e.g.
                    //esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 1); //1 = High, 0 = Low
                    //esp_deep_sleep_start();
                }
                continue;
            }

            // Button DOWN events
            if (button_press_event == BUTTON_PRESS) {
                printf("    (press received)\n");
            } else if (button_press_event == BUTTON_LONG_PRESS) {
                printf("    (long press received)\n");            
            } else if (button_press_event == BUTTON_VERY_LONG_PRESS) {
                printf("    (very long press received)\n");
            }
            last_event = button_press_event;
        }
    }
}

void setup_tasks()
{
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(gpio_button_handler, "gpio_button_handler", 2048, NULL, 10, NULL);
    xTaskCreate(led_handler, "led_handler", 2048, NULL, 10, NULL);
}

void setup_poll_task()
{
    xTaskCreate(input_poll_loop, "in_poll", 4096, NULL, 10, NULL);
}


extern "C" {
   void app_main();
}


// Calibrate analog X-Y inputs and optionally write to persistent storage. Uses initial values as "centered", then monitors min and max values for ~10 seconds to determine range. 
void calibrate(bool write_to_storage) {
    const TickType_t xDelay = 10 / portTICK_PERIOD_MS;  // 10ms

    if (SIXPIN_ENABLED){center_x = 0;} else{center_x = get_analog_raw(ANALOG_X);};
    min_x = center_x;
    max_x = center_x;

    if (SIXPIN_ENABLED){center_y = 0;} else{center_y = get_analog_raw(ANALOG_Y);};
    min_y = center_y;
    max_y = center_y;

    // Read extreme values for ~10 seconds
    for (int i = 0; i < 1000; i++) {
        int32_t  x;
        if (SIXPIN_ENABLED) {x = countx;} else{x = get_analog_raw(ANALOG_X);};
        
        if (x < min_x) min_x = x;
        if (x > max_x) max_x = x;

        int32_t  y;
        if (SIXPIN_ENABLED) {y = county;} else{y = get_analog_raw(ANALOG_Y);};
       
        if (y < min_y) min_y = y;
        if (y > max_y) max_y = y;

        vTaskDelay(xDelay);
    }

    printf("Calibration results:\n");
    printf("X (left, center, right): %d, %d, %d\n", min_x, center_x, max_x);
    printf("Y (up, center, down):    %d, %d, %d\n", min_y, center_y, max_y);
    if (write_to_storage) {
        write_storage_values(STORAGE_KEY_X_CENTER, center_x, STORAGE_KEY_X_MIN, min_x, STORAGE_KEY_X_MAX, max_x);
        write_storage_values(STORAGE_KEY_Y_CENTER, center_y, STORAGE_KEY_Y_MIN, min_y, STORAGE_KEY_Y_MAX, max_y);
        printf("Calibration data written to storage\n");
    }
}


void app_main(void)
{
    startup_routine_running = true;
    current_state = STATE_STARTUP;
    led_mode = LED_ON;
    bool storage_ok = init_storage();
    setup_gpio();
    setup_tasks();
    setup_bt();
    const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;  // 1s
    vTaskDelay(xDelay);
    
    // Get initial button state to enter special modes
    poll_buttons();

    // Enter calibration mode if `START` is being pressed
    if (currentButtonStates[6]) {
        current_state = STATE_CALIBRATION;
        printf("Starting calibration\n");
        calibrate(storage_ok);
    } else {
        printf("Skipping calibration\n");
    }

    if (storage_ok) {
        center_x = read_storage_value(STORAGE_KEY_X_CENTER, ANALOG_CENTER);
        min_x = read_storage_value(STORAGE_KEY_X_MIN, ANALOG_MIN);
        max_x = read_storage_value(STORAGE_KEY_X_MAX, ANALOG_MAX);
        center_y = read_storage_value(STORAGE_KEY_Y_CENTER, ANALOG_CENTER);
        min_y = read_storage_value(STORAGE_KEY_Y_MIN, ANALOG_MIN);
        max_y = read_storage_value(STORAGE_KEY_Y_MAX, ANALOG_MAX);
        printf("Read joystick calibration values:\n");
        printf("    X: %d, %d, %d\n", min_x, center_x, max_x);
        printf("    Y: %d, %d, %d\n", min_y, center_y, max_y);
    }

    printf("Initial setup complete!\n");
    
    startup_routine_running = false;
    current_state = STATE_RUNNING;
    setup_poll_task();

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
            CONFIG_IDF_TARGET,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());
}


