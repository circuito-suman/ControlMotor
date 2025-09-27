#include "MotorControlWidget.h"
#include <QMessageBox>
#include <QApplication>
#include <QTime>
#include <QSplitter>
#include <QGroupBox>
#include <QRegularExpression>
#include <QFont>
#include <QDebug>
#include <limits>

// --- AxisControlWidget Implementation ---

AxisControlWidget::AxisControlWidget(const QString &axis, QWidget *parent)
    : QGroupBox(axis.toUpper() + " Axis", parent), axisName(axis.toLower())
{
    setStyleSheet(R"(
        QGroupBox { 
            font-weight: bold; 
            font-size: 12px; 
            border: 2px solid #6f42c1; 
            border-radius: 8px; 
            margin-top: 15px; 
            padding-top: 10px;
            background-color: #f8f9fa;
        }
        QGroupBox::title { 
            subcontrol-origin: margin; 
            left: 10px; 
            padding: 0 8px 0 8px; 
            color: #6f42c1;
            background-color: #f8f9fa;
        }
    )");

    QVBoxLayout *base = new QVBoxLayout;
    base->setSpacing(8);
    base->setContentsMargins(10, 15, 10, 10);

    // Position display
    posLabel = new QLabel("0.00 mm");
    posLabel->setAlignment(Qt::AlignCenter);
    posLabel->setStyleSheet(R"(
        QLabel { 
            font-weight: bold; 
            font-size: 14px; 
            color: #2d3748; 
            background-color: #e2e8f0; 
            border: 1px solid #cbd5e0; 
            border-radius: 4px; 
            padding: 6px;
            min-height: 16px;
        }
    )");
    base->addWidget(posLabel);

    // Step movement row
    QHBoxLayout *stepRow = new QHBoxLayout;
    stepRow->setSpacing(6);

    homeBtn = new QPushButton("Home");
    homeBtn->setMinimumSize(45, 28);
    homeBtn->setMaximumSize(55, 28);
    homeBtn->setStyleSheet("QPushButton { background: #28a745; color: white; font-weight: bold; border: none; border-radius: 3px; font-size: 9px; } QPushButton:hover { background: #218838; }");

    moveMinusBtn = new QPushButton("−");
    moveMinusBtn->setMinimumSize(28, 28);
    moveMinusBtn->setMaximumSize(28, 28);
    moveMinusBtn->setStyleSheet("QPushButton { background: #dc3545; color: white; font-weight: bold; border: none; border-radius: 3px; font-size: 12px; } QPushButton:hover { background: #c82333; }");

    stepSpin = new QDoubleSpinBox;
    stepSpin->setRange(0.1, 100);
    stepSpin->setValue(axis.toLower() == "z" ? 5.0 : 10.0);
    stepSpin->setSuffix(" mm");
    stepSpin->setMinimumHeight(28);
    stepSpin->setMaximumHeight(28);
    stepSpin->setStyleSheet("QDoubleSpinBox { padding: 3px; border: 1px solid #ced4da; border-radius: 3px; font-size: 9px; }");

    movePlusBtn = new QPushButton("+");
    movePlusBtn->setMinimumSize(28, 28);
    movePlusBtn->setMaximumSize(28, 28);
    movePlusBtn->setStyleSheet("QPushButton { background: #007bff; color: white; font-weight: bold; border: none; border-radius: 3px; font-size: 12px; } QPushButton:hover { background: #0056b3; }");

    stepRow->addWidget(homeBtn);
    stepRow->addWidget(moveMinusBtn);
    stepRow->addWidget(stepSpin, 1);
    stepRow->addWidget(movePlusBtn);
    base->addLayout(stepRow);

    // Goto row
    QHBoxLayout *gotoRow = new QHBoxLayout;
    gotoRow->setSpacing(6);

    goLabel = new QLabel("Go:");
    goLabel->setStyleSheet("font-weight: bold; font-size: 9px; color: #495057;");
    goLabel->setMinimumWidth(25);

    goSpin = new QDoubleSpinBox;
    goSpin->setRange(-999, 999);
    goSpin->setMinimumHeight(28);
    goSpin->setMaximumHeight(28);
    goSpin->setStyleSheet("QDoubleSpinBox { padding: 3px; border: 1px solid #ced4da; border-radius: 3px; font-size: 9px; }");

    goBtn = new QPushButton("Go");
    goBtn->setMinimumSize(35, 28);
    goBtn->setMaximumSize(45, 28);
    goBtn->setStyleSheet("QPushButton { background: #ffc107; color: #212529; font-weight: bold; border: none; border-radius: 3px; font-size: 9px; } QPushButton:hover { background: #e0a800; }");

    gotoRow->addWidget(goLabel);
    gotoRow->addWidget(goSpin, 1);
    gotoRow->addWidget(goBtn);
    base->addLayout(gotoRow);

    // Mark buttons row
    QHBoxLayout *markRow = new QHBoxLayout;
    markRow->setSpacing(4);

    QString markBtnStyle = R"(
        QPushButton { 
            background: #6c757d; 
            color: white; 
            font-weight: bold; 
            border: none; 
            border-radius: 3px; 
            font-size: 8px; 
            min-height: 24px;
            max-height: 24px;
        } 
        QPushButton:hover { 
            background: #545b62; 
        }
        QPushButton:checked { 
            background: #17a2b8; 
            color: white;
        }
    )";

    markMinBtn = new QPushButton("Min");
    markMinBtn->setCheckable(true);
    markMinBtn->setStyleSheet(markBtnStyle);

    markMidBtn = new QPushButton("Mid");
    markMidBtn->setCheckable(true);
    markMidBtn->setStyleSheet(markBtnStyle);

    markMaxBtn = new QPushButton("Max");
    markMaxBtn->setCheckable(true);
    markMaxBtn->setStyleSheet(markBtnStyle);

    markRow->addWidget(markMinBtn);
    markRow->addWidget(markMidBtn);
    markRow->addWidget(markMaxBtn);
    base->addLayout(markRow);

    setLayout(base);
}

