#ifndef STMUART_H
#define STMUART_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QObject>
#include <QString>
#include <QTime>
#include <QDebug>
#include <QThread>
#include <QFile>
#include <QTextStream>

#define SLAVE_SUCCESS_ANSWER            0       //ответ slave устройства успешен
#define SLAVE_BAD_ANSWER               -1       //ответ slave устройства содержит некорректные данные

#define CMD_ANSWER						1
#define CMD_GET_INAINFO					2       //получить данные о характеристиках акб посредством опроса датчика INA
#define CMD_SHUTDOWN					3       //послать команду о выключении прибора

#define ERR_NO_AVAILABLE_PORTS         -100     //портов для подключения нет
#define ERR_DEVICE_NOT_OPEN            -200     //например вместо "COM4" задано "COM7" или " "
#define ERR_NO_PORT_NAME               -300     //например вместо "COM4" задано ""
#define ERR_NO_EXIST_TASK              -400
#define ERR_TIMEOUT_READ               -500     //превышено время ожидания ответа slave устройства
#define ERR_TIMEOUT_WRITE              -600     //превышено время ожидания отправки сообщения в slave устройство
#define ERR_UNSUCCESS_ANSWER           -700     //slave устройство прислало, что ответ некорректен

typedef struct
{
    uint32_t hz;                //I2C frequency [Hz]
    float current_mA;           //read_current() [mA]
    float power_W;              //read_power() [W]
    float bus_voltage_V;        //read_bus_voltage() [V]
    float shunt_voltage_V;      //Read Shunt voltage data [V]
    int16_t current_reg;
    uint16_t config;
    uint16_t calib;
    uint16_t die_id;            //get the ID INA
    uint8_t addr;               //address INA
    uint8_t rsrvd_byte;
}INA230;

class StmUart : public QSerialPort
{
    Q_OBJECT

public:
    explicit StmUart(const QString &name, QObject *parent = nullptr);
    ~StmUart();
    //Проверка на наличие возможных портов
    //Возвращает количество портов или ошибку ERR_NO_AVAILABLE_PORTS
    qint32 check_availablePorts();

    //Мастер инициирует запрос подчиненному устрйству.
    //Предусмотрено два варианта запроса:
    //get_paramDevice - получить данные о характеристиках акб посредством опроса датчика INA
    //get_shutdownDevice - послать команду о выключении прибора

    //Возвращает 0 в случае успеха либо отрицательные значения
    qint32 get_paramDevice(INA230 &);
    qint32 get_shutdownDevice();

    //запись измерений в файл
    //Возвращает 0 в случае успеха либо отрицательные значения
    //Первый аргумент - это путь к месторасположению файла, 2 ой - полученная структура от slave устройства
    qint32 write_file (QString, INA230&);

private:
    QSerialPortInfo SerialPortInfo;
    QSerialPort serialPort;
    QString m_portName;
    quint32 measurementNumber;
    qint32 write_request(quint32);
    qint32 set_openPort();
    qint32 set_closePort();
};

#endif // STMUART_H
