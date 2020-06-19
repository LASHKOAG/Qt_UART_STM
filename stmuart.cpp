#include "stmuart.h"

typedef struct
{
    unsigned int command;
    unsigned int length;
    char* buff;
}tcp_packet_t;

StmUart::StmUart(const QString &name, QObject * parent)
    : QSerialPort(name, parent)
{
    m_portName = name;
    measurementNumber=1;
    check_availablePorts();
    set_openPort();
}

qint32 StmUart::check_availablePorts()
{
    qint32 countPort = 0;
    countPort = SerialPortInfo.availablePorts().count();

    //вернет номер ошибки, если нет доступных портов
    if(countPort < 1){return ERR_NO_AVAILABLE_PORTS;}

    //вернет количество доступных портов
    return countPort;
}

qint32 StmUart::set_openPort()
{
    //проверка на наличие возможных портов
    auto serialInfo = SerialPortInfo.availablePorts();

    //если подключений нет, вернуть номер ошибки
    if(serialInfo.isEmpty()){return ERR_NO_AVAILABLE_PORTS;}

    //проверка на валидность имени порта
    if (m_portName.isEmpty()) {
        qDebug() << "No port name ";
        return ERR_NO_PORT_NAME;
    }

    serialPort.setPortName(m_portName);

    //настройка порта
    bool res = serialPort.setBaudRate(QSerialPort::Baud9600)
    && serialPort.setDataBits(QSerialPort::Data8)
    && serialPort.setStopBits(QSerialPort::OneStop)
    && serialPort.setParity(QSerialPort::NoParity)
    && serialPort.open(QSerialPort::ReadWrite);
    qDebug() << "res" << res;
    return res ? 0 : ERR_DEVICE_NOT_OPEN;
}

qint32 StmUart::set_closePort()
{
    serialPort.close();
    return 0;
}

qint32 StmUart::write_request(quint32 task)
{
    tcp_packet_t _packet;
    memset(&_packet, 0x00, sizeof(tcp_packet_t));
    qint32 m_waitTimeout = 1500;

    _packet.command = task;
    _packet.length = sizeof(_packet.command);
    _packet.buff = nullptr;

    //--------write request------------------------------------------------------------------------
    char buffTemp[sizeof(tcp_packet_t)];
    memcpy(&buffTemp, &_packet, sizeof(tcp_packet_t));

    //формируем запрос
    QByteArray requestBuffer = QByteArray::fromRawData(buffTemp,sizeof(tcp_packet_t));
    qDebug() << requestBuffer.toHex();

    //отсылаем по UART
    qint64 result_serial_write = serialPort.write(requestBuffer);
    qDebug() << "result_serial_write = " << result_serial_write;
    if(result_serial_write < 0){return ERR_DEVICE_NOT_OPEN;}

    if (!serialPort.waitForBytesWritten(m_waitTimeout)) {
        qDebug() << "Wait read response timeout exceeded";
        return ERR_TIMEOUT_WRITE;
    }
    //-------end-write request------------------------------------------------------------------------

    return 0;
}