void AxisControlWidget::setPosition(double pos)
{
    posLabel->setText(QString::asprintf("%.2f mm", pos));
    goSpin->setValue(pos);
}

void AxisControlWidget::setEnabledAll(bool enabled)
{
    homeBtn->setEnabled(enabled);
    moveMinusBtn->setEnabled(enabled);
    movePlusBtn->setEnabled(enabled);
    goBtn->setEnabled(enabled);
    stepSpin->setEnabled(enabled);
    goSpin->setEnabled(enabled);
    markMinBtn->setEnabled(enabled);
    markMidBtn->setEnabled(enabled);
    markMaxBtn->setEnabled(enabled);
}

// --- MotorControlWidget Implementation ---

MotorControlWidget::MotorControlWidget(QWidget *parent)
    : QWidget(parent),
      serial(new QSerialPort(this)),
      pollTimer(new QTimer(this)),
      connected(false)
{
    setupUI();
    refreshPorts();

    // Connect signals
    connect(refreshBtn, &QPushButton::clicked, this, &MotorControlWidget::refreshPorts);
    connect(connectBtn, &QPushButton::clicked, this, &MotorControlWidget::connectPort);
    connect(disconnectBtn, &QPushButton::clicked, this, &MotorControlWidget::disconnectPort);
    connect(estopBtn, &QPushButton::clicked, this, &MotorControlWidget::emergencyStop);
    connect(sendCommandBtn, &QPushButton::clicked, [this]()
            {
        sendCustomCommand(commandInput->text());
        commandInput->clear(); });
    connect(commandInput, &QLineEdit::returnPressed, this, &MotorControlWidget::onCommandInputReturnPressed);

    connect(serial, &QSerialPort::readyRead, this, &MotorControlWidget::handleSerialReadyRead);
    connect(pollTimer, &QTimer::timeout, this, &MotorControlWidget::updatePositionPoll);
}

MotorControlWidget::~MotorControlWidget()
{
    if (serial->isOpen())
    {
        serial->close();
    }
}

