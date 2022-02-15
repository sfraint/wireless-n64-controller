/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

// TODO
//   Add MGMT button functionality - holding the button will cause future presses and releases to affect button number + 16 (or w/e)
//   Calibrate / re-scale joystick potentiometer
//   Better labeling of button state - i.e. be able to figure out current state of BUTTON_START, BUTTON_A, etc.
//   Scan over report and fix it up to match my needs
//   Factor out constants / config into a header file
//   Trim back number of buttons
//   Use HAT instead of buttons for Dpad
//   Control external, soft power switch
//   Setup management task, handling BT host switching, re-pairing, shutdown/restart, etc.
//   Change reported button names
//   Misc cleanup

// Eventual TODO
//   Support connections to multiple clients


#include "ble_gamepad_main.h"

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



















#include "NimBLEDevice.h"

extern "C"{void app_main(void);}
 
static int startTime;
static int lastIntrTime;

// The remote service we wish to connect to.
//static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID serviceUUID((uint16_t) 0x1812);

// The characteristic of the remote service we are interested in.
//static BLEUUID    charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");
static BLEUUID    charUUID((uint16_t) 0x2a4d);

static bool doConnect = false;
static bool connected = false;
static bool doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    timezone tz;
    tz.tz_minuteswest = 0;
    tz.tz_dsttime = 0;
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    gettimeofday(&tv, &tz);
    int currTime = tv.tv_usec - startTime;
    printf("Notify callback time %d\n", currTime >> 10);
    
    printf("Notify callback for characteristic %s of data length %d data: %s\n",
           pBLERemoteCharacteristic->getUUID().toString().c_str(),
           length,
           (char*)pData);
    printf("\n\n\n                TIMEDELTA %d\n\n", (currTime - lastIntrTime) >> 10);
}

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */  
class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    printf("onDisconnect");
  }
/***************** New - Security handled here ********************
****** Note: these are the same return values as defaults ********/
  uint32_t onPassKeyRequest(){
    printf("Client PassKeyRequest\n");
    return 123456; 
  }
  bool onConfirmPIN(uint32_t pass_key){
    printf("The passkey YES/NO number: %d\n", pass_key);
    return true; 
  }

  void onAuthenticationComplete(ble_gap_conn_desc desc){
    printf("Starting BLE work!\n");
  }
/*******************************************************************/
};

