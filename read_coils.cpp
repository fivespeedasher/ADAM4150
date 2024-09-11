#include <iostream>
#include <modbus/modbus.h>
#include <errno.h>
#include <vector>
#include <unistd.h> // sleep
#include <termios.h>
#include <fcntl.h>

using namespace std;

bool kbhit() {
    termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF) {
        ungetc(ch, stdin);
        return true;
    }

    return false;
}

// 使用modbus_read_bits实时读取0~6线圈的状态
int main() {
    modbus_t *ctx = nullptr;
    vector<uint8_t> coils(7, false);

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
        modbus_close(ctx);
        return -1;
    }
    
    // 设置应答延时1s
    modbus_set_response_timeout(ctx, 0 ,1000000);

    // 启用调试模式
    modbus_set_debug(ctx, TRUE);

    // 读取线圈状态
    while(1) {
        if(modbus_read_bits(ctx, 0, 7, coils.data()) == -1) {
            cout << "Failed to read coils: " << modbus_strerror(errno) << endl;
            modbus_close(ctx);
            modbus_free(ctx);
            return -1;
        }
        cout << "Coils status: ";
        for(auto coil: coils) {
            cout << static_cast<int>(coil) << " ";
        }
        cout << endl << endl;
        sleep(1); // 1s

        // 检查是否有按键按下
        if (kbhit()) {
            cout << "Key pressed, exiting..." << endl;
            break;
        }
    }
    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}