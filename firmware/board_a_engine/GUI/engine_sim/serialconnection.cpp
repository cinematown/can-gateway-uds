#include "serialconnection.h"

#include <QtGlobal>

SerialConnection::SerialConnection()
{
}

SerialConnection::~SerialConnection()
{
    close();
}

QStringList SerialConnection::availablePorts() const
{
    QStringList ports;

#ifdef Q_OS_WIN
    wchar_t target[1024] = {0};
    for (int index = 1; index <= 256; ++index) {
        const QString portName = QStringLiteral("COM%1").arg(index);
        if (QueryDosDeviceW(reinterpret_cast<LPCWSTR>(portName.utf16()),
                            target,
                            DWORD(sizeof(target) / sizeof(target[0]))) != 0) {
            ports.append(portName);
        }
    }
#endif

    return ports;
}

bool SerialConnection::open(const QString &portName, int baudRate)
{
    close();
    lastError_.clear();
    hasError_ = false;

#ifdef Q_OS_WIN
    const QString deviceName = QStringLiteral("\\\\.\\%1").arg(portName);
    handle_ = CreateFileW(reinterpret_cast<LPCWSTR>(deviceName.utf16()),
                          GENERIC_READ | GENERIC_WRITE,
                          0,
                          nullptr,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          nullptr);
    if (handle_ == INVALID_HANDLE_VALUE) {
        setLastError(QStringLiteral("Open failed"));
        return false;
    }

    SetupComm(handle_, 4096, 4096);

    DCB dcb = {};
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(handle_, &dcb)) {
        setLastError(QStringLiteral("GetCommState failed"));
        close();
        return false;
    }

    dcb.BaudRate = DWORD(baudRate);
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fBinary = TRUE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;

    if (!SetCommState(handle_, &dcb)) {
        setLastError(QStringLiteral("SetCommState failed"));
        close();
        return false;
    }

    COMMTIMEOUTS timeouts = {};
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 100;

    if (!SetCommTimeouts(handle_, &timeouts)) {
        setLastError(QStringLiteral("SetCommTimeouts failed"));
        close();
        return false;
    }

    EscapeCommFunction(handle_, SETDTR);
    EscapeCommFunction(handle_, CLRRTS);
    PurgeComm(handle_, PURGE_RXCLEAR | PURGE_TXCLEAR);

    return true;
#else
    Q_UNUSED(portName)
    Q_UNUSED(baudRate)
    setLastError(QStringLiteral("This build only supports Windows serial ports."));
    return false;
#endif
}

void SerialConnection::close()
{
#ifdef Q_OS_WIN
    if (handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(handle_);
        handle_ = INVALID_HANDLE_VALUE;
    }
#endif
}

bool SerialConnection::isOpen() const
{
#ifdef Q_OS_WIN
    return handle_ != INVALID_HANDLE_VALUE;
#else
    return false;
#endif
}

qint64 SerialConnection::write(const QByteArray &data)
{
#ifdef Q_OS_WIN
    if (!isOpen() || data.isEmpty()) {
        return 0;
    }

    DWORD written = 0;
    if (!WriteFile(handle_, data.constData(), DWORD(data.size()), &written, nullptr)) {
        setLastError(QStringLiteral("Write failed"));
        return -1;
    }

    return qint64(written);
#else
    Q_UNUSED(data)
    return -1;
#endif
}

QByteArray SerialConnection::readAll()
{
    QByteArray data;

#ifdef Q_OS_WIN
    if (!isOpen()) {
        return data;
    }

    DWORD errors = 0;
    COMSTAT status = {};
    if (!ClearCommError(handle_, &errors, &status)) {
        setLastError(QStringLiteral("Read failed"));
        return data;
    }

    if (status.cbInQue == 0) {
        return data;
    }

    data.resize(int(status.cbInQue));

    DWORD bytesRead = 0;
    if (!ReadFile(handle_, data.data(), status.cbInQue, &bytesRead, nullptr)) {
        data.clear();
        setLastError(QStringLiteral("Read failed"));
        return data;
    }

    data.resize(int(bytesRead));
#endif

    return data;
}

QString SerialConnection::errorString() const
{
    return lastError_;
}

bool SerialConnection::hasError() const
{
    return hasError_;
}

void SerialConnection::setLastError(const QString &prefix)
{
    hasError_ = true;

#ifdef Q_OS_WIN
    const DWORD code = GetLastError();
    if (code == ERROR_SUCCESS) {
        lastError_ = prefix;
        return;
    }

    wchar_t *messageBuffer = nullptr;
    const DWORD length = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                            FORMAT_MESSAGE_FROM_SYSTEM |
                                            FORMAT_MESSAGE_IGNORE_INSERTS,
                                        nullptr,
                                        code,
                                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                        reinterpret_cast<LPWSTR>(&messageBuffer),
                                        0,
                                        nullptr);

    if (length == 0 || messageBuffer == nullptr) {
        lastError_ = QStringLiteral("%1: Windows error %2").arg(prefix).arg(code);
        return;
    }

    const QString message = QString::fromWCharArray(messageBuffer, int(length)).trimmed();
    LocalFree(messageBuffer);
    lastError_ = QStringLiteral("%1: %2").arg(prefix, message);
#else
    lastError_ = prefix;
#endif
}
