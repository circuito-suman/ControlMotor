#ifndef MOTORCONTROLWIDGET_H
#define MOTORCONTROLWIDGET_H

#include <QWidget>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QVector>

struct AxisMeasurement
{
    double min = std::numeric_limits<double>::quiet_NaN();
    double mid = std::numeric_limits<double>::quiet_NaN();
    double max = std::numeric_limits<double>::quiet_NaN();
};

class AxisControlWidget : public QGroupBox
{
    Q_OBJECT
public:
    AxisControlWidget(const QString &axis, QWidget *parent = nullptr);

    void setPosition(double pos);
    void setEnabledAll(bool enabled);

    QString axisName;
    QLabel *posLabel, *goLabel;
    QPushButton *homeBtn, *moveMinusBtn, *movePlusBtn, *goBtn;
    QPushButton *markMinBtn, *markMidBtn, *markMaxBtn;
    QDoubleSpinBox *stepSpin, *goSpin;
};

class MotorControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MotorControlWidget(QWidget *parent = nullptr);
    ~MotorControlWidget();

    // Public interface for external integration
    bool isConnected() const;
    void showWidget();
    void hideWidget();

signals:
    void connectionStatusChanged(bool connected);
    void positionChanged(const QString &axis, double position);
    void errorOccurred(const QString &error);
    void commandExecuted(const QString &command, const QString &response);

public slots:
    void connectToPort(const QString &portName = "");
    void disconnectFromPort();
    void sendCustomCommand(const QString &command);

private slots:
    void refreshPorts();
    void connectPort();
    void disconnectPort();
    void directionalClicked();
    void axisHome();
    void axisMoveStep();
    void axisGoTo();
    void markPosition();
    void emergencyStop();
    void updatePositionPoll();
    void handleSerialReadyRead();
    void onCommandInputReturnPressed();

private:
    void setupUI();
    void updateStatus(const QString &message);
    AxisMeasurement *measurement(const QString &axis);

    // UI Components
    QComboBox *portCombo;
    QPushButton *refreshBtn, *connectBtn, *disconnectBtn, *estopBtn;
    QLabel *statusLabel;
    QTabWidget *tabs;
    QVector<AxisControlWidget *> axisControls;
    QTextEdit *statusLog;
    QLineEdit *commandInput;
    QPushButton *sendCommandBtn;

    // Serial Communication
    QSerialPort *serial;
    QTimer *pollTimer;

    // State
    bool connected;
    AxisMeasurement measX, measY, measZ;
    QByteArray buffer;
    double lastPosX = 0.0;
    double lastPosY = 0.0;
    double lastPosZ = 0.0;

    void handleSerialRead();
};

#endif // MOTORCONTROLWIDGET_H
