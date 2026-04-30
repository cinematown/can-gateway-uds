#include "mainwindow.h"

#include <QAbstractSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QSlider>
#include <QSpinBox>
#include <QStatusBar>
#include <QStyle>
#include <QVBoxLayout>
#include <QWidget>

namespace {

QProgressBar *makeProgressBar(int maximum, const QString &format)
{
    auto *bar = new QProgressBar;
    bar->setRange(0, maximum);
    bar->setFormat(format);
    bar->setTextVisible(true);
    bar->setMinimumHeight(20);
    return bar;
}

QSlider *makePercentSlider()
{
    auto *slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 100);
    slider->setSingleStep(1);
    slider->setPageStep(5);
    slider->setTickInterval(10);
    slider->setTickPosition(QSlider::TicksBelow);
    slider->setMinimumWidth(220);
    return slider;
}

QSpinBox *makePercentSpinBox()
{
    auto *spinBox = new QSpinBox;
    spinBox->setRange(0, 100);
    spinBox->setSuffix(QStringLiteral(" %"));
    spinBox->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    spinBox->setMinimumWidth(82);
    return spinBox;
}

int statusValue(const QHash<QString, QString> &values,
                const QString &key,
                bool *ok)
{
    const QString raw = values.value(key);
    return raw.toInt(ok);
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      serial_(new SerialConnection)
{
    setupUi();

    serialPollTimer_.setInterval(20);
    pedalDebounce_.setInterval(80);
    pedalDebounce_.setSingleShot(true);

    connect(&serialPollTimer_, &QTimer::timeout, this, &MainWindow::readSerial);
    connect(&pedalDebounce_, &QTimer::timeout, this, &MainWindow::sendPedal);

    refreshPorts();
}

MainWindow::~MainWindow()
{
    disconnectSerial();
    delete serial_;
    serial_ = nullptr;
}

void MainWindow::setupUi()
{
    setWindowTitle(QStringLiteral("EngineSim Control Panel"));
    resize(1080, 720);

    auto *central = new QWidget(this);
    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(10);
    setCentralWidget(central);

    setStyleSheet(QStringLiteral(
        "QGroupBox { font-weight: 600; }"
        "QGroupBox QLabel, QGroupBox QPushButton, QGroupBox QRadioButton, "
        "QGroupBox QCheckBox, QGroupBox QComboBox, QGroupBox QSpinBox { font-weight: 400; }"
        "QPushButton { min-height: 28px; }"
        "QComboBox, QSpinBox, QLineEdit { min-height: 26px; }"
        "QPlainTextEdit { font-family: Consolas, Courier New, monospace; }"
    ));

    auto *serialGroup = new QGroupBox(QStringLiteral("Serial"), central);
    auto *serialLayout = new QGridLayout(serialGroup);

    portCombo_ = new QComboBox(serialGroup);
    portCombo_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    baudCombo_ = new QComboBox(serialGroup);
    const QList<int> baudRates = {9600, 57600, 115200, 230400, 460800, 921600};
    for (int baud : baudRates) {
        baudCombo_->addItem(QString::number(baud), baud);
    }
    const int defaultBaudIndex = baudCombo_->findData(115200);
    if (defaultBaudIndex >= 0) {
        baudCombo_->setCurrentIndex(defaultBaudIndex);
    }

    refreshPortsButton_ = new QPushButton(style()->standardIcon(QStyle::SP_BrowserReload),
                                          QStringLiteral("Refresh"), serialGroup);
    connectButton_ = new QPushButton(style()->standardIcon(QStyle::SP_DialogApplyButton),
                                     QStringLiteral("Connect"), serialGroup);

    serialLayout->addWidget(new QLabel(QStringLiteral("Port"), serialGroup), 0, 0);
    serialLayout->addWidget(portCombo_, 0, 1);
    serialLayout->addWidget(refreshPortsButton_, 0, 2);
    serialLayout->addWidget(new QLabel(QStringLiteral("Baud"), serialGroup), 0, 3);
    serialLayout->addWidget(baudCombo_, 0, 4);
    serialLayout->addWidget(connectButton_, 0, 5);
    serialLayout->setColumnStretch(1, 1);
    root->addWidget(serialGroup);

    auto *workArea = new QHBoxLayout;
    workArea->setSpacing(10);
    root->addLayout(workArea, 1);

    auto *leftColumn = new QVBoxLayout;
    auto *rightColumn = new QVBoxLayout;
    leftColumn->setSpacing(10);
    rightColumn->setSpacing(10);
    workArea->addLayout(leftColumn, 1);
    workArea->addLayout(rightColumn, 1);

    auto *controlGroup = new QGroupBox(QStringLiteral("Engine Control"), central);
    auto *controlLayout = new QGridLayout(controlGroup);
    controlLayout->setColumnStretch(1, 1);

    adcModeButton_ = new QRadioButton(QStringLiteral("ADC"), controlGroup);
    uartModeButton_ = new QRadioButton(QStringLiteral("UART"), controlGroup);
    adcModeButton_->setChecked(true);

    auto *modeLayout = new QHBoxLayout;
    modeLayout->addWidget(adcModeButton_);
    modeLayout->addWidget(uartModeButton_);
    modeLayout->addStretch(1);

    throttleSlider_ = makePercentSlider();
    throttleSpin_ = makePercentSpinBox();
    brakeSlider_ = makePercentSlider();
    brakeSpin_ = makePercentSpinBox();

    controlLayout->addWidget(new QLabel(QStringLiteral("Mode"), controlGroup), 0, 0);
    controlLayout->addLayout(modeLayout, 0, 1, 1, 2);

    controlLayout->addWidget(new QLabel(QStringLiteral("Throttle"), controlGroup), 1, 0);
    controlLayout->addWidget(throttleSlider_, 1, 1);
    controlLayout->addWidget(throttleSpin_, 1, 2);

    controlLayout->addWidget(new QLabel(QStringLiteral("Brake"), controlGroup), 2, 0);
    controlLayout->addWidget(brakeSlider_, 2, 1);
    controlLayout->addWidget(brakeSpin_, 2, 2);

    liveUpdateCheck_ = new QCheckBox(QStringLiteral("Live update"), controlGroup);
    liveUpdateCheck_->setChecked(true);

    applyPedalButton_ = new QPushButton(style()->standardIcon(QStyle::SP_DialogApplyButton),
                                        QStringLiteral("Apply Pedal"), controlGroup);
    stopButton_ = new QPushButton(style()->standardIcon(QStyle::SP_MediaStop),
                                  QStringLiteral("Stop"), controlGroup);
    resetButton_ = new QPushButton(style()->standardIcon(QStyle::SP_BrowserReload),
                                   QStringLiteral("Reset"), controlGroup);

    auto *pedalButtonLayout = new QHBoxLayout;
    pedalButtonLayout->addWidget(liveUpdateCheck_);
    pedalButtonLayout->addStretch(1);
    pedalButtonLayout->addWidget(applyPedalButton_);
    pedalButtonLayout->addWidget(stopButton_);
    pedalButtonLayout->addWidget(resetButton_);
    controlLayout->addLayout(pedalButtonLayout, 3, 0, 1, 3);

    monitorIntervalSpin_ = new QSpinBox(controlGroup);
    monitorIntervalSpin_->setRange(100, 5000);
    monitorIntervalSpin_->setSingleStep(50);
    monitorIntervalSpin_->setValue(200);
    monitorIntervalSpin_->setSuffix(QStringLiteral(" ms"));

    monitorOnButton_ = new QPushButton(QStringLiteral("Monitor On"), controlGroup);
    monitorOffButton_ = new QPushButton(QStringLiteral("Monitor Off"), controlGroup);
    monitorOnceButton_ = new QPushButton(QStringLiteral("Once"), controlGroup);

    auto *monitorLayout = new QHBoxLayout;
    monitorLayout->addWidget(new QLabel(QStringLiteral("Monitor"), controlGroup));
    monitorLayout->addWidget(monitorIntervalSpin_);
    monitorLayout->addWidget(monitorOnButton_);
    monitorLayout->addWidget(monitorOffButton_);
    monitorLayout->addWidget(monitorOnceButton_);
    controlLayout->addLayout(monitorLayout, 4, 0, 1, 3);

    leftColumn->addWidget(controlGroup);

    auto *statusGroup = new QGroupBox(QStringLiteral("Engine Status"), central);
    auto *statusLayout = new QGridLayout(statusGroup);
    statusLayout->setColumnStretch(1, 1);

    connectionValueLabel_ = new QLabel(QStringLiteral("Disconnected"), statusGroup);
    modeValueLabel_ = new QLabel(QStringLiteral("adc"), statusGroup);
    txCountValueLabel_ = new QLabel(QStringLiteral("0"), statusGroup);

    throttleBar_ = makeProgressBar(100, QStringLiteral("%v / %m"));
    brakeBar_ = makeProgressBar(100, QStringLiteral("%v / %m"));
    rpmBar_ = makeProgressBar(6000, QStringLiteral("%v rpm"));
    speedBar_ = makeProgressBar(130, QStringLiteral("%v km/h"));
    coolantBar_ = makeProgressBar(120, QStringLiteral("%v C"));

    statusLayout->addWidget(new QLabel(QStringLiteral("Connection"), statusGroup), 0, 0);
    statusLayout->addWidget(connectionValueLabel_, 0, 1);
    statusLayout->addWidget(new QLabel(QStringLiteral("Mode"), statusGroup), 1, 0);
    statusLayout->addWidget(modeValueLabel_, 1, 1);
    statusLayout->addWidget(new QLabel(QStringLiteral("Throttle"), statusGroup), 2, 0);
    statusLayout->addWidget(throttleBar_, 2, 1);
    statusLayout->addWidget(new QLabel(QStringLiteral("Brake"), statusGroup), 3, 0);
    statusLayout->addWidget(brakeBar_, 3, 1);
    statusLayout->addWidget(new QLabel(QStringLiteral("RPM"), statusGroup), 4, 0);
    statusLayout->addWidget(rpmBar_, 4, 1);
    statusLayout->addWidget(new QLabel(QStringLiteral("Speed"), statusGroup), 5, 0);
    statusLayout->addWidget(speedBar_, 5, 1);
    statusLayout->addWidget(new QLabel(QStringLiteral("Coolant"), statusGroup), 6, 0);
    statusLayout->addWidget(coolantBar_, 6, 1);
    statusLayout->addWidget(new QLabel(QStringLiteral("TX Count"), statusGroup), 7, 0);
    statusLayout->addWidget(txCountValueLabel_, 7, 1);

    leftColumn->addWidget(statusGroup);
    leftColumn->addStretch(1);

    auto *consoleGroup = new QGroupBox(QStringLiteral("Serial Console"), central);
    auto *consoleLayout = new QVBoxLayout(consoleGroup);

    terminalLog_ = new QPlainTextEdit(consoleGroup);
    terminalLog_->setReadOnly(true);
    terminalLog_->setMaximumBlockCount(1200);

    commandEdit_ = new QLineEdit(consoleGroup);
    commandEdit_->setPlaceholderText(QStringLiteral("CLI command"));
    sendCommandButton_ = new QPushButton(QStringLiteral("Send"), consoleGroup);

    auto *commandLayout = new QHBoxLayout;
    commandLayout->addWidget(commandEdit_, 1);
    commandLayout->addWidget(sendCommandButton_);

    consoleLayout->addWidget(terminalLog_, 1);
    consoleLayout->addLayout(commandLayout);

    rightColumn->addWidget(consoleGroup, 1);

    connect(refreshPortsButton_, &QPushButton::clicked, this, &MainWindow::refreshPorts);
    connect(connectButton_, &QPushButton::clicked, this, &MainWindow::toggleConnection);
    connect(sendCommandButton_, &QPushButton::clicked, this, &MainWindow::sendCustomCommand);
    connect(commandEdit_, &QLineEdit::returnPressed, this, &MainWindow::sendCustomCommand);

    connect(adcModeButton_, &QRadioButton::clicked, this, [this]() {
        if (!updatingControls_) {
            sendCommand(QStringLiteral("mode adc"));
        }
    });
    connect(uartModeButton_, &QRadioButton::clicked, this, [this]() {
        if (!updatingControls_) {
            sendCommand(QStringLiteral("mode uart"));
        }
    });

    connect(throttleSlider_, &QSlider::valueChanged, this, [this](int value) {
        if (updatingControls_) {
            return;
        }
        QSignalBlocker blocker(throttleSpin_);
        throttleSpin_->setValue(value);
        throttleBar_->setValue(value);
        schedulePedalSend();
    });
    connect(throttleSpin_, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        if (updatingControls_) {
            return;
        }
        QSignalBlocker blocker(throttleSlider_);
        throttleSlider_->setValue(value);
        throttleBar_->setValue(value);
        schedulePedalSend();
    });
    connect(brakeSlider_, &QSlider::valueChanged, this, [this](int value) {
        if (updatingControls_) {
            return;
        }
        QSignalBlocker blocker(brakeSpin_);
        brakeSpin_->setValue(value);
        brakeBar_->setValue(value);
        schedulePedalSend();
    });
    connect(brakeSpin_, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        if (updatingControls_) {
            return;
        }
        QSignalBlocker blocker(brakeSlider_);
        brakeSlider_->setValue(value);
        brakeBar_->setValue(value);
        schedulePedalSend();
    });

    connect(applyPedalButton_, &QPushButton::clicked, this, &MainWindow::sendPedal);
    connect(stopButton_, &QPushButton::clicked, this, [this]() {
        setThrottleControl(0);
        setBrakeControl(0);
        sendCommand(QStringLiteral("stop"));
    });
    connect(resetButton_, &QPushButton::clicked, this, [this]() {
        sendCommand(QStringLiteral("sim_reset"));
        QTimer::singleShot(100, this, [this]() {
            sendCommand(QStringLiteral("status"), false);
        });
    });
    connect(monitorOnButton_, &QPushButton::clicked, this, [this]() {
        sendCommand(QStringLiteral("monitor on %1").arg(monitorIntervalSpin_->value()));
    });
    connect(monitorOffButton_, &QPushButton::clicked, this, [this]() {
        sendCommand(QStringLiteral("monitor off"));
    });
    connect(monitorOnceButton_, &QPushButton::clicked, this, [this]() {
        sendCommand(QStringLiteral("monitor once"));
    });

    statusBar()->showMessage(QStringLiteral("Disconnected"));
}

