#include <iostream>
#include <modbus/modbus.h>
#include <errno.h>
#include <vector>
using namespace std;


int save_exit(modbus_t *ctx) {
    modbus_close(ctx);
    modbus_free(ctx);
    return -1;
}
void counter_clear(vector<vector<uint8_t>>& counter_mode, modbus_t *ctx) {
    cout << "Clearing counter... " << endl;
    for(int i = 0; i < 7; i++) {
        counter_mode[i][1] = 1; // Counter mode: Clear
        counter_mode[i][2] = 1; // Counter mode: Clear overflow
        if(modbus_write_bits(ctx, (32 + i*4), 4, counter_mode[i].data()) == -1) {
            cout << "Failed to write bits: " << modbus_strerror(errno) << endl;
            save_exit(ctx);
        }
        // ADAM会清0后相应寄存器自动重置0，不需要自己设置
        // sleep(0.2); // 200ms
        // counter_mode[i][1] = 0;
        // counter_mode[i][2] = 0;
        // if(modbus_write_bits(ctx, (32 + i*4), 4, counter_mode[i].data()) == -1) {
        //     cout << "Failed to write bits: " << modbus_strerror(errno) << endl;
        //     save_exit(ctx);
        // }
    }
}

// 用于监视DI的计数
int main() {
    modbus_t *ctx;

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
        save_exit(ctx);
    }

    // 设置应答延时1s
    modbus_set_response_timeout(ctx, 0, 1000000);

    // 启用调试模式
    modbus_set_debug(ctx, TRUE);

    // 设置DI4~6为计数模式
    vector<uint16_t> pin_mode(7, 224);
    for(int i = 4; i < 7; i++) {
        pin_mode[i] = 225; // 224: DI mode, 225: Counter mode
    }
    if(modbus_write_registers(ctx, 78, 7, pin_mode.data()) == -1) {
        cout << "Failed to write registers: " << modbus_strerror(errno) << endl;
        save_exit(ctx);
    }

    // counter start
    vector<vector<uint8_t>> counter_mode(7, vector<uint8_t>(4, 0));
    for(int i = 4; i < 7; i++) {
        counter_mode[i][0] = 1; // Counter mode: START
        // 开始计数
        if(modbus_write_bits(ctx, (32 + i*4), 4, counter_mode[i].data()) == -1) {
            cout << "Failed to write bits: " << modbus_strerror(errno) << endl;
            save_exit(ctx);
        }
    }

    // clear counter
    counter_clear(counter_mode, ctx);

    // 读取DI的计数（32bit）
    vector<uint16_t> counter(14, 0);
    while(true) {
        if(modbus_read_registers(ctx, 0, 14, counter.data()) == -1) {
            cout << "Failed to read registers: " << modbus_strerror(errno) << endl;
            save_exit(ctx);
        }
        cout << "DI counter: ";
        for(int i = 0; i < 14; i += 2) {
            cout << static_cast<int>(counter[i]) << " ";
        }
        cout << endl;
        sleep(1);
    }
    
    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}