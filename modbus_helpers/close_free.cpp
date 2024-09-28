#include "close_free.h"

void close_and_free_modbus(modbus_t *ctx) {
    modbus_close(ctx);
    modbus_free(ctx);
}