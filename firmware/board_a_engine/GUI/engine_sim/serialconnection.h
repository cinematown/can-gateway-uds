#ifndef SERIALCONNECTION_H
#define SERIALCONNECTION_H

#include <QByteArray>
#include <QString>
#include <QStringList>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

class SerialConnection
{
public:
    SerialConnection();
    ~SerialConnection();

    QStringList availablePorts() const;
    bool open(const QString &portName, int baudRate);
    void close();
    bool isOpen() const;
    qint64 write(const QByteArray &data);
    QByteArray readAll();
    QString errorString() const;
    bool hasError() const;

private:
    void setLastError(const QString &prefix);

    QString lastError_;
    bool hasError_ = false;

#ifdef Q_OS_WIN
    HANDLE handle_ = INVALID_HANDLE_VALUE;
#endif
};

#endif
