#include "ExampleIntegration.h"
#include <QApplication>
#include <QMessageBox>
#include <QGroupBox>

ExampleIntegration::ExampleIntegration(QWidget *parent)
    : QMainWindow(parent),
      motorWidget(nullptr),
      motorControlVisible(false)
{
    setupUI();

    // Create motor control widget (initially hidden)
    motorWidget = new MotorControlWidget();

    // Connect signals to monitor motor control status
    connect(motorWidget, &MotorControlWidget::connectionStatusChanged,
            this, &ExampleIntegration::onMotorConnectionChanged);
    connect(motorWidget, &MotorControlWidget::positionChanged,
            this, &ExampleIntegration::onMotorPositionChanged);
    connect(motorWidget, &MotorControlWidget::errorOccurred,
            this, &ExampleIntegration::onMotorError);
}

ExampleIntegration::~ExampleIntegration()
{
    if (motorWidget)
    {
        delete motorWidget;
    }
}

void ExampleIntegration::setupUI()
{
    setWindowTitle("Example Application with Motor Control Integration");
    setMinimumSize(400, 300);

    QWidget *central = new QWidget;
    setCentralWidget(central);

    QVBoxLayout *layout = new QVBoxLayout;

    // Your existing application content
    QLabel *titleLabel = new QLabel("Your Main Application");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin: 20px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    // Motor control integration section
    QGroupBox *motorGroup = new QGroupBox("Motor Control Integration");
    QVBoxLayout *motorLayout = new QVBoxLayout;

    motorControlBtn = new QPushButton("Open Motor Control");
    connect(motorControlBtn, &QPushButton::clicked, this, &ExampleIntegration::openMotorControl);
    motorLayout->addWidget(motorControlBtn);

    motorStatusLabel = new QLabel("Motor Status: Disconnected");
    motorStatusLabel->setStyleSheet("font-weight: bold; color: red;");
    motorLayout->addWidget(motorStatusLabel);

    positionLabel = new QLabel("Position: X:0.00, Y:0.00, Z:0.00");
    positionLabel->setStyleSheet("font-family: monospace;");
    motorLayout->addWidget(positionLabel);

    motorGroup->setLayout(motorLayout);
    layout->addWidget(motorGroup);

    layout->addStretch(); // Push everything to top

    central->setLayout(layout);

    // Status bar
    statusBar()->showMessage("Ready");
}

void ExampleIntegration::openMotorControl()
{
    if (!motorWidget)
        return;

    if (motorControlVisible)
    {
        motorWidget->hideWidget();
        motorControlBtn->setText("Open Motor Control");
        motorControlVisible = false;
    }
    else
    {
        motorWidget->showWidget();
        motorControlBtn->setText("Close Motor Control");
        motorControlVisible = true;
    }
}

void ExampleIntegration::onMotorConnectionChanged(bool connected)
{
    if (connected)
    {
        motorStatusLabel->setText("Motor Status: Connected");
        motorStatusLabel->setStyleSheet("font-weight: bold; color: green;");
        statusBar()->showMessage("Motor controller connected");
    }
    else
    {
        motorStatusLabel->setText("Motor Status: Disconnected");
        motorStatusLabel->setStyleSheet("font-weight: bold; color: red;");
        positionLabel->setText("Position: X:0.00, Y:0.00, Z:0.00");
        statusBar()->showMessage("Motor controller disconnected");
    }
}

void ExampleIntegration::onMotorPositionChanged(const QString &axis, double position)
{
    // Update position display in main application
    static double xPos = 0.0, yPos = 0.0, zPos = 0.0;

    if (axis == "X")
        xPos = position;
    else if (axis == "Y")
        yPos = position;
    else if (axis == "Z")
        zPos = position;

    positionLabel->setText(QString("Position: X:%1, Y:%2, Z:%3")
                               .arg(xPos, 0, 'f', 2)
                               .arg(yPos, 0, 'f', 2)
                               .arg(zPos, 0, 'f', 2));
}

void ExampleIntegration::onMotorError(const QString &error)
{
    QMessageBox::warning(this, "Motor Control Error", error);
    statusBar()->showMessage("Motor error: " + error, 3000);
}