void MotorControlWidget::setupUI()
{
    setWindowTitle("Motor Control - Advanced");
    setMinimumSize(1100, 750);
    resize(1200, 800);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    // Connection section
    QGroupBox *connectionGroup = new QGroupBox("Connection");
    connectionGroup->setMaximumHeight(70);
    QHBoxLayout *connLayout = new QHBoxLayout(connectionGroup);
    connLayout->setSpacing(6);

    QLabel *portLabel = new QLabel("Port:");
    portCombo = new QComboBox();
    portCombo->setMinimumWidth(150);

    QLabel *baudLabel = new QLabel("Baud:");
    QComboBox *baudCombo = new QComboBox();
    baudCombo->addItems({"9600", "19200", "38400", "57600", "115200"});
    baudCombo->setCurrentText("115200");
    baudCombo->setMinimumWidth(80);

    refreshBtn = new QPushButton("Refresh");
    refreshBtn->setFixedWidth(80);
    refreshBtn->setStyleSheet("QPushButton { background: #607D8B; color: white; font-weight: bold; border-radius: 5px; padding: 6px; } QPushButton:hover { background: #455A64; }");

    connectBtn = new QPushButton("Connect");
    connectBtn->setFixedWidth(80);
    connectBtn->setStyleSheet("QPushButton { background: #4CAF50; color: white; font-weight: bold; border-radius: 5px; padding: 6px; } QPushButton:hover { background: #45a049; }");

    disconnectBtn = new QPushButton("Disconnect");
    disconnectBtn->setFixedWidth(80);
    disconnectBtn->setStyleSheet("QPushButton { background: #f44336; color: white; font-weight: bold; border-radius: 5px; padding: 6px; } QPushButton:hover { background: #d32f2f; }");
    disconnectBtn->setEnabled(false);

    statusLabel = new QLabel("Disconnected");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("QLabel { color: #f44336; font-weight: bold; padding: 6px; border: 2px solid #f44336; border-radius: 5px; }");

    connLayout->addWidget(portLabel);
    connLayout->addWidget(portCombo);
    connLayout->addWidget(baudLabel);
    connLayout->addWidget(baudCombo);
    connLayout->addWidget(refreshBtn);
    connLayout->addWidget(connectBtn);
    connLayout->addWidget(disconnectBtn);
    connLayout->addStretch();
    connLayout->addWidget(statusLabel);

    mainLayout->addWidget(connectionGroup);

    // Main splitter for resizable panels
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal);
    mainSplitter->setChildrenCollapsible(false);

    // Left panel - Controls
    QWidget *leftPanel = new QWidget();
    leftPanel->setMinimumWidth(550);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(10);

    // Create horizontal layout for directional and axis controls side by side
    QHBoxLayout *controlsHLayout = new QHBoxLayout();
    controlsHLayout->setSpacing(10);

    // === FULL 3x3 DIRECTIONAL GRID ===
    QGroupBox *directionalGroup = new QGroupBox("Directional Control");
    directionalGroup->setFixedSize(300, 240);
    QVBoxLayout *dirMainLayout = new QVBoxLayout(directionalGroup);
    dirMainLayout->setSpacing(6);

    // 3x3 Grid for all directional buttons
    QGridLayout *dirLayout = new QGridLayout();
    dirLayout->setSpacing(3);

    QSize buttonSize(50, 35);

    // Row 0 - NW, N (Y+), NE
    QPushButton *nwBtn = new QPushButton("NW");
    nwBtn->setFixedSize(buttonSize);
    nwBtn->setStyleSheet("QPushButton { background: #9C27B0; color: white; font-weight: bold; border-radius: 5px; font-size: 10px; } QPushButton:hover { opacity: 0.8; }");
    dirLayout->addWidget(nwBtn, 0, 0);

    QPushButton *northBtn = new QPushButton("N");
    northBtn->setFixedSize(buttonSize);
    northBtn->setStyleSheet("QPushButton { background: #4CAF50; color: white; font-weight: bold; border-radius: 5px; font-size: 10px; } QPushButton:hover { opacity: 0.8; }");
    dirLayout->addWidget(northBtn, 0, 1);

    QPushButton *neBtn = new QPushButton("NE");
    neBtn->setFixedSize(buttonSize);
    neBtn->setStyleSheet("QPushButton { background: #9C27B0; color: white; font-weight: bold; border-radius: 5px; font-size: 10px; } QPushButton:hover { opacity: 0.8; }");
    dirLayout->addWidget(neBtn, 0, 2);

    // Row 1 - W (X-), HOME, E (X+)
    QPushButton *westBtn = new QPushButton("W");
    westBtn->setFixedSize(buttonSize);
    westBtn->setStyleSheet("QPushButton { background: #2196F3; color: white; font-weight: bold; border-radius: 5px; font-size: 10px; } QPushButton:hover { opacity: 0.8; }");
    dirLayout->addWidget(westBtn, 1, 0);

    QPushButton *homeBtn = new QPushButton("HOME");
    homeBtn->setFixedSize(buttonSize);
    homeBtn->setStyleSheet("QPushButton { background: #FF5722; color: white; font-weight: bold; border-radius: 5px; font-size: 9px; } QPushButton:hover { background: #E64A19; }");
    dirLayout->addWidget(homeBtn, 1, 1);

    QPushButton *eastBtn = new QPushButton("E");
    eastBtn->setFixedSize(buttonSize);
    eastBtn->setStyleSheet("QPushButton { background: #2196F3; color: white; font-weight: bold; border-radius: 5px; font-size: 10px; } QPushButton:hover { opacity: 0.8; }");
    dirLayout->addWidget(eastBtn, 1, 2);

    // Row 2 - SW, S (Y-), SE
    QPushButton *swBtn = new QPushButton("SW");
    swBtn->setFixedSize(buttonSize);
    swBtn->setStyleSheet("QPushButton { background: #9C27B0; color: white; font-weight: bold; border-radius: 5px; font-size: 10px; } QPushButton:hover { opacity: 0.8; }");
    dirLayout->addWidget(swBtn, 2, 0);

    QPushButton *southBtn = new QPushButton("S");
    southBtn->setFixedSize(buttonSize);
    southBtn->setStyleSheet("QPushButton { background: #4CAF50; color: white; font-weight: bold; border-radius: 5px; font-size: 10px; } QPushButton:hover { opacity: 0.8; }");
    dirLayout->addWidget(southBtn, 2, 1);

    QPushButton *seBtn = new QPushButton("SE");
    seBtn->setFixedSize(buttonSize);
    seBtn->setStyleSheet("QPushButton { background: #9C27B0; color: white; font-weight: bold; border-radius: 5px; font-size: 10px; } QPushButton:hover { opacity: 0.8; }");
    dirLayout->addWidget(seBtn, 2, 2);

    dirMainLayout->addLayout(dirLayout);

    // Z controls below directional grid
    QHBoxLayout *zLayout = new QHBoxLayout();
    QPushButton *zUpBtn = new QPushButton("Z+");
    zUpBtn->setFixedSize(60, 30);
    zUpBtn->setStyleSheet("QPushButton { background: #FF9800; color: white; font-weight: bold; border-radius: 5px; font-size: 10px; } QPushButton:hover { opacity: 0.8; }");

    QPushButton *zDownBtn = new QPushButton("Z-");
    zDownBtn->setFixedSize(60, 30);
    zDownBtn->setStyleSheet("QPushButton { background: #FF9800; color: white; font-weight: bold; border-radius: 5px; font-size: 10px; } QPushButton:hover { opacity: 0.8; }");

    zLayout->addStretch();
    zLayout->addWidget(zUpBtn);
    zLayout->addWidget(zDownBtn);
    zLayout->addStretch();

    dirMainLayout->addLayout(zLayout);

    // Step size control
    QHBoxLayout *stepLayout = new QHBoxLayout();
    QLabel *stepLabel = new QLabel("Step:");
    QDoubleSpinBox *stepSpinBox = new QDoubleSpinBox();
    stepSpinBox->setRange(0.1, 100.0);
    stepSpinBox->setValue(1.0);
    stepSpinBox->setSuffix(" mm");
    stepSpinBox->setDecimals(1);
    stepSpinBox->setFixedWidth(80);

    stepLayout->addWidget(stepLabel);
    stepLayout->addWidget(stepSpinBox);
    stepLayout->addStretch();

    dirMainLayout->addLayout(stepLayout);

    controlsHLayout->addWidget(directionalGroup);

    QGroupBox *axisGroup = new QGroupBox("Individual Axis Control");
    axisGroup->setMinimumWidth(230);
    QVBoxLayout *axisLayout = new QVBoxLayout(axisGroup);
    axisLayout->setSpacing(4);

    axisControls.clear();
    QStringList axes = {"X", "Y", "Z"};
    for (const QString &axis : axes)
    {
        AxisControlWidget *axisWidget = new AxisControlWidget(axis);
        axisWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        axisControls.push_back(axisWidget);
        axisLayout->addWidget(axisWidget);

        // Connect axis control signals
        connect(axisWidget->homeBtn, &QPushButton::clicked, this, &MotorControlWidget::axisHome);
        connect(axisWidget->moveMinusBtn, &QPushButton::clicked, this, &MotorControlWidget::axisMoveStep);
        connect(axisWidget->movePlusBtn, &QPushButton::clicked, this, &MotorControlWidget::axisMoveStep);
        connect(axisWidget->goBtn, &QPushButton::clicked, this, &MotorControlWidget::axisGoTo);
        connect(axisWidget->markMinBtn, &QPushButton::clicked, this, &MotorControlWidget::markPosition);
        connect(axisWidget->markMidBtn, &QPushButton::clicked, this, &MotorControlWidget::markPosition);
        connect(axisWidget->markMaxBtn, &QPushButton::clicked, this, &MotorControlWidget::markPosition);
    }

    controlsHLayout->addWidget(axisGroup);
    leftLayout->addLayout(controlsHLayout);

    // Emergency Controls
    QGroupBox *emergencyGroup = new QGroupBox("Emergency Controls");
    emergencyGroup->setMaximumHeight(80);
    QHBoxLayout *emergencyLayout = new QHBoxLayout(emergencyGroup);
    emergencyLayout->setSpacing(8);

    estopBtn = new QPushButton("EMERGENCY STOP");
    estopBtn->setStyleSheet("QPushButton { background: #f44336; color: white; font-weight: bold; font-size: 12px; border-radius: 5px; padding: 8px; } QPushButton:hover { background: #d32f2f; }");
    estopBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QPushButton *homeAllBtn = new QPushButton("Home All");
    homeAllBtn->setStyleSheet("QPushButton { background: #9C27B0; color: white; font-weight: bold; border-radius: 5px; padding: 8px; } QPushButton:hover { background: #7B1FA2; }");
    homeAllBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    emergencyLayout->addWidget(estopBtn, 2);
    emergencyLayout->addWidget(homeAllBtn, 1);

    leftLayout->addWidget(emergencyGroup);
    leftLayout->addStretch();

    mainSplitter->addWidget(leftPanel);

    // Right panel - Commands and Status
    QWidget *rightPanel = new QWidget();
    rightPanel->setMinimumWidth(400);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(8);

    // Direct Commands
    QGroupBox *commandGroup = new QGroupBox("Direct Commands");
    commandGroup->setMaximumHeight(80);
    QVBoxLayout *cmdLayout = new QVBoxLayout(commandGroup);
    cmdLayout->setSpacing(4);

    QHBoxLayout *cmdInputLayout = new QHBoxLayout();
    commandInput = new QLineEdit();
    commandInput->setPlaceholderText("Enter any command to send to device");
    commandInput->setStyleSheet("QLineEdit { border: 2px solid #ddd; border-radius: 5px; padding: 6px; font-family: monospace; }");

    sendCommandBtn = new QPushButton("Send");
    sendCommandBtn->setFixedWidth(80);
    sendCommandBtn->setStyleSheet("QPushButton { background: #2196F3; color: white; font-weight: bold; border-radius: 5px; padding: 6px; } QPushButton:hover { background: #1976D2; }");

    cmdInputLayout->addWidget(commandInput);
    cmdInputLayout->addWidget(sendCommandBtn);
    cmdLayout->addLayout(cmdInputLayout);

    rightLayout->addWidget(commandGroup);

    // Status Log
    QGroupBox *statusGroup = new QGroupBox("Status Log & Serial Monitor");
    QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);
    statusLayout->setSpacing(4);

    statusLog = new QTextEdit();
    statusLog->setReadOnly(true);
    statusLog->setStyleSheet("QTextEdit { background: #f9f9f9; border: 2px solid #ddd; border-radius: 5px; font-family: monospace; font-size: 11px; color: black; }");
    statusLog->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QHBoxLayout *logButtonLayout = new QHBoxLayout();
    QPushButton *clearBtn = new QPushButton("Clear");
    clearBtn->setFixedWidth(80);
    clearBtn->setStyleSheet("QPushButton { background: #757575; color: white; font-weight: bold; border-radius: 5px; padding: 4px; } QPushButton:hover { background: #616161; }");

    logButtonLayout->addStretch();
    logButtonLayout->addWidget(clearBtn);

    statusLayout->addWidget(statusLog);
    statusLayout->addLayout(logButtonLayout);

    rightLayout->addWidget(statusGroup);

    mainSplitter->addWidget(rightPanel);

    // Set initial splitter proportions
    mainSplitter->setSizes({650, 450});
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 1);

    mainLayout->addWidget(mainSplitter);

    // Connect all button signals
    connect(refreshBtn, &QPushButton::clicked, this, &MotorControlWidget::refreshPorts);
    connect(connectBtn, &QPushButton::clicked, this, &MotorControlWidget::connectPort);
    connect(disconnectBtn, &QPushButton::clicked, this, &MotorControlWidget::disconnectPort);
    connect(estopBtn, &QPushButton::clicked, this, &MotorControlWidget::emergencyStop);
    connect(sendCommandBtn, &QPushButton::clicked, [this]()
            {
        QString cmd = commandInput->text();
        if (!cmd.trimmed().isEmpty()) {
            sendCustomCommand(cmd);
            commandInput->clear();
        } });
    connect(commandInput, &QLineEdit::returnPressed, [this]()
            {
        QString cmd = commandInput->text();
        if (!cmd.trimmed().isEmpty()) {
            sendCustomCommand(cmd);
            commandInput->clear();
        } });
    connect(clearBtn, &QPushButton::clicked, statusLog, &QTextEdit::clear);
    connect(homeAllBtn, &QPushButton::clicked, [this]()
            { sendCustomCommand("G28"); });

    // === DIRECTIONAL BUTTON CONNECTIONS (with proper X/Z axis reversal) ===
    // Basic directions
    connect(northBtn, &QPushButton::clicked, [this, stepSpinBox]()
            { sendCustomCommand(QString("G91\nG1 Y%1 F1000\nG90").arg(stepSpinBox->value())); });
    connect(southBtn, &QPushButton::clicked, [this, stepSpinBox]()
            { sendCustomCommand(QString("G91\nG1 Y%1 F1000\nG90").arg(-stepSpinBox->value())); });
    connect(eastBtn, &QPushButton::clicked, [this, stepSpinBox]()
            { sendCustomCommand(QString("G91\nG1 X%1 F1000\nG90").arg(-stepSpinBox->value())); }); // X reversed
    connect(westBtn, &QPushButton::clicked, [this, stepSpinBox]()
            { sendCustomCommand(QString("G91\nG1 X%1 F1000\nG90").arg(stepSpinBox->value())); }); // X reversed

    // Diagonal movements
    connect(neBtn, &QPushButton::clicked, [this, stepSpinBox]()
            { sendCustomCommand(QString("G91\nG1 X%1 Y%2 F1000\nG90").arg(-stepSpinBox->value()).arg(stepSpinBox->value())); });
    connect(nwBtn, &QPushButton::clicked, [this, stepSpinBox]()
            { sendCustomCommand(QString("G91\nG1 X%1 Y%2 F1000\nG90").arg(stepSpinBox->value()).arg(stepSpinBox->value())); });
    connect(seBtn, &QPushButton::clicked, [this, stepSpinBox]()
            { sendCustomCommand(QString("G91\nG1 X%1 Y%2 F1000\nG90").arg(-stepSpinBox->value()).arg(-stepSpinBox->value())); });
    connect(swBtn, &QPushButton::clicked, [this, stepSpinBox]()
            { sendCustomCommand(QString("G91\nG1 X%1 Y%2 F1000\nG90").arg(stepSpinBox->value()).arg(-stepSpinBox->value())); });

    // Z controls with reversal
    connect(zUpBtn, &QPushButton::clicked, [this, stepSpinBox]()
            { sendCustomCommand(QString("G91\nG1 Z%1 F1000\nG90").arg(-stepSpinBox->value())); }); // Z reversed
    connect(zDownBtn, &QPushButton::clicked, [this, stepSpinBox]()
            { sendCustomCommand(QString("G91\nG1 Z%1 F1000\nG90").arg(stepSpinBox->value())); }); // Z reversed

    // HOME button
    connect(homeBtn, &QPushButton::clicked, [this]()
            { sendCustomCommand("G28"); });

    // Initialize ports and status
    refreshPorts();
    updateStatus("Motor control widget initialized - Ready for connection");
}

