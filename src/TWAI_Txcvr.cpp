#include "TWAI_Txcvr.h"
#include <driver/gpio.h>
#include <esp_timer.h>
#include <cstring>

TWAI_Txcvr::Config::Config(Type t, gpio_num_t stby, gpio_num_t en, const uint8_t* ci) 
            : type(t), standby_pin(stby), enable_pin(en) {
            if (ci) memcpy(custom_init, ci, 4);
            else memset(custom_init, 0, 4);
        }

bool TWAI_Txcvr::begin(Config& config) {
    cfg = &config;
    
    // Inicialización común
    if (cfg->standby_pin != GPIO_NUM_NC) {
        gpio_reset_pin(cfg->standby_pin);
        gpio_set_direction(cfg->standby_pin, GPIO_MODE_OUTPUT);
    }
    
    if (cfg->enable_pin != GPIO_NUM_NC) {
        gpio_reset_pin(cfg->enable_pin);
        gpio_set_direction(cfg->enable_pin, GPIO_MODE_OUTPUT);
    }

    // Inicialización específica
    switch(cfg->type) {
        case Type::TJA1050: init_tja1050(); break;
        case Type::MCP2551: init_mcp2551(); break;
        default: break;
    }
    
    initialized = true;
    return true;
}

void TWAI_Txcvr::init_tja1050() {
    // TJA1050 requiere STBY=LOW para modo normal
    write_pin(cfg->standby_pin, false);
    
    // EN=HIGH para habilitar transmisión (si está conectado)
    write_pin(cfg->enable_pin, true);
}

void TWAI_Txcvr::init_mcp2551() {
    // MCP2551 requiere STBY=HIGH para operación normal
    write_pin(cfg->standby_pin, true);
}

void TWAI_Txcvr::set_normal_mode() {
    switch(cfg->type) {
        case Type::TJA1050:
            write_pin(cfg->standby_pin, false); // STBY = LOW (modo normal)
            write_pin(cfg->enable_pin, true);   // EN = HIGH (habilitado)
            break;
        case Type::MCP2551:
            write_pin(cfg->standby_pin, true);  // STBY = HIGH (operación normal)
            break;
        default:
            break;
    }
}

void TWAI_Txcvr::set_standby_mode() {
    switch(cfg->type) {
        case Type::TJA1050:
            write_pin(cfg->standby_pin, true);  // STBY = HIGH (standby)
            break;
        case Type::MCP2551:
            write_pin(cfg->standby_pin, false); // STBY = LOW (standby)
            break;
        default:
            break;
    }
}

void TWAI_Txcvr::set_silent_mode(bool silent) {
    if (cfg->type == Type::TJA1050) {
        write_pin(cfg->standby_pin, silent);
    }
}

void TWAI_Txcvr::write_pin(gpio_num_t pin, bool state) {
    if (pin != GPIO_NUM_NC) {
        gpio_set_level(pin, state ? 1 : 0);
    }
}

bool TWAI_Txcvr::is_connected() const {
    if (!initialized) return false;

    // 1. Verificación básica de pines
    if (cfg->standby_pin != GPIO_NUM_NC) {
        if (gpio_get_level(cfg->standby_pin) == -1) { // Error al leer pin
            return false;
        }
    }

    int64_t last_time;

    // 2. Prueba de modo especial para TJA1050
    if (cfg->type == Type::TJA1050) {
        // Prueba de cambio de modo standby
        bool original_state = cfg->standby_pin != GPIO_NUM_NC ? 
                            gpio_get_level(cfg->standby_pin) : false;
        
        // Intentar cambiar estado
        gpio_set_level(cfg->standby_pin, !original_state);
        last_time = esp_timer_get_time();
        while (esp_timer_get_time() - last_time < 50); // Pequeño delay para estabilización
        
        bool new_state = cfg->standby_pin != GPIO_NUM_NC ? 
                       gpio_get_level(cfg->standby_pin) : false;
        
        // Restaurar estado original
        gpio_set_level(cfg->standby_pin, original_state);
        
        if (new_state == original_state) {
            return false; // No hubo cambio -> transceiver no responde
        }
    }

    // 3. Para MCP2551 verificar pull-up interno (si está habilitado)
    #ifdef CONFIG_IDF_TARGET_ESP32
    if (cfg->type == Type::MCP2551 && cfg->standby_pin != GPIO_NUM_NC) {
        gpio_set_pull_mode(cfg->standby_pin, GPIO_PULLUP_ONLY);
        last_time = esp_timer_get_time();
        while (esp_timer_get_time() - last_time < 10);
        if (gpio_get_level(cfg->standby_pin) == 0) {
            return false; // Pull-up no detectado
        }
        gpio_set_pull_mode(cfg->standby_pin, GPIO_FLOATING);
    }
    #endif

    return true; // Todas las pruebas pasaron
}
