#include "SerialBridge.h"

#include <QDateTime>
#include <QRegularExpression>
#include <QSerialPortInfo>

SerialBridge::SerialBridge(QObject *parent)
    : QObject(parent)
{
    connect(&m_serial, &QSerialPort::readyRead, this, &SerialBridge::onReadyRead);
    connect(&m_serial, &QSerialPort::errorOccurred, this, &SerialBridge::onErrorOccurred);
    refreshPorts();
    setStatusText("Select a serial port");
}

QStringList SerialBridge::portNames() const { return m_portNames; }
bool SerialBridge::connected() const { return m_serial.isOpen(); }
QString SerialBridge::statusText() const { return m_statusText; }

int SerialBridge::can1Rx() const { return m_can1Rx; }
int SerialBridge::can1Tx() const { return m_can1Tx; }
int SerialBridge::can2Rx() const { return m_can2Rx; }
int SerialBridge::can2Tx() const { return m_can2Tx; }
int SerialBridge::busy() const { return m_busy; }
int SerialBridge::errors() const { return m_errors; }
bool SerialBridge::warning() const { return m_warning; }

int SerialBridge::rpm() const { return m_rpm; }
int SerialBridge::speed() const { return m_speed; }
int SerialBridge::coolant() const { return m_coolant; }
bool SerialBridge::ignition() const { return m_ignition; }
int SerialBridge::alive() const { return m_alive; }
QString SerialBridge::lastEngineRx() const { return m_lastEngineRx; }

bool SerialBridge::doorFl() const { return m_doorFl; }
bool SerialBridge::doorFr() const { return m_doorFr; }
bool SerialBridge::doorRl() const { return m_doorRl; }
bool SerialBridge::doorRr() const { return m_doorRr; }
bool SerialBridge::turnLeft() const { return m_turnLeft; }
bool SerialBridge::turnRight() const { return m_turnRight; }
bool SerialBridge::highBeam() const { return m_highBeam; }
bool SerialBridge::fogLamp() const { return m_fogLamp; }
QString SerialBridge::lastBodyRx() const { return m_lastBodyRx; }

bool SerialBridge::clusterRpmActive() const { return m_clusterRpmActive; }
bool SerialBridge::clusterSpeedActive() const { return m_clusterSpeedActive; }
bool SerialBridge::clusterBodyActive() const { return m_clusterBodyActive; }
QString SerialBridge::latestFrameId() const { return m_latestFrameId; }
QString SerialBridge::latestFrameBus() const { return m_latestFrameBus; }
QString SerialBridge::latestFrameDir() const { return m_latestFrameDir; }
QString SerialBridge::latestFrameDlc() const { return m_latestFrameDlc; }
QString SerialBridge::latestFrameRaw() const { return m_latestFrameRaw; }
QStringList SerialBridge::latestFrameBytes() const { return m_latestFrameBytes; }
QString SerialBridge::latestFrameDecoded() const { return m_latestFrameDecoded; }

void SerialBridge::refreshPorts()
{
    QStringList names;
    const QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &port : ports) {
        names.append(port.systemLocation());
    }

    if (names != m_portNames) {
        m_portNames = names;
        emit portNamesChanged();
    }

    if (m_portNames.isEmpty()) {
        setStatusText("No serial ports found");
    }
}

void SerialBridge::resetMonitor()
{
    if (m_serial.isOpen()) {
        m_serial.close();
        emit connectionChanged();
    }

    m_rxBuffer.clear();
    m_can1Rx = 0;
    m_can1Tx = 0;
    m_can2Rx = 0;
    m_can2Tx = 0;
    m_busy = 0;
    m_errors = 0;
    m_warning = false;
    m_rpm = 0;
    m_speed = 0;
    m_coolant = 0;
    m_ignition = false;
    m_alive = 0;
    m_lastEngineRx = "-";
    m_doorFl = false;
    m_doorFr = false;
    m_doorRl = false;
    m_doorRr = false;
    m_turnLeft = false;
    m_turnRight = false;
    m_highBeam = false;
    m_fogLamp = false;
    m_lastBodyRx = "-";
    m_clusterRpmActive = false;
    m_clusterSpeedActive = false;
    m_clusterBodyActive = false;
    m_latestFrameId = "-";
    m_latestFrameBus = "-";
    m_latestFrameDir = "-";
    m_latestFrameDlc = "-";
    m_latestFrameRaw = "-- -- -- -- -- -- -- --";
    m_latestFrameBytes = {"--", "--", "--", "--", "--", "--", "--", "--"};
    m_latestFrameDecoded = "-";

    refreshPorts();
    setStatusText("Serial monitor reset");
    emit dataChanged();
}

