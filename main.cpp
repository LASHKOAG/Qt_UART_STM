#include <iostream>
#include "stmuart.h"
//master
using namespace std;

int main()
{
    StmUart *stm_uart = new StmUart("COM4");
//    StmUart *stm_uart = new StmUart("COM7");
    INA230 ina;
while(1){
    qint32 res = stm_uart->get_paramDevice(ina);
//    qint32 res = stm_uart->get_shutdownDevice();

    qDebug() <<"result request = " << res;
    qDebug() <<"stm_uart->ina230.hz " << ina.hz;
    qDebug() <<"stm_uart->ina230.current_mA " << ina.current_mA;
    qDebug() <<"stm_uart->ina230.power_W " << ina.power_W;

    stm_uart->write_file ("D:/1ENERG/code/stm32/Q_Serial_Port_stm/UARTstm/measure1.txt", ina);
    QThread::sleep(10);
}

    delete stm_uart;

    return 0;
}
