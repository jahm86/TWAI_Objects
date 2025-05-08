/**
 * @file TWAI_BasicExample.ino
 * @brief Basic CAN controller usage example for ESP32
 * @details Demonstrates:
 * - TWAI controller initialization
 * - TJA1050 transceiver setup
 * - Message reception in dedicated task
 * - CAN bus error handling
 */

 #include <Arduino.h>
 #include <TWAI_Object.h>
 
 /**
  * @brief FreeRTOS task for CAN message processing
  * @param pvParameters Task parameters (unused)
  *
  * @details Receives messages from event queue and processes them:
  * - Prints normal messages with ID and data
  * - Reports bus error events
  */
 void can_receive_task(void *pvParameters) {
     TWAI_Object::can_event_t event;
 
     while (true) {
         // Wait for message (blocking operation)
         if (xQueueReceive(TWAI_Object::twai.get_event_queue(), &event, portMAX_DELAY)) {
 
             if (event.is_error) {
                 // Handle error event
                 Serial.println("[ERROR] CAN bus error detected");
             } else {
                 // Process normal message
                 Serial.printf("[RX] T:%u ID:0x%X DLC:%d Data:",
                               event.timestamp,
                               event.message.identifier,
                               event.message.data_length_code);
 
                 // Print data bytes
                 for (int i = 0; i < event.message.data_length_code; i++) {
                     Serial.printf(" %02X", event.message.data[i]);
                 }
                 Serial.println();
             }
         }
     }
 }
 
 TWAI_Txcvr txcvr; ///< CAN transceiver instance
 
 /**
  * @brief System initialization
  */
 void setup() {
     Serial.begin(115200);
 
     // TJA1050 transceiver configuration
     TWAI_Txcvr::Config tja_cfg(TWAI_Txcvr::Type::TJA1050, GPIO_NUM_15);
     txcvr.begin(tja_cfg);
     TWAI_Object::twai.link_transceiver(txcvr);
 
     // Initialize CAN controller at 500kbps
     if (!TWAI_Object::twai.begin(GPIO_NUM_5, GPIO_NUM_4, 500000)) {
         Serial.println("Error initializing CAN controller");
         while (1); // Halt on error
     }
 
     // Configure silent mode (listen-only)
     txcvr.set_silent_mode(true);
 
     // Create CAN reception task
     xTaskCreate(
         can_receive_task,     // Task function
         "CAN_RX",             // Descriptive name
         4096,                 // Stack size
         NULL,                 // Parameters
         tskIDLE_PRIORITY + 2, // Priority (higher than loop)
         NULL                  // Task handle
     );
 
     Serial.println("CAN system successfully initialized");
 }
 
 /**
  * @brief Main program loop
  * @note CAN processing happens in the separate task
  */
 void loop() {
     // Example: Add periodic message sending here
     delay(1000);
 }
 