bool MotorControlWidget::isConnected() const
{
    return connected && serial->isOpen();
}

void MotorControlWidget::showWidget()
{
    show();
    raise();
    activateWindow();
}

void MotorControlWidget::hideWidget()
{
    hide();
}

void MotorControlWidget::connectToPort(const QString &portName)
{
    if (!portName.isEmpty())
    {
        // Find and select the specified port
        for (int i = 0; i < portCombo->count(); ++i)
        {
            if (portCombo->itemText(i).contains(portName))
            {
                portCombo->setCurrentIndex(i);
                break;
            }
        }
    }
    connectPort();
}

void MotorControlWidget::disconnectFromPort()
{
    disconnectPort();
}

void MotorControlWidget::sendCustomCommand(const QString &command)
{
    if (!isConnected())
    {
        updateStatus("Error: Not connected to serial port");
        emit errorOccurred("Not connected to serial port");
        return;
    }

    QString trimmedCmd = command.trimmed();
    if (trimmedCmd.isEmpty())
    {
        updateStatus("Error: Empty command");
        return;
    }

    QString cmd = trimmedCmd;
    if (!cmd.endsWith('\n'))
    {
        cmd += '\n';
    }

    serial->write(cmd.toUtf8());

    // Add sent command to status log with timestamp and color
    QString timestamp = QTime::currentTime().toString("hh:mm:ss");
    statusLog->append(QString("[%1] <span style='color: orange;'>TX: %2</span>").arg(timestamp, trimmedCmd));

    updateStatus("Sent: " + trimmedCmd);
    emit commandExecuted(command.trimmed(), ""); // Response will be handled in serial read
}