bool connectToServer() {
    printf("Forming a connection to %s\n", myDevice->getAddress().toString().c_str());

    BLEClient*  pClient  = BLEDevice::createClient();
    printf(" - Created client\n");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    printf(" - Connected to server\n");

    std::vector<NimBLERemoteService*>* services = pClient->getServices();
    printf(" - services size %d\n\n\n\n", services->size());
    // No services for some reason...not sure what's going
    for(NimBLERemoteService* service : *services)
    {
    printf("  service: %s\n", service->toString().c_str());
    //ignore value
    //Value v = iter->second;
    }

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      printf("Failed to find our service UUID: %s\n", serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    printf(" - Found our service\n");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      printf("Failed to find our characteristic UUID: %s\n", charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    printf(" - Found our characteristic\n");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      printf("The characteristic value was: %s\n", value.c_str());
      }
    /** registerForNotify() has been deprecated and replaced with subscribe() / unsubscribe().
     *  Subscribe parameter defaults are: notifications=true, notifyCallback=nullptr, response=false.
     *  Unsubscribe parameter defaults are: response=false. 
     */
    if(pRemoteCharacteristic->canNotify()) {
        //pRemoteCharacteristic->registerForNotify(notifyCallback);
        printf("Registered for notify\n");
        pRemoteCharacteristic->subscribe(true, notifyCallback);
    } else {
        printf("Could not register for notify\n");
    }

    connected = true;
    return true;
}

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
   
/*** Only a reference to the advertised device is passed now
  void onResult(BLEAdvertisedDevice advertisedDevice) { **/     
  void onResult(BLEAdvertisedDevice* advertisedDevice) {
    const char *device_name_ptr = "ESP32 BLE Gamepad";
    //const char *actual_device_name_ptr = advertisedDevice->getName().c_str();
    std::string actual_device_name = advertisedDevice->getName();
    printf("BLE Advertised Device found (%s): %s\n", actual_device_name.c_str(), advertisedDevice->toString().c_str());
    //printf("   strcmp result is             %d\n", strcmp(actual_device_name_ptr, device_name_ptr));
    //printf("   strcmp std::string result is %d\n", actual_device_name == device_name_ptr);
    //printf("   strcmp result is %d\n", advertisedDevice->toString().begin("Name: ESP32 BLE Gamepad"));
    //printf("   strcmp result sanity check   %d\n", strcmp("ESP32 BLE Gamepad", device_name_ptr));

    // We have found a device, let us now see if it contains the service we are looking for.
/********************************************************************************
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
********************************************************************************/
    if (actual_device_name == device_name_ptr || (advertisedDevice->haveServiceUUID() && advertisedDevice->isAdvertisingService(serviceUUID))) {
      printf("    intervals: min: %d  max: %d  adv: %d\n", advertisedDevice->getMinInterval(), advertisedDevice->getMaxInterval(), advertisedDevice->getAdvInterval());
      printf("found match!\n\n\n\n\n");
      BLEDevice::getScan()->stop();
/*******************************************************************
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
*******************************************************************/
      myDevice = advertisedDevice; /** Just save the reference now, no need to copy the object */
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks


// This is the Arduino main loop function.
void connectTask (void * parameter){
    for(;;) {
      // If the flag "doConnect" is true then we have scanned for and found the desired
      // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
      // connected we set the connected flag to be true.
      if (doConnect == true) {
        if (connectToServer()) {
          printf("We are now connected to the BLE Server.\n");
        } else {
          printf("We have failed to connect to the server; there is nothin more we will do.\n");
        }
        doConnect = false;
      }

      // If we are connected to a peer BLE Server, update the characteristic each time we are reached
      // with the current time since boot.
      if (connected) {
      // Skip writing stuff for this debug code
        if (false) {
        char buf[256];
        snprintf(buf, 256, "Time since boot: %lu", (unsigned long)(esp_timer_get_time() / 1000000ULL));
        printf("Setting new characteristic value to %s\n", buf);
        
        // Set the characteristic's value to be the array of bytes that is actually a string.
        /*** Note: write value now returns true if successful, false otherwise - try again or disconnect ***/
        pRemoteCharacteristic->writeValue((uint8_t*)buf, strlen(buf), false);
        }
      }else if(doScan){
        BLEDevice::getScan()->start(0);  // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
      }
      
      vTaskDelay(1000/portTICK_PERIOD_MS); // Delay a second between loops.
    }
    
    vTaskDelete(NULL);
} // End of loop


#define GPIO_INPUT_IO_0     16
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT_IO_0)
#define ESP_INTR_FLAG_DEFAULT 0
static xQueueHandle gpio_intr_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_intr_queue, &gpio_num, NULL);
}

static void gpio_intr_handler(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_intr_queue, &io_num, portMAX_DELAY)) {
            timezone tz;
            tz.tz_minuteswest = 0;
            tz.tz_dsttime = 0;
            timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 0;
            gettimeofday(&tv, &tz);
            lastIntrTime = tv.tv_usec - startTime;
            printf("GPIO intr time %d\n", lastIntrTime >> 10);
            printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level((gpio_num_t) io_num));
        }
    }
}

void app_main(void) {
  // Setup time
  timezone tz;
  tz.tz_minuteswest = 0;
  tz.tz_dsttime = 0;
  timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  
  
    gpio_config_t io_conf = {};
    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    //bit mask of the pins, use GPIO16 / RX2
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-down mode
    io_conf.pull_up_en = (gpio_pullup_t) 0;
    io_conf.pull_down_en = (gpio_pulldown_t) 1;
    gpio_config(&io_conf);
    //change gpio intrrupt type for one pin
    //gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_ANYEDGE);
    gpio_set_intr_type((gpio_num_t) GPIO_INPUT_IO_0, GPIO_INTR_POSEDGE);
    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //create a queue to handle gpio event from isr
    gpio_intr_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(gpio_intr_handler, "gpio_intr_handler", 2048, NULL, 10, NULL);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add((gpio_num_t) GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);
    
    
  
  printf("Starting BLE Client application...\n");
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);

  settimeofday(&tv, &tz);
  gettimeofday(&tv, &tz);
  startTime = tv.tv_usec;
  
  xTaskCreate(connectTask, "connectTask", 5000, NULL, 1, NULL);
  pBLEScan->start(5, false);
} // End of setup.





