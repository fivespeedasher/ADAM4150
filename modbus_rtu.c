#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "modbus.h"
#include <unistd.h> // for sleep

int main(void)
{
    const char *modbus_strerror(int errnum); 
    const char *errno;
    int i = 0;
    int rc = 0;
    modbus_t *ctx = NULL;
    uint16_t tab_reg[64] = {0};

    //打开端口: 端口，波特率，校验位，数据位，停止位
    ctx = modbus_new_rtu("/dev/ttyUSB0", 9600, 'N', 8, 1);

    if (ctx == NULL) {
        printf("Unable to create the libmodbus context\n");
        return -1;
    }
    //设置从机地址
    modbus_set_slave(ctx, 2);
    // //设置串口模式(可选)
    // modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);
    // //设置RTS(可选)
    // modbus_rtu_set_rts(ctx, MODBUS_RTU_RTS_UP);

    //建立连接
    if (modbus_connect(ctx) == -1) {
        printf("Connection to slave failed\n");
        modbus_free(ctx);
        return -1;
    }
    //设置应答延时(可选)
    modbus_set_response_timeout(ctx, 0, 1000000);

    // 写单个线圈，将地址为16的线圈置一
    if (modbus_write_bit(ctx, 17, TRUE) == -1) {
        fprintf(stderr, "Failed to write coil\n: %s", modbus_strerror(errno));
        modbus_close(ctx);
        modbus_free(ctx);
        return -1;
    }

    modbus_close(ctx);
    modbus_free(ctx);
    
    return 0;

}