void MotorControlWidget::refreshPorts()
{
    portCombo->clear();
    const QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports)
    {
        portCombo->addItem(QString("%1 - %2").arg(info.portName(), info.description()));
    }

    if (portCombo->count() == 0)
    {
        portCombo->addItem("No ports available");
    }
}

void MotorControlWidget::connectPort()
{
    QString portText = portCombo->currentText();
    if (portText == "No ports available" || portText.isEmpty())
    {
        QMessageBox::warning(this, "Connection Error", "No serial port selected");
        return;
    }

    QString portName = portText.split(" ").first();

    if (serial->isOpen())
    {
        serial->close();
    }

    serial->setPortName(portName);
    serial->setBaudRate(115200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!serial->open(QIODevice::ReadWrite))
    {
        QString error = QString("Failed to open port %1: %2").arg(portName, serial->errorString());
        QMessageBox::critical(this, "Connection Error", error);
        updateStatus("Connection failed: " + error);
        emit errorOccurred(error);
        return;
    }

    connected = true;
    updateStatus("✅ Connected to " + portName);
    statusLabel->setText("Connected");
    statusLabel->setStyleSheet("font-weight: bold; color: #28a745; font-size: 12px; padding: 8px; background: #d4edda; border-radius: 4px; border: 1px solid #c3e6cb;");

    connectBtn->setEnabled(false);
    disconnectBtn->setEnabled(true);

    for (auto *aw : axisControls)
    {
        aw->setEnabledAll(true);
    }

    pollTimer->start(750); // Poll position every 750ms
    emit connectionStatusChanged(true);
}

