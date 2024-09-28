#include <iostream>
#include <modbus/modbus.h>
#include <errno.h>
#include <vector>

using namespace std;

int main() {
    modbus_t *ctx = nullptr;
    int DO;
    bool flag;

    // 打开端口
    ctx = modbus_new_rtu("/dev/ttyUSB0", 9600, 'N', 8, 1);

    if(ctx == nullptr) {
        cout << "Unable to create the libmodbus context: " << modbus_strerror(errno) << endl;
        return -1;
    }

    // 设置从机地址
    modbus_set_slave(ctx, 2);

    // connect
    if(modbus_connect(ctx) == -1) {
        cout << "Connection to slave failed: " << modbus_strerror(errno) << endl;
        modbus_free(ctx);
        return -1;
    }
    // 设置应答延时
    modbus_set_response_timeout(ctx, 0, 1000000);

    // 启用调试模式
    modbus_set_debug(ctx, TRUE);

    //设定DI4~7为脉冲输出（pulse）模式
    vector<uint16_t> pin_mode(8, 0);
    for(int i = 4; i < 8; i++) {
        pin_mode[i] = 1;
    }
    if(modbus_write_registers(ctx, 85, 8, pin_mode.data()) == -1) {
        cout << "Failed to write registers: " << modbus_strerror(errno) << endl;
        modbus_close(ctx);
        modbus_free(ctx);
        return -1;
    }

    // 修改脉冲输出频率 （32bit）
    // 读取脉冲输出频率
    vector<uint16_t> Toffs(16, 0);
    vector<uint16_t> Tons(16, 0);
    if(modbus_read_registers(ctx, 14, 16, Toffs.data()) == -1 || modbus_read_registers(ctx, 30, 16, Tons.data()) == -1) {
        cout << "Failed to read registers: " << modbus_strerror(errno) << endl;
        modbus_close(ctx);
        modbus_free(ctx);
        return -1;
    }
    // 设置时间周期为1s（ADAM模块单位为0.1ms）, 输入占空比, 自动计算脉冲频率
    vector<float> duty_cycles = {0, 0, 0, 0, 0, 0, 0, 0,
                                 0.5, 0, 0.6, 0, 0.7, 0, 0.8, 0}; 
    for(int i = 0; i < 16; i += 2) {
        if(pin_mode[i / 2] == 1) {
            Tons[i] = 10000 * duty_cycles[i];
            Toffs[i] = 10000 - Tons[i];
        }
    }
    // 写入脉冲输出频率
    if(modbus_write_registers(ctx, 30, 16, Tons.data()) == -1 || modbus_write_registers(ctx, 14, 16, Toffs.data()) == -1) {
        cout << "Failed to write registers: " << modbus_strerror(errno) << endl;
        modbus_close(ctx);
        modbus_free(ctx);
        return -1;
    }

    // 设置脉冲输出次数 （32bit）
    vector<uint16_t> pulse_count(16, 0);
    for(int i = 0; i < 16; i += 2) {
        if(pin_mode[i / 2] == 1) {
            pulse_count[i] = 10;
        }
    }
    if(modbus_write_registers(ctx, 46, 16, pulse_count.data()) == -1) {
        cout << "Failed to write registers: " << modbus_strerror(errno) << endl;
        modbus_close(ctx);
        modbus_free(ctx);
        return -1;
    }

    // 关闭端口
    modbus_close(ctx);
    modbus_free(ctx);
    return 0;

}