#include "mainwindow.h"
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QtMath>
#include <QFileDialog>
#include <QTime>

// --- AxisControlWidget ---

AxisControlWidget::AxisControlWidget(const QString &axis, QWidget *parent)
    : QGroupBox(axis.toUpper() + " Axis", parent), axisName(axis)
{
    setStyleSheet("QGroupBox { border:1.5px solid #92dafc; border-radius:8px; margin-top:10px; background:#fafdff; }");
    QVBoxLayout *base = new QVBoxLayout;

    posLabel = new QLabel("0.00 mm");
    posLabel->setAlignment(Qt::AlignCenter);
    posLabel->setStyleSheet("font-weight:bold; font-size:15px; color:#147;");
    base->addWidget(posLabel);

    QHBoxLayout *stepRow = new QHBoxLayout;
    stepRow->setSpacing(10);
    homeBtn = new QPushButton("Home");
    moveMinusBtn = new QPushButton("-");
    stepSpin = new QDoubleSpinBox;
    stepSpin->setRange(0.1, 100);
    stepSpin->setValue(axis == "z" ? 5.0 : 10.0);
    stepSpin->setSuffix(" mm");
    movePlusBtn = new QPushButton("+");
    stepRow->addWidget(homeBtn);
    stepRow->addWidget(moveMinusBtn);
    stepRow->addWidget(stepSpin);
    stepRow->addWidget(movePlusBtn);
    base->addLayout(stepRow);

    QHBoxLayout *gotoRow = new QHBoxLayout;
    gotoRow->setSpacing(10);
    goLabel = new QLabel("Goto:");
    goSpin = new QDoubleSpinBox;
    goSpin->setRange(-999, 999);
    goBtn = new QPushButton("Go To");
    gotoRow->addWidget(goLabel);
    gotoRow->addWidget(goSpin);
    gotoRow->addWidget(goBtn);
    base->addLayout(gotoRow);

    QHBoxLayout *markRow = new QHBoxLayout;
    markMinBtn = new QPushButton("Min");
    markMidBtn = new QPushButton("Mid");
    markMaxBtn = new QPushButton("Max");
    markRow->addWidget(markMinBtn);
    markRow->addWidget(markMidBtn);
    markRow->addWidget(markMaxBtn);
    base->addLayout(markRow);

    base->setSpacing(8);
    setLayout(base);
    markMinBtn->setCheckable(true);
    markMidBtn->setCheckable(true);
    markMaxBtn->setCheckable(true);
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

// --- MainWindow ---

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      serial(new QSerialPort(this)),
      pollTimer(new QTimer(this)),
      connected(false)
{
    buildUI();
    refreshPorts();

    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshPorts);
    connect(connectBtn, &QPushButton::clicked, this, &MainWindow::connectPort);
    connect(disconnectBtn, &QPushButton::clicked, this, &MainWindow::disconnectPort);
    connect(estopBtn, &QPushButton::clicked, this, &MainWindow::emergencyStop);

    connect(serial, &QSerialPort::readyRead, this, &MainWindow::handleSerialReadyRead);
    connect(pollTimer, &QTimer::timeout, this, &MainWindow::updatePositionPoll);
}