void MotorControlWidget::disconnectPort()
{
    if (serial->isOpen())
    {
        serial->close();
    }

    connected = false;
    pollTimer->stop();

    updateStatus("❌ Disconnected");
    statusLabel->setText("Disconnected");
    statusLabel->setStyleSheet("font-weight: bold; color: #dc3545; font-size: 12px; padding: 8px; background: #f8d7da; border-radius: 4px; border: 1px solid #f5c6cb;");

    connectBtn->setEnabled(true);
    disconnectBtn->setEnabled(false);

    for (auto *aw : axisControls)
    {
        aw->setEnabledAll(false);
    }

    emit connectionStatusChanged(false);
}

void MotorControlWidget::updateStatus(const QString &message)
{
    QString timestamp = QTime::currentTime().toString("hh:mm:ss");
    statusLog->append(QString("[%1] %2").arg(timestamp, message));
}

void MotorControlWidget::directionalClicked()
{
    if (!isConnected())
    {
        updateStatus("Error: Not connected");
        return;
    }

    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    if (!btn)
        return;

    QString dir = btn->text();
    double step = 10.0;

    if (dir == "Home")
    {
        sendCustomCommand("G28");
        return;
    }

    double dx = 0, dy = 0;
    if (dir.contains("N"))
        dy += step;
    if (dir.contains("S"))
        dy -= step;
    // Reverse X axis direction for correct movement
    if (dir.contains("E"))
        dx -= step; // East moves negative X
    if (dir.contains("W"))
        dx += step; // West moves positive X

    QString command = QString("G1 X%1 Y%2 F3000").arg(dx).arg(dy);
    sendCustomCommand(command);
}

