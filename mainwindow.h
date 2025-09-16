

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "TinybeeController.h"
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QTextEdit>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QVector>

struct AxisMeasurement {
    double min = NAN, mid = NAN, max = NAN;
};

class AxisControlWidget : public QGroupBox {
    Q_OBJECT
public:
    AxisControlWidget(const QString& axis, QWidget* parent = nullptr);

    void setPosition(double pos);
    void setEnabledAll(bool enabled);

    QString axisName;
    QLabel *posLabel, *goLabel;
    QPushButton *homeBtn, *moveMinusBtn, *movePlusBtn, *goBtn;
    QPushButton *markMinBtn, *markMidBtn, *markMaxBtn;
    QDoubleSpinBox *stepSpin, *goSpin;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);

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
    void updatePositionPoll(); // called by QTimer
    void handleSerialReadyRead();

private:
    void buildUI();
    void updateStatus(const QString& message);
    AxisMeasurement* measurement(const QString& axis);

    // UI
    QComboBox* portCombo;
    QPushButton *refreshBtn, *connectBtn, *disconnectBtn, *estopBtn;
    QLabel *statusLabel;
    QTabWidget* tabs;
    QVector<AxisControlWidget*> axisControls;
    QTextEdit* statusLog;

    // Serial
    QSerialPort* serial;
    QTimer* pollTimer;

    // Logic
    bool connected;
    AxisMeasurement measX, measY, measZ;
    QByteArray buffer;
    void handleSerialRead();
    double lastPosX = 0.0;
    double lastPosY = 0.0;
    double lastPosZ = 0.0;

};

#endif // MAINWINDOW_H
