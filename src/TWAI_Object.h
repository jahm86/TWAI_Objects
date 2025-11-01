#pragma once
#include <driver/twai.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <vector>
#include "TWAI_Txcvr.h"

#ifndef MAX_EVENT_QUEUE_ITEMS
/**Maximun number of items in event queue*/
#define MAX_EVENT_QUEUE_ITEMS (8)
#endif  // MAX_EVENT_QUEUE_ITEMS

/**
 * @class TWAI_Object
 * @brief Main CAN controller interface for ESP32 TWAI peripheral
 * 
 * @details Provides high-level API for CAN communication with:
 * - Hardware filter management
 * - Interrupt-driven operation
 * - Transceiver integration
 */
class TWAI_Object {
public:
    // Default global instance (for compatibility)
    static TWAI_Object twai;

    // Types

    /**
     * @struct CAN_Event 
     * @brief CAN message or error event structure
     */
    typedef struct {
        twai_message_t message;  ///< Received CAN frame data
        uint32_t timestamp;      ///< FreeRTOS tick count when event occurred
        bool is_error;           ///< Flag indicating error event (true) or data message (false)
    } can_event_t;

    /**
     * @enum twai_filter_type_t
     * @brief Filter operation modes
     */
    typedef enum {
        TWAI_FILTER_TYPE_MASK,    ///< Bitmask filter (acceptance_code & acceptance_mask)
        TWAI_FILTER_TYPE_LIST,    ///< Per list filter
        TWAI_FILTER_TYPE_RANGE    ///< ID range filter (min <= ID <= max)
    } twai_filter_type_t;
    
    /**
     * @struct twai_user_filter_t
     * @brief User-defined filter configuration
     */
    typedef struct {
        uint32_t id;            ///< Filter base ID or minimum range value
        uint32_t mask_or_end_id;///< Mask bits or maximum range value
        twai_filter_type_t type;///< Filter operation mode
        bool is_extended;       ///< True for extended (29-bit) IDs
    } twai_user_filter_t;

    // Constructor/destructor
    TWAI_Object();
    ~TWAI_Object();

    /**
     * @brief Initialize CAN controller
     * @param tx_pin GPIO number for TX
     * @param rx_pin GPIO number for RX 
     * @param baud_rate CAN bus speed in bps
     * @param mode Operation mode (Normal/ListenOnly/NoAck)
     * @param controller_num ID of controller (for future ESP32 CAN implementations)
     * @return true if initialization succeeded
     * 
     * @throws std::runtime_error if pin configuration is invalid
     * @note Automatically enters reset then operation mode
     */
    bool begin(
        gpio_num_t tx_pin = GPIO_NUM_21,
        gpio_num_t rx_pin = GPIO_NUM_22,
        uint32_t baud_rate = 500000,
        twai_mode_t mode = TWAI_MODE_NORMAL,
        int controller_num = 0
    );
    
    void end();
    
    /**
     * @brief Send CAN message
     * @param msg Message to transmit
     * @param timeout Maximum wait time in ticks
     * @return true if message was queued for transmission
     * 
     * @warning Blocks if transmit queue is full
     * @note Actual transmission is handled asynchronously
     */
    bool send(const twai_message_t& msg, TickType_t timeout = pdMS_TO_TICKS(100));
    
    // Filters

    /**
     * @brief Configure basic filter mode
     * @param acceptance_code Filter pattern value
     * @param acceptance_mask Bitmask for pattern matching
     * @param is_extended True for extended ID filtering
     * @return True if configuration succeeded
     */
    bool set_filter_mode(uint32_t acceptance_code,
        uint32_t acceptance_mask,
        bool is_extended);
            
    /**
     * @brief Add single filter to active set
     * @param filter Filter configuration to add
     * @return True if filter was added successfully
     */
    bool add_filter(const twai_user_filter_t& filter);

    /**
     * @brief Replace all active filters
     * @param filters Array of filter configurations
     * @param count Number of filters in array
     * @return True if filters were applied
     */
    bool set_filters(const twai_user_filter_t* filters, uint8_t count);

    /**
     * @brief Remove all filters (accept all messages)
     * @return True if operation succeeded
     */
    bool clear_filters();

    // Events

    /**
     * @brief Get event queue handle
     * @return FreeRTOS queue handle for CAN events
     * 
     * @details Queue contains:
     * - Received messages (is_error = false)
     * - Error events (is_error = true)
     */
    QueueHandle_t get_event_queue();

    /**
     * @brief Enable/disable error event reporting
     * @param enable True to enable error events
     */
    void enable_error_events(bool enable);

    // Status

    /**
     * @brief Get current controller status
     * @return TWAI status information structure
     */
    twai_status_info_t get_status();

    /**
     * @brief Check bus-off state
     * @return True if controller is in bus-off condition
     */
    bool is_bus_off();

    /**
     * @brief Initiate bus recovery from bus-off state
     * @return True if recovery sequence started
     */
    bool initiate_recovery();

    // Physical interface

    /**
     * @brief Link transceiver instance to controller
     * @param txcvr Reference to initialized transceiver
     * @return true if association succeeded
     * 
     * @pre Transceiver must be initialized via begin()
     */
    bool link_transceiver(TWAI_Txcvr& txcvr);

private:
    twai_general_config_t g_config;                 ///< TWAI general configuration (pins, mode)
    twai_timing_config_t t_config;                  ///< Bit timing parameters (baudrate, sampling)
    twai_filter_config_t f_config;                  ///< Hardware filter settings
    QueueHandle_t event_queue = nullptr;            ///< FreeRTOS queue for CAN events
    bool error_events_enabled = false;              ///< Error event reporting flag
    int controller_id = 0;                          ///< Controller index (for multi-CAN chips)
    std::vector<twai_user_filter_t> active_filters; ///< Active filter configurations
    TWAI_Txcvr* connected_txcvr = nullptr;          ///< Linked transceiver instance

    /**
     * @brief Apply hardware filter configurations
     * @return True if filters were successfully programmed
     * @note Internal use - called automatically by filter management methods
     */
    bool apply_hardware_filters();

    /**
     * @brief Interrupt service routine wrapper
     * @param arg Pointer to TWAI_Object instance
     * @note Internal use - registered during begin()
     */
    static void IRAM_ATTR twai_isr_handler(void* arg);

    /**
     * @brief Main interrupt handler
     * @note Internal use - processes RX/TX interrupts
     */
    void handle_interrupt();
};