void SerialBridge::connectToPort(const QString &portName, int baudRate)
{
    if (m_serial.isOpen()) {
        m_serial.close();
        emit connectionChanged();
    }

    if (portName.isEmpty()) {
        setStatusText("Select a serial port first");
        return;
    }

    m_serial.setPortName(portName);
    m_serial.setBaudRate(baudRate);
    m_serial.setDataBits(QSerialPort::Data8);
    m_serial.setParity(QSerialPort::NoParity);
    m_serial.setStopBits(QSerialPort::OneStop);
    m_serial.setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial.open(QIODevice::ReadWrite)) {
        setStatusText("Open failed: " + m_serial.errorString());
        emit connectionChanged();
        return;
    }

    m_rxBuffer.clear();
    setStatusText("Connected to " + portName);
    emit connectionChanged();
}

void SerialBridge::disconnectPort()
{
    if (m_serial.isOpen()) {
        m_serial.close();
    }
    setStatusText("Disconnected");
    emit connectionChanged();
}

void SerialBridge::sendCommand(const QString &command)
{
    if (!m_serial.isOpen()) {
        setStatusText("Serial port is not connected");
        return;
    }

    QByteArray payload = command.toUtf8();
    if (!payload.endsWith('\n')) {
        payload.append("\r\n");
    }
    m_serial.write(payload);
}

void SerialBridge::onReadyRead()
{
    m_rxBuffer.append(m_serial.readAll());

    while (true) {
        const int newline = m_rxBuffer.indexOf('\n');
        if (newline < 0) {
            break;
        }

        QByteArray lineBytes = m_rxBuffer.left(newline);
        m_rxBuffer.remove(0, newline + 1);
        lineBytes.replace("\r", "");

        const QString line = QString::fromUtf8(lineBytes).trimmed();
        if (!line.isEmpty()) {
            processLine(line);
        }
    }
}

void SerialBridge::onErrorOccurred(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError) {
        return;
    }

    if (error == QSerialPort::ResourceError) {
        setStatusText("Serial disconnected: " + m_serial.errorString());
        m_serial.close();
        emit connectionChanged();
    }
}

void SerialBridge::setStatusText(const QString &text)
{
    if (m_statusText == text) {
        return;
    }
    m_statusText = text;
    emit statusTextChanged();
}

void SerialBridge::processLine(const QString &line)
{
    if (line.startsWith("[GW]")) {
        parseGatewayStatus(line);
        emit gatewayLineReceived(nowString(), line);
        return;
    }

    if (line.startsWith("[RX") || line.startsWith("[TX")) {
        parseFrameLine(line);
        return;
    }

    emit rawLineReceived(nowString(), line);
}

void SerialBridge::parseGatewayStatus(const QString &line)
{
    static const QRegularExpression regex(
        R"(\[GW\]\s+RX1=(\d+)\s+TX1=(\d+)\s+RX2=(\d+)\s+TX2=(\d+)\s+busy=(\d+)\s+err=(\d+)\s+warn=(\d+)(?:\s+rpm=(\d+)\s+speed=(\d+)\s+coolant=(\d+)\s+ign=(\d+)\s+alive=(\d+)\s+active=(\d+)\s+age=(\d+))?)");
    const QRegularExpressionMatch match = regex.match(line);
    if (!match.hasMatch()) {
        return;
    }

    m_can1Rx = match.captured(1).toInt();
    m_can1Tx = match.captured(2).toInt();
    m_can2Rx = match.captured(3).toInt();
    m_can2Tx = match.captured(4).toInt();
    m_busy = match.captured(5).toInt();
    m_errors = match.captured(6).toInt();
    m_warning = match.captured(7).toInt() != 0;

    if (!match.captured(8).isEmpty()) {
        m_rpm = match.captured(8).toInt();
        m_speed = match.captured(9).toInt();
        m_coolant = match.captured(10).toInt();
        m_ignition = match.captured(11).toInt() != 0;
        m_alive = match.captured(12).toInt();
        m_lastEngineRx = match.captured(14) + " ms";
    }

    emit dataChanged();
}

void SerialBridge::parseFrameLine(const QString &line)
{
    static const QRegularExpression regex(
        R"(\[(RX|TX)([12])\]\s+id=0x([0-9A-Fa-f]+)\s+dlc=(\d+)\s+data=([0-9A-Fa-f ]+)(?:\s+st=(-?\d+))?)");
    const QRegularExpressionMatch match = regex.match(line);
    if (!match.hasMatch()) {
        emit rawLineReceived(nowString(), line);
        return;
    }

    const QString dir = match.captured(1);
    const QString bus = "CAN" + match.captured(2);
    const QString id = "0x" + match.captured(3).toUpper();
    const QString dlc = match.captured(4);
    const QString payload = match.captured(5).simplified().toUpper();

    QList<int> bytes;
    QStringList normalizedBytes;
    const QStringList byteTexts = payload.split(' ', Qt::SkipEmptyParts);
    for (const QString &byteText : byteTexts) {
        bool ok = false;
        const int value = byteText.toInt(&ok, 16);
        if (ok) {
            bytes.append(value & 0xFF);
            normalizedBytes.append(QString("%1").arg(value & 0xFF, 2, 16, QLatin1Char('0')).toUpper());
        }
    }

    while (normalizedBytes.size() < 8) {
        normalizedBytes.append("--");
    }

    const QString decoded = decodeFrame(id, bytes, dir);
    m_latestFrameId = id;
    m_latestFrameBus = bus;
    m_latestFrameDir = dir;
    m_latestFrameDlc = dlc;
    m_latestFrameRaw = payload;
    m_latestFrameBytes = normalizedBytes.mid(0, 8);
    m_latestFrameDecoded = decoded.isEmpty() ? "No decoder registered for this CAN ID" : decoded;
    emit dataChanged();

    emit frameLineReceived(nowString(), bus, dir, id, dlc, payload, decoded);
}