void MotorControlWidget::axisHome()
{
    AxisControlWidget *aw = qobject_cast<AxisControlWidget *>(sender()->parent());
    if (!aw)
        return;

    QString command = QString("G28 %1").arg(aw->axisName.toUpper());
    sendCustomCommand(command);
}

void MotorControlWidget::axisMoveStep()
{
    AxisControlWidget *aw = qobject_cast<AxisControlWidget *>(sender()->parent());
    if (!aw)
        return;

    bool minus = (sender() == aw->moveMinusBtn);
    double step = aw->stepSpin->value();
    double curr = aw->goSpin->value();

    // Reverse direction for X and Z axes (invert the button behavior)
    if (aw->axisName.toLower() == "x" || aw->axisName.toLower() == "z")
    {
        minus = !minus; // Invert the direction for X and Z axes
    }

    double pos = curr + (minus ? -step : step);
    QString command = QString("G1 %1%2 F3000").arg(aw->axisName.toUpper()).arg(pos);
    sendCustomCommand(command);
}

void MotorControlWidget::axisGoTo()
{
    AxisControlWidget *aw = qobject_cast<AxisControlWidget *>(sender()->parent());
    if (!aw)
        return;

    double pos = aw->goSpin->value();
    QString command = QString("G1 %1%2 F3000").arg(aw->axisName.toUpper()).arg(pos);
    sendCustomCommand(command);
}

