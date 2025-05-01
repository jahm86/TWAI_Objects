#pragma once
#include <driver/twai.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <vector>
#include "TWAI_Txcvr.h"

class TWAI_Object {
public:
    // Instancia global por defecto (para compatibilidad)
    static TWAI_Object twai;

    // Tipos
    typedef struct {
        twai_message_t message;
        uint32_t timestamp;
        bool is_error;
    } can_event_t;

    typedef enum {
        TWAI_FILTER_TYPE_MASK,    // Filtro por máscara
        TWAI_FILTER_TYPE_LIST,    // Filtro por lista
        TWAI_FILTER_TYPE_RANGE    // Filtro por rango
    } twai_filter_type_t;
    
    typedef struct {
        uint32_t id;             // ID base o valor inicial
        uint32_t mask_or_end_id; // Máscara o ID final (para rangos)
        twai_filter_type_t type;
        bool is_extended;        // ¿ID extendido (29-bit)?
    } twai_user_filter_t;

    // Constructor/destructor
    TWAI_Object();
    ~TWAI_Object();

    // Control básico
    bool begin(
        gpio_num_t tx_pin = GPIO_NUM_5,
        gpio_num_t rx_pin = GPIO_NUM_4,
        uint32_t baud_rate = 500000,
        twai_mode_t mode = TWAI_MODE_NORMAL,
        int controller_num = 0  // Para futuros ESP32 multi-CAN
    );
    
    void end();
    
    // Envío
    bool send(const twai_message_t& msg, TickType_t timeout = pdMS_TO_TICKS(100));
    
    // Filtros

    // Método único para configuración básica (reemplaza set_acceptance_filter)
    bool set_filter_mode(uint32_t acceptance_code,
        uint32_t acceptance_mask,
        bool is_extended);
            
    // Métodos para gestión avanzada
    bool add_filter(const twai_user_filter_t& filter);
    bool set_filters(const twai_user_filter_t* filters, uint8_t count);
    bool clear_filters();

    // Eventos
    QueueHandle_t get_event_queue();
    void enable_error_events(bool enable);

    // Estado
    twai_status_info_t get_status();
    bool is_bus_off();
    bool initiate_recovery();

    // Transceiver
    bool link_transceiver(TWAI_Txcvr& txcvr);

private:
    twai_general_config_t g_config;
    twai_timing_config_t t_config;
    twai_filter_config_t f_config;
    QueueHandle_t event_queue = nullptr;
    bool error_events_enabled = false;
    int controller_id = 0;
    std::vector<twai_user_filter_t> active_filters;    
    TWAI_Txcvr* connected_txcvr = nullptr;

    bool apply_hardware_filters();
    static void IRAM_ATTR twai_isr_handler(void* arg);
    void handle_interrupt();
};