qint32 StmUart::get_paramDevice(INA230 &ina)
{
    tcp_packet_t packet;
    memset(&ina, 0x00, sizeof(INA230));
    memset(&packet, 0x00, sizeof(tcp_packet_t));

    qint32 result_write_request = write_request(CMD_GET_INAINFO);
    if(result_write_request != 0){return result_write_request;}

    // read response
    //время ожидания ответа от slave устройства
    qint32 currentWaitTimeout =3500;
    //чтение ответа с устройства с ожиданием(slave может ответить не сразу)
    if (serialPort.waitForReadyRead(currentWaitTimeout)) {
        QByteArray responseData = serialPort.readAll();
        while (serialPort.waitForReadyRead(10))
            //собираем все байты в один массив
            responseData += serialPort.readAll();
        qDebug() << "response raw hex:" << responseData.toHex();

        //разбираем ответ по соответствующим переменным структуры
        quint32 pos=0;
        quint32 command_answer;
        memcpy(&command_answer, responseData.data(), sizeof(command_answer));
        pos+=sizeof(command_answer);
        qDebug() << "command_answer:" << command_answer;
        packet.command = command_answer;

        quint32 length;
        memcpy(&length, responseData.data() + pos, sizeof(length));
        pos+=sizeof(length);
        qDebug() << "length:" << length;
        packet.length;

        quint32 command_receive;
        memcpy(&command_receive, responseData.data() + pos, sizeof(command_receive));
        pos+=sizeof(command_receive);
        qDebug() << "command_receive:" << command_receive;

        qint32 status_ans;
        memcpy(&status_ans, responseData.data() + pos, sizeof(status_ans));
        pos+=sizeof(status_ans);
        qDebug() << "status_ans:" << status_ans;
        if(status_ans == SLAVE_BAD_ANSWER){
            return ERR_UNSUCCESS_ANSWER;
        }

        quint32 hz;
        memcpy(&hz, responseData.data() + pos, sizeof(hz));
        pos+=sizeof(hz);
        qDebug() << "hz:" << hz;
        ina.hz = hz;

        float current_mA;
        memcpy(&current_mA, responseData.data() + pos, sizeof(current_mA));
        pos+=sizeof(current_mA);
        qDebug() << "current_mA:" << current_mA;
        ina.current_mA = current_mA;

        float power_W;
        memcpy(&power_W, responseData.data() + pos, sizeof(power_W));
        pos+=sizeof(power_W);
        qDebug() << "power_W:" << power_W;
        ina.power_W = power_W;

        float bus_voltage_V;
        memcpy(&bus_voltage_V, responseData.data() + pos, sizeof(bus_voltage_V));
        pos+=sizeof(bus_voltage_V);
        qDebug() << "bus_voltage_V:" << bus_voltage_V;
        ina.bus_voltage_V = bus_voltage_V;

        float shunt_voltage_V;
        memcpy(&shunt_voltage_V, responseData.data() + pos, sizeof(shunt_voltage_V));
        pos+=sizeof(shunt_voltage_V);
        qDebug() << "shunt_voltage_V:" << shunt_voltage_V;
        ina.shunt_voltage_V = shunt_voltage_V;

        qint16 current_reg;
        memcpy(&current_reg, responseData.data() + pos, sizeof(current_reg));
        pos+=sizeof(current_reg);
        qDebug() << "current_reg:" << current_reg;
        ina.current_reg = current_reg;

        quint16 config;
        memcpy(&config, responseData.data() + pos, sizeof(config));
        pos+=sizeof(config);
        qDebug() << "config:" << config;
        ina.config = config;

        quint16 calib;
        memcpy(&calib, responseData.data() + pos, sizeof(calib));
        pos+=sizeof(calib);
        qDebug() << "calib:" << calib;
        ina.calib = calib;

        quint16 die_id;
        memcpy(&die_id, responseData.data() + pos, sizeof(die_id));
        pos+=sizeof(die_id);
        qDebug() << "die_id:" << die_id;
        ina.die_id = die_id;

        uint8_t addr;
        memcpy(&addr, responseData.data() + pos, sizeof(addr));
        pos+=sizeof(addr);
        qDebug() << "addr:" << addr;
        ina.addr = addr;
    }else{
        qDebug() << "Wait write response timeout exceeded";
        return ERR_TIMEOUT_READ;
    }
    return 0;
}

qint32 StmUart::get_shutdownDevice()
{
    qint32 result_write_request = write_request(CMD_SHUTDOWN);
    if(result_write_request != 0){return result_write_request;}

    return 0;
}

qint32 StmUart::write_file (QString filename, INA230& inaData)
{
    QFile *file = new QFile(filename);

    if(!file->open(QFile::Append | QFile::Text)){
        qDebug() << " Could not open file for writing";
        return -1;
    }

    QTextStream out(file);

    out << measurementNumber << ";";
    out << QTime::currentTime().toString() << ";";
    out << inaData.power_W << ";";
    out << inaData.current_mA << ";";
    out << "\n";
    measurementNumber++;

    file->flush();
    file->close();
    delete file;
    return 0;
}

StmUart::~StmUart(){
    set_closePort();
}
