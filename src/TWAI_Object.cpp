#include "TWAI_Object.h"
#include <cstring>

// Instancia global
TWAI_Object TWAI_Object::twai;

TWAI_Object::TWAI_Object() {}

TWAI_Object::~TWAI_Object() {
    end();
}

bool TWAI_Object::begin(gpio_num_t tx_pin, gpio_num_t rx_pin, 
                       uint32_t baud_rate, twai_mode_t mode, int controller_num) {
    controller_id = controller_num;

    // Guardar configuraciones
    g_config = TWAI_GENERAL_CONFIG_DEFAULT(tx_pin, rx_pin, mode);
    
    switch (baud_rate) {
        case 1000000: t_config = TWAI_TIMING_CONFIG_1MBITS(); break;
        case 800000:  t_config = TWAI_TIMING_CONFIG_800KBITS(); break;
        case 500000:  t_config = TWAI_TIMING_CONFIG_500KBITS(); break;
        case 250000:  t_config = TWAI_TIMING_CONFIG_250KBITS(); break;
        case 125000:  t_config = TWAI_TIMING_CONFIG_125KBITS(); break;
        case 100000:  t_config = TWAI_TIMING_CONFIG_100KBITS(); break;
        default: return false;
    }

    // Inicializar event_queue
    event_queue = xQueueCreate(MAX_EVENT_QUEUE_ITEMS, sizeof(can_event_t));
    if (event_queue == 0) {
        return false;
    }

    f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    
    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
        return false;
    }
    return twai_start() == ESP_OK;
}

bool TWAI_Object::link_transceiver(TWAI_Txcvr& txcvr) {
    connected_txcvr = &txcvr;
    return true;
}

bool TWAI_Object::set_filter_mode(uint32_t acceptance_code, uint32_t acceptance_mask, bool is_extended) {
    active_filters.clear();
    active_filters.push_back({
        .id = acceptance_code,
        .mask_or_end_id = acceptance_mask,
        .type = TWAI_FILTER_TYPE_MASK,
        .is_extended = is_extended
    });
    return apply_hardware_filters();
}

bool TWAI_Object::add_filter(const twai_user_filter_t& filter) {
    if (active_filters.size() >= 32) return false;
    
    active_filters.push_back(filter);
    return apply_hardware_filters();
}

bool TWAI_Object::set_filters(const twai_user_filter_t* filters, uint8_t count) {
    if (!filters || count > 32) return false;
    
    // Copia segura a buffer interno
    twai_user_filter_t temp_filters[32];
    memcpy(temp_filters, filters, sizeof(twai_user_filter_t) * count);
    
    // Limpiar y agregar nuevos filtros
    active_filters.clear();
    for (uint8_t i = 0; i < count; ++i) {
        active_filters.push_back(temp_filters[i]);
    }
    
    return apply_hardware_filters();
}

bool TWAI_Object::clear_filters() {    
    // Limpiar filtros
    active_filters.clear();
    
    return apply_hardware_filters();
}

bool TWAI_Object::apply_hardware_filters() {
    // 1. Detener el controlador temporalmente
    twai_stop();
    twai_driver_uninstall();

    // 2. Configurar filtros según active_filters
    twai_filter_config_t final_filter = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (!active_filters.empty()) {
        // Configuración para primer filtro
        const auto& primary_filter = active_filters[0];
        
        final_filter.acceptance_code = primary_filter.id << (primary_filter.is_extended ? 0 : 21);
        final_filter.acceptance_mask = 
            (primary_filter.type == TWAI_FILTER_TYPE_MASK) 
            ? (primary_filter.mask_or_end_id << (primary_filter.is_extended ? 0 : 21))
            : 0x7FFFFFFF; // Máscara completa para filtro de rango
    }

    // 3. Reinstalar driver con nuevos filtros
    esp_err_t err = twai_driver_install(&g_config, &t_config, &final_filter);
    if (err != ESP_OK) return false;

    // 4. Para ESP32-C3/C6 con doble banco de filtros:
    // TODO: Probar con estos controladores cuando los tenga disponibles.
    #if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
    if (active_filters.size() > 1) {
        const auto& secondary_filter = active_filters[1];
        TWAI.filter_reg_conf.acceptance_code = secondary_filter.id;
        TWAI.filter_reg_conf.acceptance_mask = secondary_filter.mask_or_end_id;
    }
    #endif

    return twai_start() == ESP_OK;
}

void IRAM_ATTR TWAI_Object::twai_isr_handler(void* arg) {
    TWAI_Object* instance = static_cast<TWAI_Object*>(arg);
    if (instance) {
        instance->handle_interrupt();
    }
}

void IRAM_ATTR TWAI_Object::handle_interrupt() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    twai_status_info_t status;
    twai_get_status_info(&status);

    if (status.msgs_to_rx > 0) {
        can_event_t event = {0};
        while (twai_receive(&event.message, 0) == ESP_OK) {
            event.timestamp = xTaskGetTickCountFromISR();
            xQueueSendFromISR(event_queue, &event, &xHigherPriorityTaskWoken);
        }
    }

    if (error_events_enabled && status.state == TWAI_STATE_BUS_OFF) {
        can_event_t event = {0};
        event.is_error = true;
        xQueueSendFromISR(event_queue, &event, &xHigherPriorityTaskWoken);
    }

    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

// Implementación del resto de métodos...
bool TWAI_Object::send(const twai_message_t& msg, TickType_t timeout) {
    return twai_transmit(&msg, timeout) == ESP_OK;
}

QueueHandle_t TWAI_Object::get_event_queue() {
    return event_queue;
}

void TWAI_Object::enable_error_events(bool enable) {
    error_events_enabled = enable;
}

twai_status_info_t TWAI_Object::get_status() {
    twai_status_info_t status;
    twai_get_status_info(&status);
    return status;
}

bool TWAI_Object::is_bus_off() {
    return get_status().state == TWAI_STATE_BUS_OFF;
}

bool TWAI_Object::initiate_recovery() {
    return twai_initiate_recovery() == ESP_OK;
}

void TWAI_Object::end() {
    if (event_queue) {
        vQueueDelete(event_queue);
        event_queue = nullptr;
    }
    twai_stop();
    twai_driver_uninstall();
}