void MainWindow::buildUI()
{
    setWindowTitle("TinyBee Axis Measurement Tool");
    QWidget *central = new QWidget;
    QVBoxLayout *rootLayout = new QVBoxLayout;
    rootLayout->setSpacing(16);
    rootLayout->setContentsMargins(18, 12, 18, 12);

    // Connection group
    QGroupBox *connGroup = new QGroupBox;
    connGroup->setStyleSheet("QGroupBox { border:0; }");
    QHBoxLayout *connRow = new QHBoxLayout;
    QLabel *portLbl = new QLabel("Port:");
    portCombo = new QComboBox;
    refreshBtn = new QPushButton("Refresh");
    connectBtn = new QPushButton("Connect");
    disconnectBtn = new QPushButton("Disconnect");
    disconnectBtn->setEnabled(false);
    statusLabel = new QLabel("Disconnected");
    statusLabel->setStyleSheet("font-weight: bold;");
    connRow->addWidget(portLbl);
    connRow->addWidget(portCombo, 2);
    connRow->addWidget(refreshBtn);
    connRow->addWidget(connectBtn);
    connRow->addWidget(disconnectBtn);
    connRow->addWidget(statusLabel);
    connGroup->setLayout(connRow);
    rootLayout->addWidget(connGroup);

    // Tabs
    tabs = new QTabWidget;
    // QWidget* measTab = new QWidget; QVBoxLayout* mtab = new QVBoxLayout;
    // // Motor Directional Controls (3x3 grid)
    // QGroupBox* dirGroup = new QGroupBox("Motor Directional Controls");
    // dirGroup->setStyleSheet("QGroupBox { border:1.3px solid #92dafc; border-radius:8px; margin-top:12px; background:#fafdff; font-weight:bold; }");
    // QGridLayout* dirGrid = new QGridLayout;
    // QString dnames[3][3] = { {"NW","N","NE"}, {"W","Home","E"}, {"SW","S","SE"} };
    // for(int i=0;i<3;++i) for(int j=0;j<3;++j) {
    //         QPushButton* btn = new QPushButton(dnames[i][j]);
    //         btn->setMinimumSize(74,32);
    //         btn->setStyleSheet("background:#e9f6fc; font-weight:bold; color:#033; border-radius:4px;");
    //         dirGrid->addWidget(btn,i,j);
    //         connect(btn,&QPushButton::clicked,this,&MainWindow::directionalClicked);
    //     }
    // dirGroup->setLayout(dirGrid);
    // mtab->addWidget(dirGroup);

    QWidget *measTab = new QWidget;
    QVBoxLayout *mtab = new QVBoxLayout;

    // --- Motor Directional Controls section with heading centered ---
    QGroupBox *dirGroup = new QGroupBox(""); // Set title to "" so it's not rendered

    QVBoxLayout *dirVBox = new QVBoxLayout;

    // Create a centered bold heading label
    QLabel *dirHeading = new QLabel("Motor Directional Controls");
    dirHeading->setAlignment(Qt::AlignCenter);
    dirHeading->setStyleSheet("font-weight: bold; font-size: 16px; color: #176583; padding-bottom: 5px;");

    // Create grid of buttons
    QGridLayout *dirGrid = new QGridLayout;
    QString dnames[3][3] = {{"NW", "N", "NE"}, {"W", "Home", "E"}, {"SW", "S", "SE"}};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
        {
            QPushButton *btn = new QPushButton(dnames[i][j]);
            btn->setMinimumSize(74, 32);
            btn->setStyleSheet("background:#e9f6fc; font-weight:bold; color:#033; border-radius:4px;");
            dirGrid->addWidget(btn, i, j);
            connect(btn, &QPushButton::clicked, this, &MainWindow::directionalClicked);
        }

    // Add heading then grid to vertical box
    dirVBox->addWidget(dirHeading);
    dirVBox->addLayout(dirGrid);

    dirGroup->setLayout(dirVBox);
    // Retain outer groupbox look with styles
    dirGroup->setStyleSheet("QGroupBox { border:1.3px solid #92dafc; border-radius:8px; margin-top:12px; background:#fafdff; padding-top:6px; }");

    // Add to tab
    mtab->addWidget(dirGroup);

    // Axis Control Widgets
    axisControls.clear();
    for (QString ax : {"X", "Y", "Z"})
    {
        AxisControlWidget *aw = new AxisControlWidget(ax);
        aw->setStyleSheet("QGroupBox {border:1.2px solid #7aceef;border-radius:7px;}");
        axisControls.push_back(aw);
        mtab->addWidget(aw);
        connect(aw->homeBtn, &QPushButton::clicked, this, &MainWindow::axisHome);
        connect(aw->moveMinusBtn, &QPushButton::clicked, this, &MainWindow::axisMoveStep);
        connect(aw->movePlusBtn, &QPushButton::clicked, this, &MainWindow::axisMoveStep);
        connect(aw->goBtn, &QPushButton::clicked, this, &MainWindow::axisGoTo);
        connect(aw->markMinBtn, &QPushButton::clicked, this, &MainWindow::markPosition);
        connect(aw->markMidBtn, &QPushButton::clicked, this, &MainWindow::markPosition);
        connect(aw->markMaxBtn, &QPushButton::clicked, this, &MainWindow::markPosition);
        aw->setEnabledAll(false);
    }
    measTab->setLayout(mtab);
    tabs->addTab(measTab, "Measurement");

    rootLayout->addWidget(tabs);

    // Bottom row: EMERGENCY + LOG
    QHBoxLayout *botRow = new QHBoxLayout;
    estopBtn = new QPushButton("EMERGENCY STOP");
    estopBtn->setStyleSheet("background:#fd5462; color:#fff; font-weight:bold; font-size:16px; border-radius:8px; padding:9px 18px;");
    statusLog = new QTextEdit;
    statusLog->setReadOnly(true);
    statusLog->setMaximumHeight(70);
    botRow->addWidget(estopBtn, 0);
    botRow->addWidget(statusLog, 2);

    rootLayout->addLayout(botRow);
    central->setLayout(rootLayout);
    setCentralWidget(central);
}

