// TinyBeeController.cpp
#include "TinybeeController.h"
#include <QElapsedTimer>
#include <QDebug>
#include <QRegularExpression>

TinyBeeController::TinyBeeController(QObject *parent)
    : QObject(parent)
{
    connect(&m_serial, &QSerialPort::readyRead, this, &TinyBeeController::onReadyRead);
    connect(&m_serial, &QSerialPort::errorOccurred, this, &TinyBeeController::onErrorOccurred);
}

TinyBeeController::~TinyBeeController()
{
    disconnectPort();
}

bool TinyBeeController::connectPort(const QString &portName, qint32 baudRate)
{
    if (m_serial.isOpen())
    {
        m_serial.close();
    }

    m_serial.setPortName(portName);
    m_serial.setBaudRate(baudRate);
    m_serial.setDataBits(QSerialPort::Data8);
    m_serial.setParity(QSerialPort::NoParity);
    m_serial.setStopBits(QSerialPort::OneStop);
    m_serial.setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial.open(QIODevice::ReadWrite))
    {
        m_hasError = true;
        QString err = QString("Failed to open serial port %1: %2").arg(portName, m_serial.errorString());
        emit errorOccurred(err);
        qCritical() << err;
        m_connected = false;
        return false;
    }

    // Clear buffers for clean start
    m_responseBuffer.clear();
    m_serial.clear(QSerialPort::AllDirections);

    m_connected = true;
    m_hasError = false;
    emit connected();
    qInfo() << "Serial port opened:" << portName << "at baud" << baudRate;
    return true;
}

void TinyBeeController::disconnectPort()
{
    if (m_serial.isOpen())
        m_serial.close();

    m_connected = false;
    emit disconnected();
    qInfo() << "Serial port closed";
}

bool TinyBeeController::isConnected() const
{
    return m_connected && m_serial.isOpen();
}

QString TinyBeeController::buildCommandString(const GCodeCommand &cmd) const
{
    switch (cmd.type)
    {
    case GCodeCommandType::FirmwareInfo:
        return "M115\n";
    case GCodeCommandType::Home:
        if (cmd.x == 0 && cmd.y == 0 && cmd.z == 0)
            return "G28\n"; // Home all axes
        else
        {
            QString axes;
            if (cmd.x != 0)
                axes += " X";
            if (cmd.y != 0)
                axes += " Y";
            if (cmd.z != 0)
                axes += " Z";
            return QString("G28%1\n").arg(axes);
        }
    case GCodeCommandType::Move:
        return QString("G1 X%1 Y%2 Z%3 F%4\n")
            .arg(cmd.x, 0, 'f', 3)
            .arg(cmd.y, 0, 'f', 3)
            .arg(cmd.z, 0, 'f', 3)
            .arg(cmd.feedrate);
    case GCodeCommandType::EmergencyStop:
        return "M112\n";
    case GCodeCommandType::Custom:
        return cmd.customCommand.trimmed() + "\n";
    default:
        return QString();
    }
}

bool TinyBeeController::sendCommand(const GCodeCommand &cmd, QString *response, int timeoutMs)
{
    if (!isConnected())
    {
        QString err = "Cannot send command: Not connected to serial port";
        emit errorOccurred(err);
        qWarning() << err;
        return false;
    }

    QString cmdStr = buildCommandString(cmd);
    if (cmdStr.isEmpty())
    {
        qWarning() << "Empty command string built for GCodeCommand";
        return false;
    }

    QMutexLocker locker(&m_mutex);
    m_responseBuffer.clear();

    QByteArray data = cmdStr.toUtf8();
    if (m_serial.write(data) == -1)
    {
        QString err = QString("Failed to write command to serial port: %1").arg(cmdStr.trimmed());
        emit errorOccurred(err);
        qCritical() << err;
        return false;
    }

    if (!m_serial.waitForBytesWritten(timeoutMs))
    {
        QString err = QString("Timeout waiting for bytes to be written: %1").arg(cmdStr.trimmed());
        emit errorOccurred(err);
        qCritical() << err;
        return false;
    }

    QString resp;
    if (!waitForResponse(resp, timeoutMs))
    {
        QString err = QString("Timeout or incomplete response for command: %1").arg(cmdStr.trimmed());
        emit errorOccurred(err);
        qWarning() << err;
        return false;
    }

    resp = resp.trimmed();
    if (response)
        *response = resp;

    qInfo() << "Command:" << cmdStr.trimmed() << "; Response:" << resp;
    return true;
}

bool TinyBeeController::waitForResponse(QString &outResponse, int timeoutMs)
{
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < timeoutMs)
    {
        if (m_serial.waitForReadyRead(timeoutMs - timer.elapsed()))
        {
            QByteArray newData = m_serial.readAll();
            m_responseBuffer.append(newData);

            // Customize per device: response done if contains "ok" or ends with newline
            if (m_responseBuffer.contains("\nok") || m_responseBuffer.endsWith('\n'))
            {
                outResponse = QString::fromUtf8(m_responseBuffer);
                m_responseBuffer.clear();
                return true;
            }
        }
    }

    outResponse.clear();
    return false; // Timeout
}

void TinyBeeController::onReadyRead()
{
    QByteArray data = m_serial.readAll();
    QMutexLocker locker(&m_mutex);
    m_responseBuffer.append(data);
}

void TinyBeeController::onErrorOccurred(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError)
        return;
    QString err = QString("Serial port error: %1").arg(m_serial.errorString());
    emit errorOccurred(err);
    qCritical() << err;
    m_hasError = true;
}

bool TinyBeeController::parseResponse(const QString &response, QHash<QString, QString> &parsed)
{
    parsed.clear();
    // Example: "X:10.00 Y:15.00 Z:5.00 E:0.00 Count X:1000 Y:1500 Z:500"
    QStringList parts = response.trimmed().split(' ', Qt::SkipEmptyParts);

    for (const QString &part : parts)
    {
        int colonIndex = part.indexOf(':');
        if (colonIndex > 0 && colonIndex < part.length() - 1)
        {
            QString key = part.left(colonIndex);
            QString val = part.mid(colonIndex + 1);
            parsed.insert(key, val);
        }
    }
    return !parsed.isEmpty();
}

bool TinyBeeController::getPosition(MotorPosition &pos, int timeoutMs)
{
    QString response;
    if (!sendCommand(GCodeCommand{GCodeCommandType::Custom, -1, 0, 0, 0, 0, "M114"}, &response, timeoutMs))
    {
        qWarning() << "Failed to get position (M114)";
        return false;
    }

    QHash<QString, QString> parsed;
    if (!parseResponse(response, parsed))
    {
        qWarning() << "Failed to parse position response:" << response;
        return false;
    }

    bool okX, okY, okZ;
    double xVal = parsed.value("X").toDouble(&okX);
    double yVal = parsed.value("Y").toDouble(&okY);
    double zVal = parsed.value("Z").toDouble(&okZ);

    if (!okX || !okY || !okZ)
    {
        qWarning() << "Position parse error from response:" << response;
        return false;
    }

    pos.x = xVal;
    pos.y = yVal;
    pos.z = zVal;

    emit positionUpdated(pos);
    return true;
}