void MainWindow::refreshPorts()
{
    const QString previousPort = portCombo_->currentData().toString();

    portCombo_->clear();

    const QStringList ports = serial_->availablePorts();
    for (const QString &portName : ports) {
        portCombo_->addItem(portName, portName);
    }

    if (ports.isEmpty()) {
        portCombo_->addItem(QStringLiteral("No serial ports"), QString());
        portCombo_->setEnabled(false);
        appendLog(QStringLiteral("SYS"), QStringLiteral("No serial ports found"));
        return;
    }

    portCombo_->setEnabled(!isSerialOpen());

    const int previousIndex = portCombo_->findData(previousPort);
    if (previousIndex >= 0) {
        portCombo_->setCurrentIndex(previousIndex);
    }
}

void MainWindow::toggleConnection()
{
    if (isSerialOpen()) {
        disconnectSerial();
    } else {
        connectSerial();
    }
}

void MainWindow::connectSerial()
{
    const QString portName = portCombo_->currentData().toString();
    if (portName.isEmpty()) {
        appendLog(QStringLiteral("SYS"), QStringLiteral("Select a serial port first"));
        return;
    }

    if (!serial_->open(portName, baudCombo_->currentData().toInt())) {
        appendLog(QStringLiteral("SYS"), serial_->errorString());
        setConnectionState(QStringLiteral("Disconnected"));
        return;
    }

    rxBuffer_.clear();
    serialPollTimer_.start();

    portCombo_->setEnabled(false);
    baudCombo_->setEnabled(false);
    refreshPortsButton_->setEnabled(false);
    connectButton_->setText(QStringLiteral("Disconnect"));
    connectButton_->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
    setConnectionState(QStringLiteral("Connected to %1").arg(portName));
    appendLog(QStringLiteral("SYS"), QStringLiteral("Connected to %1 at %2 baud")
                  .arg(portName)
                  .arg(baudCombo_->currentData().toInt()));

    sendCommand(QStringLiteral("monitor on %1").arg(monitorIntervalSpin_->value()), false);
    QTimer::singleShot(100, this, [this]() {
        sendCommand(QStringLiteral("status"), false);
    });
}