void MainWindow::refreshPorts()
{
    portCombo->clear();
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts())
        portCombo->addItem(info.portName() + " - " + info.description());
}

void MainWindow::connectPort()
{
    QString port = portCombo->currentText().split(" ").first();
    if (port.isEmpty())
    {
        updateStatus("No port selected.");
        return;
    }

    connect(serial, &QSerialPort::readyRead, this, &MainWindow::handleSerialRead);

    if (serial->isOpen())
        serial->close();
    serial->setPortName(port);
    serial->setBaudRate(115200);
    if (!serial->open(QIODevice::ReadWrite))
    {
        updateStatus("Failed to open port " + port + ": " + serial->errorString());
        return;
    }
    updateStatus("Connected to " + port);
    for (auto *aw : axisControls)
        aw->setEnabledAll(true);
    connectBtn->setEnabled(false);
    disconnectBtn->setEnabled(true);
    connected = true;
    statusLabel->setText("Connected");
    pollTimer->start(750);
}

void MainWindow::disconnectPort()
{
    serial->close();
    updateStatus("Disconnected");
    statusLabel->setText("Disconnected");
    for (auto *aw : axisControls)
        aw->setEnabledAll(false);
    connectBtn->setEnabled(true);
    disconnectBtn->setEnabled(false);
    connected = false;
    pollTimer->stop();
}

void MainWindow::updateStatus(const QString &msg)
{
    statusLog->append("[" + QTime::currentTime().toString() + "] " + msg);
}

void MainWindow::directionalClicked()
{
    if (!connected)
    {
        updateStatus("Not connected.");
        return;
    }
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    QString dir = btn->text();
    double step = 10.0;
    double dx = 0, dy = 0;
    if (dir.contains("N"))
        dy += step;
    if (dir.contains("S"))
        dy -= step;
    // Reverse X axis direction: E (East) should move in negative X, W (West) should move in positive X
    if (dir.contains("E"))
        dx -= step; // Reversed for X axis
    if (dir.contains("W"))
        dx += step; // Reversed for X axis
    if (dir == "Home")
    {
        serial->write("G28\n");
        updateStatus("Sent: Home all axes.");
    }
    else
    {
        QString gcode = QString("G1 X%1 Y%2 F3000\n").arg(dx).arg(dy);
        serial->write(gcode.toUtf8());
        updateStatus("Sent: " + gcode.trimmed());
    }
}

