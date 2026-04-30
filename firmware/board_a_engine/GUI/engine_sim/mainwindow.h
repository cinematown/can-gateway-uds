#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QByteArray>
#include <QHash>
#include <QMainWindow>
#include <QString>
#include <QTimer>

#include "serialconnection.h"

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QProgressBar;
class QPushButton;
class QRadioButton;
class QSlider;
class QSpinBox;

class MainWindow : public QMainWindow
{
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void setupUi();
    void refreshPorts();
    void toggleConnection();
    void connectSerial();
    void disconnectSerial();
    void readSerial();
    void handleSerialError(const QString &message);

    void sendCommand(const QString &command, bool logTx = true);
    void schedulePedalSend();
    void sendPedal();
    void sendCustomCommand();

    void handleLine(const QString &line);
    void parseMonitorLine(const QString &line);
    void parseStatusLine(const QString &line);
    void applyStatusValues(const QHash<QString, QString> &values);

    void setConnectionState(const QString &state);
    void setModeControls(const QString &mode);
    void setThrottleControl(int value);
    void setBrakeControl(int value);
    void appendLog(const QString &direction, const QString &text);
    bool isSerialOpen() const;

    SerialConnection *serial_ = nullptr;
    QByteArray rxBuffer_;
    QTimer serialPollTimer_;
    QTimer pedalDebounce_;
    bool updatingControls_ = false;

    QComboBox *portCombo_ = nullptr;
    QComboBox *baudCombo_ = nullptr;
    QPushButton *refreshPortsButton_ = nullptr;
    QPushButton *connectButton_ = nullptr;

    QRadioButton *adcModeButton_ = nullptr;
    QRadioButton *uartModeButton_ = nullptr;
    QSlider *throttleSlider_ = nullptr;
    QSlider *brakeSlider_ = nullptr;
    QSpinBox *throttleSpin_ = nullptr;
    QSpinBox *brakeSpin_ = nullptr;
    QCheckBox *liveUpdateCheck_ = nullptr;
    QSpinBox *monitorIntervalSpin_ = nullptr;
    QPushButton *applyPedalButton_ = nullptr;
    QPushButton *stopButton_ = nullptr;
    QPushButton *resetButton_ = nullptr;
    QPushButton *monitorOnButton_ = nullptr;
    QPushButton *monitorOffButton_ = nullptr;
    QPushButton *monitorOnceButton_ = nullptr;

    QLabel *connectionValueLabel_ = nullptr;
    QLabel *modeValueLabel_ = nullptr;
    QProgressBar *throttleBar_ = nullptr;
    QProgressBar *brakeBar_ = nullptr;
    QProgressBar *rpmBar_ = nullptr;
    QProgressBar *speedBar_ = nullptr;
    QProgressBar *coolantBar_ = nullptr;
    QLabel *txCountValueLabel_ = nullptr;

    QPlainTextEdit *terminalLog_ = nullptr;
    QLineEdit *commandEdit_ = nullptr;
    QPushButton *sendCommandButton_ = nullptr;
};

#endif
