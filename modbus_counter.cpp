#include <iostream>
#include <modbus.h>
#include <errno.h>
#include <vector>


using namespace std;


int close_and_free_modbus(modbus_t *ctx) {
    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}
void counter_clear(vector<vector<uint8_t>>& counter_mode, modbus_t *ctx) {
    cout << endl << "Clearing counter... " << endl;
    for(int i = 0; i < 7; i++) {
        counter_mode[i][1] = 1; // Counter mode: Clear
        counter_mode[i][2] = 1; // Counter mode: Clear overflow
        if(modbus_write_bits(ctx, (32 + i*4), 4, counter_mode[i].data()) == -1) {
            cout << "Failed to write bits: " << modbus_strerror(errno) << endl;
            close_and_free_modbus(ctx);
        }
    }
    cout << "Finish clearing" << endl << endl;
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
        close_and_free_modbus(ctx);
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
        close_and_free_modbus(ctx);
    }

    // counter start
    vector<vector<uint8_t>> counter_mode(7, vector<uint8_t>(4, 0));
    for(int i = 0; i < 7; i++) {
        counter_mode[i][0] = 1; // Counter mode: START
        counter_mode[i][1] = 1; // Counter mode: Clear
        // counter_mode[i][2] = 1; // Counter mode: Clear overflow automatically
        counter_mode[i][3] = 1; // State: 重新上电清除计数
        // 写入线圈(单线圈写入)
        for(int j = 0; j < 4; j++) {
            if(modbus_write_bit(ctx, (32 + i*4 + j), static_cast<bool>(counter_mode[i][j])) == -1) {
                cout << "Failed to write bit: " << modbus_strerror(errno) << endl;
                close_and_free_modbus(ctx);
            }
        }
        // 读改动
        modbus_read_bits(ctx, (32 + i*4), 4, counter_mode[i].data());
        for(auto c:counter_mode[i]) {
            cout << static_cast<int>(c) << " ";
        }
        cout << endl;
    }

    // clear counter
    // counter_clear(counter_mode, ctx);

    // set filter（32bit）
    vector<uint16_t> filter_L(14, 0);
    vector<uint16_t> filter_H(14, 0);
    uint16_t L_width = 150; // 15ms
    uint16_t H_width = 150; // 15ms
    for(int i = 0; i < 14; i += 2) {
        filter_L[i] = L_width;
        filter_H[i] = H_width;
    }
    if(modbus_write_registers(ctx, 93, 14, filter_L.data()) == -1 || modbus_write_registers(ctx, 107, 14, filter_H.data()) == -1) {
        cout << "Failed to write registers: " << modbus_strerror(errno) << endl;
        close_and_free_modbus(ctx);
    }

    // 读取DI的计数（32bit）
    vector<uint16_t> counter(14, 0);
    while(true) {
        if(modbus_read_registers(ctx, 0, 14, counter.data()) == -1) {
            cout << "Failed to read registers: " << modbus_strerror(errno) << endl;
            close_and_free_modbus(ctx);
        }
        cout << "DI counter: ";
        for(int i = 0; i < 14; i += 2) {
            cout << static_cast<int>(counter[i]) << " ";
        }
        cout << endl;
        sleep(1);
    }
    
    close_and_free_modbus(ctx);
}