void MainWindow::disconnectSerial()
{
    if (!serial_ || !serial_->isOpen()) {
        return;
    }

    serialPollTimer_.stop();
    serial_->write("monitor off\r\n");
    serial_->close();

    portCombo_->setEnabled(true);
    baudCombo_->setEnabled(true);
    refreshPortsButton_->setEnabled(true);
    connectButton_->setText(QStringLiteral("Connect"));
    connectButton_->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
    setConnectionState(QStringLiteral("Disconnected"));
    appendLog(QStringLiteral("SYS"), QStringLiteral("Disconnected"));
}

void MainWindow::readSerial()
{
    if (!serial_ || !serial_->isOpen()) {
        return;
    }

    const QByteArray data = serial_->readAll();
    if (serial_->hasError()) {
        handleSerialError(serial_->errorString());
        return;
    }

    if (data.isEmpty()) {
        return;
    }

    rxBuffer_.append(data);

    int newlineIndex = rxBuffer_.indexOf('\n');
    while (newlineIndex >= 0) {
        const QByteArray rawLine = rxBuffer_.left(newlineIndex);
        rxBuffer_.remove(0, newlineIndex + 1);

        const QString line = QString::fromUtf8(rawLine).trimmed();
        if (!line.isEmpty()) {
            handleLine(line);
        }

        newlineIndex = rxBuffer_.indexOf('\n');
    }
}