void MotorControlWidget::markPosition()
{
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    AxisControlWidget *aw = qobject_cast<AxisControlWidget *>(btn ? btn->parent() : nullptr);

    if (!aw || !btn)
        return;

    QString type = (btn == aw->markMinBtn)   ? "min"
                   : (btn == aw->markMidBtn) ? "mid"
                   : (btn == aw->markMaxBtn) ? "max"
                                             : "";

    if (type.isEmpty())
        return;

    // Get current position for the axis
    double val = 0.0;
    if (aw->axisName == "x")
        val = lastPosX;
    else if (aw->axisName == "y")
        val = lastPosY;
    else if (aw->axisName == "z")
        val = lastPosZ;

    AxisMeasurement *m = measurement(aw->axisName);
    if (!m)
        return;

    if (type == "min")
        m->min = val;
    else if (type == "mid")
        m->mid = val;
    else if (type == "max")
        m->max = val;

    // Update button states
    aw->markMinBtn->setChecked(type == "min");
    aw->markMidBtn->setChecked(type == "mid");
    aw->markMaxBtn->setChecked(type == "max");

    updateStatus(QString("Marked %1 %2 position: %3 mm").arg(aw->axisName.toUpper(), type).arg(val, 0, 'f', 2));
}

void MotorControlWidget::emergencyStop()
{
    if (isConnected())
    {
        sendCustomCommand("M112"); // Emergency stop G-code
        updateStatus("EMERGENCY STOP ACTIVATED");
    }
}

void MotorControlWidget::updatePositionPoll()
{
    if (isConnected())
    {
        sendCustomCommand("M114"); // Request position
    }
}

void MotorControlWidget::handleSerialReadyRead()
{
    handleSerialRead();
}

void MotorControlWidget::onCommandInputReturnPressed()
{
    sendCustomCommand(commandInput->text());
    commandInput->clear();
}

AxisMeasurement *MotorControlWidget::measurement(const QString &axis)
{
    QString ax = axis.toLower();
    if (ax == "x")
        return &measX;
    else if (ax == "y")
        return &measY;
    else if (ax == "z")
        return &measZ;
    return nullptr;
}

void MotorControlWidget::handleSerialRead()
{
    QByteArray newData = serial->readAll();
    buffer.append(newData);

    // Process complete lines
    while (buffer.contains('\n'))
    {
        int lineEnd = buffer.indexOf('\n');
        QByteArray lineData = buffer.left(lineEnd);
        buffer.remove(0, lineEnd + 1);

        QString line = QString::fromUtf8(lineData).trimmed();
        if (line.isEmpty())
            continue;

        QString timestamp = QTime::currentTime().toString("hh:mm:ss");

        // Color code different types of responses
        QString displayLine;
        if (line.startsWith("ok") || line.contains("OK"))
        {
            displayLine = QString("[%1] <span style='color: green;'>RX: %2</span>").arg(timestamp, line);
        }
        else if (line.startsWith("error") || line.startsWith("Error") || line.contains("error") || line.contains("Error"))
        {
            displayLine = QString("[%1] <span style='color: red;'>RX: %2</span>").arg(timestamp, line);
        }
        else if (line.startsWith("//") || line.startsWith(";"))
        {
            displayLine = QString("[%1] <span style='color: gray;'>RX: %2</span>").arg(timestamp, line);
        }
        else
        {
            displayLine = QString("[%1] <span style='color: blue;'>RX: %2</span>").arg(timestamp, line);
        }

        statusLog->append(displayLine);

        // Parse position updates (M114 response)
        if (line.contains("X:") && line.contains("Y:") && line.contains("Z:"))
        {
            QRegularExpression rx(R"(X:([-+]?\d*\.?\d+)\s+Y:([-+]?\d*\.?\d+)\s+Z:([-+]?\d*\.?\d+))");
            QRegularExpressionMatch match = rx.match(line);
            if (match.hasMatch())
            {
                lastPosX = match.captured(1).toDouble();
                lastPosY = match.captured(2).toDouble();
                lastPosZ = match.captured(3).toDouble();

                // Update axis control widgets
                for (auto *aw : axisControls)
                {
                    if (aw->axisName == "x")
                    {
                        aw->setPosition(lastPosX);
                        emit positionChanged("X", lastPosX);
                    }
                    else if (aw->axisName == "y")
                    {
                        aw->setPosition(lastPosY);
                        emit positionChanged("Y", lastPosY);
                    }
                    else if (aw->axisName == "z")
                    {
                        aw->setPosition(lastPosZ);
                        emit positionChanged("Z", lastPosZ);
                    }
                }
            }
        }
    }
}
