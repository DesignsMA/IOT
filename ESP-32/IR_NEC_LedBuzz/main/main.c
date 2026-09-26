#include "htcw_rmt_ir.h"
#include "esp_log.h"

static void ir_callback(
    rmt_ir_vendor_t brand,
    uint32_t code,
    size_t bits,
    void *state)
{
    // Imprime la marca, el código y la cantidad de bits recibidos en el log de ESP32
    ESP_LOGI("IR", "Marca: %d, codigo: 0x%08lx, bits: %u",
             brand, (unsigned long)code, (unsigned)bits);


}

void app_main(void)
{
    rmt_ir_recv_handle_t receiver;

    rmt_ir_recv_create(
        4,              // GPIO conectado al receptor IR
        ir_callback,
        NULL,
        &receiver
    );

    // El callback se ejecuta automáticamente al recibir un código.
}