void MainWindow::handleSerialError(const QString &message)
{
    appendLog(QStringLiteral("ERR"), message);

    if (serial_) {
        serialPollTimer_.stop();
        serial_->close();
    }

    portCombo_->setEnabled(true);
    baudCombo_->setEnabled(true);
    refreshPortsButton_->setEnabled(true);
    connectButton_->setText(QStringLiteral("Connect"));
    connectButton_->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
    setConnectionState(QStringLiteral("Disconnected"));
}

void MainWindow::sendCommand(const QString &command, bool logTx)
{
    const QString trimmed = command.trimmed();
    if (trimmed.isEmpty()) {
        return;
    }

    if (!isSerialOpen()) {
        appendLog(QStringLiteral("SYS"), QStringLiteral("Not connected: %1").arg(trimmed));
        return;
    }

    QByteArray payload = trimmed.toUtf8();
    payload.append("\r\n");
    if (serial_->write(payload) < 0) {
        handleSerialError(serial_->errorString());
        return;
    }

    if (logTx) {
        appendLog(QStringLiteral("TX"), trimmed);
    }
}

void MainWindow::schedulePedalSend()
{
    if (updatingControls_ || !liveUpdateCheck_->isChecked() || !isSerialOpen()) {
        return;
    }

    pedalDebounce_.start();
}

