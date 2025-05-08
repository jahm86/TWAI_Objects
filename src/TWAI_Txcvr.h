#pragma once
#include <driver/gpio.h>

/**
 * @class TWAI_Txcvr
 * @brief Hardware abstraction for CAN transceivers (TJA1050, MCP2551, etc.)
 * 
 * @details Handles power modes, fault detection, and physical layer configuration.
 * Supports automatic recovery from bus-off states.
 */
class TWAI_Txcvr {
public:
    /**
     * @enum Type
     * @brief Supported transceiver ICs
     */
    enum class Type {
        TJA1050,        ///< NXP TJA1050 High-speed CAN transceiver
        MCP2551,        ///< Microchip MCP2551 CAN bus transceiver
        SN65HVD23x,     ///< TI SN65HVD23x Series Transceivers
        USER_DEFINED    ///< Custom transceiver implementation
    };

    /**
     * @struct Config
     * @brief Transceiver configuration parameters
     */
    struct Config {
        /**
         * @brief Construct new Config instance
         * @param t Transceiver type
         * @param stby Standby pin (GPIO_NUM_NC if unused)
         * @param en Enable pin (GPIO_NUM_NC if unused)
         * @param ci Custom init sequence (nullptr for default)
         */
        Config(Type t, gpio_num_t stby = GPIO_NUM_NC, gpio_num_t en = GPIO_NUM_NC, const uint8_t* ci = nullptr);

        Type type;                              ///< Transceiver model type
        gpio_num_t standby_pin = GPIO_NUM_NC;   ///< STBY/RST pin (GPIO_NUM_NC if unused)
        gpio_num_t enable_pin = GPIO_NUM_NC;    ///< EN/CS pin (GPIO_NUM_NC if unused)
        uint8_t custom_init[4] = {0};           ///< Custom initialization sequence
    };

    // Constructor
    TWAI_Txcvr() = default;
    
    /**
     * @brief Initialize transceiver hardware
     * @param config Configuration struct
     * @return true if initialization succeeded
     * @post Transceiver will be in normal operation mode
     * @warning Must be called before any other methods
     * @note Automatically configures GPIO pins if specified
     * 
     * @code{.cpp}
     * TWAI_Txcvr::Config cfg(TWAI_Txcvr::Type::TJA1050, GPIO_NUM_15);
     * if (!txcvr.begin(cfg)) {
     *     Serial.println("Transceiver init failed!");
     * }
     * @endcode
     */
    bool begin(Config& config);
    
    // Mode control

    /**
     * @brief Set transceiver to normal operation mode
     * @details Enables full CAN bus communication:
     * - TJA1050: STBY = LOW
     * - MCP2551: STBY = HIGH
     */
    void set_normal_mode();

    /**
     * @brief Set transceiver to low-power standby mode
     * @details Disables transmitter while maintaining bus monitoring:
     * - TJA1050: STBY = HIGH
     * - MCP2551: STBY = LOW
     */
    void set_standby_mode();

    /**
     * @brief Enable/disable silent mode (listen-only)
     * @param silent Whether to enable silent mode
     * @details Only affects TJA1050 transceivers
     */
    void set_silent_mode(bool silent = true);
    
    // Status

    /**
     * @brief Verify transceiver connection
     * @return true if transceiver responds to commands
     * @details Performs hardware-specific checks:
     * - TJA1050: Verifies STBY pin control
     * - MCP2551: Checks internal pull-up resistance
     */
    bool is_connected() const;

private:
    Config *cfg = nullptr;      ///< Pointer to active configuration
    bool initialized = false;   ///< Initialization status flag
    
    /**
     * @brief TJA1050-specific initialization
     * @note Configures standby and enable pins
     */
    void init_tja1050();

    /**
     * @brief MCP2551-specific initialization  
     * @note Configures standby pin behavior
     */
    void init_mcp2551();

    /**
     * @brief Safe GPIO write helper
     * @param pin Target GPIO number
     * @param state Desired pin state
     */
    inline void write_pin(gpio_num_t pin, bool state);
};