QString SerialBridge::decodeFrame(const QString &id, const QList<int> &bytes, const QString &dir)
{
    if (id == "0x100" && bytes.size() >= 6) {
        m_rpm = bytes[0] | (bytes[1] << 8);
        m_speed = bytes[2] | (bytes[3] << 8);
        m_coolant = bytes[4];
        m_ignition = (bytes[5] & 0x01) != 0;
        m_alive = (bytes[5] & 0xFE) >> 1;
        m_lastEngineRx = nowString();
        emit dataChanged();
        return QString("rpm=%1 speed=%2 coolant=%3 ign=%4 alive=%5")
            .arg(m_rpm)
            .arg(m_speed)
            .arg(m_coolant)
            .arg(m_ignition ? 1 : 0)
            .arg(m_alive);
    }

    if (id == "0x390" && bytes.size() >= 8) {
        const bool anyDoor = bitAt(bytes, 4) || bitAt(bytes, 16);
        m_doorFl = anyDoor;
        m_doorFr = anyDoor;
        m_doorRl = anyDoor;
        m_doorRr = anyDoor;
        m_turnLeft = bitAt(bytes, 34);
        m_turnRight = bitAt(bytes, 35);
        m_highBeam = bitAt(bytes, 37) || bitAt(bytes, 49);
        m_fogLamp = bitAt(bytes, 58);
        m_lastBodyRx = nowString();
        if (dir == "TX") {
            m_clusterBodyActive = true;
        }
        emit dataChanged();
        return QString("door=%1 left=%2 right=%3 high=%4 fog=%5")
            .arg(anyDoor ? 1 : 0)
            .arg(m_turnLeft ? 1 : 0)
            .arg(m_turnRight ? 1 : 0)
            .arg(m_highBeam ? 1 : 0)
            .arg(m_fogLamp ? 1 : 0);
    }

    if (id == "0x280") {
        QString decoded = "cluster rpm frame";
        if (bytes.size() >= 5) {
            const int clusterRpm = static_cast<int>(leSignal(bytes, 16, 16) / 4U);
            decoded = QString("cluster_rpm=%1").arg(clusterRpm);
        }

        if (dir == "TX") {
            m_clusterRpmActive = true;
            emit dataChanged();
        }
        return decoded;
    }

    if (id == "0x1A0") {
        QString decoded = "cluster speed frame";
        if (bytes.size() >= 4) {
            const int clusterSpeed = static_cast<int>(leSignal(bytes, 17, 15) / 100U);
            decoded = QString("cluster_speed=%1 km/h").arg(clusterSpeed);
        }

        if (dir == "TX") {
            m_clusterSpeedActive = true;
            emit dataChanged();
        }
        return decoded;
    }

    if (id == "0x288") {
        if (bytes.size() >= 2) {
            const int raw = static_cast<int>(leSignal(bytes, 8, 8));
            const int temp = (raw * 3) / 4 - 48;
            return QString("cluster_coolant=%1 C").arg(temp);
        }
        return "cluster coolant frame";
    }

    if (id == "0x480") {
        m_warning = true;
        emit dataChanged();
        const bool rpmWarning = !bytes.isEmpty() && ((bytes[0] & 0x01) != 0);
        const bool overheat = !bytes.isEmpty() && ((bytes[0] & 0x02) != 0);
        const bool general = !bytes.isEmpty() && ((bytes[0] & 0x04) != 0);
        return QString("warning rpm=%1 overheat=%2 general=%3")
            .arg(rpmWarning ? 1 : 0)
            .arg(overheat ? 1 : 0)
            .arg(general ? 1 : 0);
    }

    return "";
}

uint32_t SerialBridge::leSignal(const QList<int> &bytes, int startBit, int bitLength) const
{
    uint32_t raw = 0;
    for (int i = 0; i < bitLength; ++i) {
        const int bitPos = startBit + i;
        const int byteIndex = bitPos / 8;
        const int bitIndex = bitPos % 8;
        if (byteIndex >= 0 && byteIndex < bytes.size() &&
            ((bytes[byteIndex] & (1 << bitIndex)) != 0)) {
            raw |= (1u << i);
        }
    }
    return raw;
}

QString SerialBridge::nowString() const
{
    return QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
}

bool SerialBridge::bitAt(const QList<int> &bytes, int bit) const
{
    const int byteIndex = bit / 8;
    const int bitIndex = bit % 8;
    if (byteIndex < 0 || byteIndex >= bytes.size()) {
        return false;
    }
    return (bytes[byteIndex] & (1 << bitIndex)) != 0;
}