void MainWindow::sendPedal()
{
    const bool connected = isSerialOpen();

    sendCommand(QStringLiteral("pedal %1 %2")
                    .arg(throttleSpin_->value())
                    .arg(brakeSpin_->value()));

    if (connected) {
        setModeControls(QStringLiteral("uart"));
    }
}

void MainWindow::sendCustomCommand()
{
    const QString command = commandEdit_->text().trimmed();
    if (command.isEmpty()) {
        return;
    }

    sendCommand(command);
    commandEdit_->clear();
}

void MainWindow::handleLine(const QString &line)
{
    appendLog(QStringLiteral("RX"), line);

    if (line.contains(QStringLiteral("MON "))) {
        parseMonitorLine(line);
        return;
    }

    parseStatusLine(line);
}

void MainWindow::parseMonitorLine(const QString &line)
{
    const int monIndex = line.indexOf(QStringLiteral("MON "));
    if (monIndex < 0) {
        return;
    }

    const QString payload = line.mid(monIndex + 4);
    static const QRegularExpression keyValueRegex(QStringLiteral("(\\w+)=([^\\s]+)"));

    QHash<QString, QString> values;
    QRegularExpressionMatchIterator it = keyValueRegex.globalMatch(payload);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        values.insert(match.captured(1).toLower(), match.captured(2));
    }

    applyStatusValues(values);
}

