#pragma once
#include <driver/gpio.h>


class TWAI_Txcvr {
public:

    enum class Type {
        TJA1050,
        MCP2551,
        SN65HVD23x,
        USER_DEFINED
    };

    struct Config {
        Type type;
        gpio_num_t standby_pin = GPIO_NUM_NC;
        gpio_num_t enable_pin = GPIO_NUM_NC;
        uint8_t custom_init[4] = {0};
        // Constructor para fácil inicialización
        Config(Type t, gpio_num_t stby = GPIO_NUM_NC, gpio_num_t en = GPIO_NUM_NC, const uint8_t* ci = nullptr);
    };

    // Constructor
    TWAI_Txcvr() = default;
    
    // Inicialización
    bool begin(Config& config);
    
    // Control de modos
    void set_normal_mode();
    void set_standby_mode();
    void set_silent_mode(bool silent = true);
    
    // Estado
    bool is_connected() const;

private:
    Config *cfg;
    bool initialized = false;
    
    void init_tja1050();
    void init_mcp2551();
    inline void write_pin(gpio_num_t pin, bool state);
};