void MainWindow::axisHome()
{
    AxisControlWidget *aw = qobject_cast<AxisControlWidget *>(sender()->parent());
    QString ax = aw->axisName;
    QString cmd = QString("G28 %1\n").arg(ax.toUpper());
    serial->write(cmd.toUtf8());
    updateStatus("Sent: " + cmd.trimmed());
}

void MainWindow::axisMoveStep()
{
    AxisControlWidget *aw = qobject_cast<AxisControlWidget *>(sender()->parent());
    bool minus = (sender() == aw->moveMinusBtn);
    double step = aw->stepSpin->value(), curr = aw->goSpin->value();

    // Reverse direction for X and Z axes (invert the button behavior)
    if (aw->axisName.toLower() == "x" || aw->axisName.toLower() == "z")
    {
        minus = !minus; // Invert the direction for X and Z axes
    }

    double pos = curr + (minus ? -step : step);
    QString cmd = QString("G1 %1%2 F3000\n").arg(aw->axisName.toUpper()).arg(pos);
    serial->write(cmd.toUtf8());
    updateStatus("Sent: " + cmd.trimmed());
}

void MainWindow::axisGoTo()
{
    AxisControlWidget *aw = qobject_cast<AxisControlWidget *>(sender()->parent());
    double pos = aw->goSpin->value();
    QString cmd = QString("G1 %1%2 F3000\n").arg(aw->axisName.toUpper()).arg(pos);
    serial->write(cmd.toUtf8());
    updateStatus("Sent: " + cmd.trimmed());
}

void MainWindow::markPosition()
{
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    AxisControlWidget *aw = qobject_cast<AxisControlWidget *>(btn ? btn->parent() : nullptr);
    if (!aw || !btn)
        return;
    QString type = (btn == aw->markMinBtn)   ? "min"
                   : (btn == aw->markMidBtn) ? "mid"
                   : (btn == aw->markMaxBtn) ? "max"
                                             : "";

    // Assume you store the last polled position per axis
    double val = 0.0;
    if (aw->axisName == "x")
        val = lastPosX;
    else if (aw->axisName == "y")
        val = lastPosY;
    else
        val = lastPosZ;

    AxisMeasurement *m = measurement(aw->axisName);
    if (type == "min")
        m->min = val;
    else if (type == "mid")
        m->mid = val;
    else if (type == "max")
        m->max = val;

    // Button highlight: you may want to visually show mark
    aw->markMinBtn->setChecked(type == "min");
    aw->markMidBtn->setChecked(type == "mid");
    aw->markMaxBtn->setChecked(type == "max");

    // Optionally: update summary or log
    updateStatus(QString("Marked %1 axis %2: %3 mm").arg(aw->axisName.toUpper(), type, QString::number(val, 'f', 2)));
}

void MainWindow::emergencyStop()
{
    serial->write("M112\n");
    updateStatus("Sent: EMERGENCY STOP");
}

void MainWindow::updatePositionPoll()
{
    if (serial->isOpen())
        serial->write("M114\n"); // ask for coords
}

void MainWindow::handleSerialReadyRead()
{
    buffer += serial->readAll();
    // Parse responses, update UI/axis positions as needed
}

AxisMeasurement *MainWindow::measurement(const QString &axis)
{
    if (axis == "x" || axis == "X")
        return &measX;
    if (axis == "y" || axis == "Y")
        return &measY;
    return &measZ;
}

void MainWindow::handleSerialRead()
{
    buffer.append(serial->readAll());

    while (true)
    {
        int newlineIndex = buffer.indexOf('\n');
        if (newlineIndex == -1)
            break; // No complete line yet

        QByteArray lineData = buffer.left(newlineIndex + 1);
        buffer.remove(0, newlineIndex + 1);

        QString line = QString::fromUtf8(lineData).trimmed();
        if (line.isEmpty())
            continue; // Skip empty lines

        QString timestamp = QTime::currentTime().toString("HH:mm:ss");
        statusLog->append(QString("[%1] %2").arg(timestamp, line));
    }
}
