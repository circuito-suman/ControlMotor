#ifndef EXAMPLEINTEGRATION_H
#define EXAMPLEINTEGRATION_H

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QStatusBar>
#include "MotorControlWidget.h"

/**
 * Example showing how to integrate MotorControlWidget into another project
 * This demonstrates how you can add motor control functionality to any existing application
 */
class ExampleIntegration : public QMainWindow
{
    Q_OBJECT

public:
    ExampleIntegration(QWidget *parent = nullptr);
    ~ExampleIntegration();

private slots:
    void openMotorControl();
    void onMotorConnectionChanged(bool connected);
    void onMotorPositionChanged(const QString &axis, double position);
    void onMotorError(const QString &error);

private:
    void setupUI();

    // Your existing application widgets
    QPushButton *motorControlBtn;
    QLabel *motorStatusLabel;
    QLabel *positionLabel;

    // Motor control integration
    MotorControlWidget *motorWidget;
    bool motorControlVisible;
};

#endif // EXAMPLEINTEGRATION_H