void MainWindow::parseStatusLine(const QString &line)
{
    QString text = line;
    const int promptIndex = text.lastIndexOf(QStringLiteral("CLI >"));
    if (promptIndex >= 0) {
        text = text.mid(promptIndex + 5).trimmed();
    }

    static const QRegularExpression statusRegex(
        QStringLiteral("^(mode|throttle|brake|rpm|speed|coolant|tx_count)\\s*[:=]\\s*([A-Za-z0-9]+)"),
        QRegularExpression::CaseInsensitiveOption);

    const QRegularExpressionMatch match = statusRegex.match(text);
    if (!match.hasMatch()) {
        return;
    }

    QString key = match.captured(1).toLower();
    if (key == QStringLiteral("tx_count")) {
        key = QStringLiteral("tx");
    }

    QHash<QString, QString> values;
    values.insert(key, match.captured(2));
    applyStatusValues(values);
}

void MainWindow::applyStatusValues(const QHash<QString, QString> &values)
{
    const bool wasUpdating = updatingControls_;
    updatingControls_ = true;

    if (values.contains(QStringLiteral("mode"))) {
        setModeControls(values.value(QStringLiteral("mode")).toLower());
    }

    bool ok = false;
    if (values.contains(QStringLiteral("throttle"))) {
        const int value = statusValue(values, QStringLiteral("throttle"), &ok);
        if (ok) {
            throttleBar_->setValue(value);
            if (!throttleSlider_->isSliderDown()) {
                setThrottleControl(value);
            }
        }
    }

    if (values.contains(QStringLiteral("brake"))) {
        const int value = statusValue(values, QStringLiteral("brake"), &ok);
        if (ok) {
            brakeBar_->setValue(value);
            if (!brakeSlider_->isSliderDown()) {
                setBrakeControl(value);
            }
        }
    }

    if (values.contains(QStringLiteral("rpm"))) {
        const int value = statusValue(values, QStringLiteral("rpm"), &ok);
        if (ok) {
            rpmBar_->setValue(value);
        }
    }

    if (values.contains(QStringLiteral("speed"))) {
        const int value = statusValue(values, QStringLiteral("speed"), &ok);
        if (ok) {
            speedBar_->setValue(value);
        }
    }

    if (values.contains(QStringLiteral("coolant"))) {
        const int value = statusValue(values, QStringLiteral("coolant"), &ok);
        if (ok) {
            coolantBar_->setValue(value);
        }
    }

    if (values.contains(QStringLiteral("tx"))) {
        const QString txValue = values.value(QStringLiteral("tx"));
        txCountValueLabel_->setText(txValue);
    }

    updatingControls_ = wasUpdating;
}

void MainWindow::setConnectionState(const QString &state)
{
    connectionValueLabel_->setText(state);
    statusBar()->showMessage(state);
}

void MainWindow::setModeControls(const QString &mode)
{
    const QString normalized = mode.toLower();
    modeValueLabel_->setText(normalized);

    QSignalBlocker adcBlocker(adcModeButton_);
    QSignalBlocker uartBlocker(uartModeButton_);
    adcModeButton_->setChecked(normalized == QStringLiteral("adc"));
    uartModeButton_->setChecked(normalized == QStringLiteral("uart"));
}

void MainWindow::setThrottleControl(int value)
{
    QSignalBlocker sliderBlocker(throttleSlider_);
    QSignalBlocker spinBlocker(throttleSpin_);
    throttleSlider_->setValue(value);
    throttleSpin_->setValue(value);
    throttleBar_->setValue(value);
}

void MainWindow::setBrakeControl(int value)
{
    QSignalBlocker sliderBlocker(brakeSlider_);
    QSignalBlocker spinBlocker(brakeSpin_);
    brakeSlider_->setValue(value);
    brakeSpin_->setValue(value);
    brakeBar_->setValue(value);
}

void MainWindow::appendLog(const QString &direction, const QString &text)
{
    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss.zzz"));
    terminalLog_->appendPlainText(QStringLiteral("[%1] %2 %3").arg(timestamp, direction, text));
}

bool MainWindow::isSerialOpen() const
{
    return serial_ && serial_->isOpen();
}
