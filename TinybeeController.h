// TinyBeeController.h
#ifndef TINYBEECONTROLLER_H
#define TINYBEECONTROLLER_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>
#include <QHash>
#include <QMutex>

// Motor position representation
struct MotorPosition {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

// Enumerate command types with data encapsulation
enum class GCodeCommandType {
    FirmwareInfo,
    Home,
    Move,
    EmergencyStop,
    Custom
};

// Command container
struct GCodeCommand {
    GCodeCommandType type;
    int motorIndex = -1;
    double x = 0.0, y = 0.0, z = 0.0;
    int feedrate = 1000;
    QString customCommand;
};

class TinyBeeController : public QObject {
    Q_OBJECT
public:
    explicit TinyBeeController(QObject *parent = nullptr);
    ~TinyBeeController();

    // Serial port management
    bool connectPort(const QString& portName, qint32 baudRate = 115200);
    void disconnectPort();
    bool isConnected() const;

    // Command handling
    bool sendCommand(const GCodeCommand& cmd, QString* response = nullptr, int timeoutMs = 2000);

    // Parse key:value responses to map
    bool parseResponse(const QString& response, QHash<QString, QString>& parsed);

    // Retrieve motor position (M114)
    bool getPosition(MotorPosition& pos, int timeoutMs = 2000);

    // Status query
    bool connected() const { return m_connected; }
    bool hasError() const { return m_hasError; }

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);
    void positionUpdated(const MotorPosition& pos);
    void logMessage(const QString& msg);

private slots:
    void onReadyRead();
    void onErrorOccurred(QSerialPort::SerialPortError error);

private:
    QSerialPort m_serial;
    QByteArray m_responseBuffer;
    QMutex m_mutex;  // Thread safety

    bool m_connected = false;
    bool m_hasError = false;

    QString buildCommandString(const GCodeCommand& cmd) const;
    bool waitForResponse(QString& outResponse, int timeoutMs);
};

#endif // TINYBEECONTROLLER_H
