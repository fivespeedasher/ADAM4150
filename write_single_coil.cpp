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
    // modbus_set_debug(ctx, TRUE);

    // 设置DO0~3为数字输出（DO）模式
    vector<uint16_t> pin_mode(8, 0);
    if(modbus_read_registers(ctx, 85, 8, pin_mode.data()) == -1) {
        cout << "Failed to read registers: " << modbus_strerror(errno) << endl;
        modbus_close(ctx);
        modbus_free(ctx);
        return -1;
    }
    for(int i = 0; i < 4; i++) {
        pin_mode[i] = 0;
    }
    if(modbus_write_registers(ctx, 85, 8, pin_mode.data()) == -1) {
        cout << "Failed to write registers: " << modbus_strerror(errno) << endl;
        modbus_close(ctx);
        modbus_free(ctx);
        return -1;
    }

/* 模式设定-逐个输入的方式 */
    // // 模式设定：指定寄存器为数字输出（DO）模式， 对应位置DOgital output mode 清0
    // int pin;
    // while(true) {
    //     cout << "Please input the pin(0~7) you want to set to DO mode: ";
    //     string input;
    //     getline(cin, input);
        
    //     // 如果输入为空，跳出循环
    //     if (input.empty()) {
    //         cout << "Input is empty, exiting..." << endl;
    //         break;
    //     }

    //     // 将输入转换为整数
    //     int pin;
    //     try {
    //         pin = stoi(input);
    //     } catch (invalid_argument&) {
    //         cout << "Invalid input, please enter a number between 0 and 7." << endl;
    //         continue;
    //     }

    //     // 检查 pin 是否在有效范围内
    //     if (pin >= 0 && pin <= 7) {
    //         pin_mode[pin] = 0;
    //         cout << "Set pin " << pin << " to DO mode" << endl;
    //     } else {
    //         cout << "Pin out of range, please enter a number between 0 and 7." << endl;
    //     }
    // }
/* end */

    
    if(modbus_write_registers(ctx, 85, 8, pin_mode.data()) == -1) {
        cout << "Failed to write registers: " << modbus_strerror(errno) << endl;
        modbus_close(ctx);
        modbus_free(ctx);
        return -1;
    }

    // 写单个线圈, 位置为DI(17~24)
    cout << "Please input the DI(0~7) and flag(0/1):" << endl;
    while(cin >> DO >> flag && DO >= 0 && DO <= 7) {
        int mapped_DO = 16 + DO;
        // 16~23是modbus上的地址，0~7是用户输入的地址
        if(modbus_write_bit(ctx, mapped_DO, flag) == -1) {
            cout << "Failed to write coil: " << modbus_strerror(errno) << endl;
            modbus_close(ctx);
            modbus_free(ctx);
            return -1;
        }
        cout << "Write coil successfully" << endl << endl;
        cout << "For continue, input the DO(0~7) and flag(0/1):" << endl;
    }
    cout << "DO out of range" << endl;

    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}