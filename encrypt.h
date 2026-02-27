#ifndef ENCRYPT_H
#define ENCRYPT_H


#include <QCryptographicHash>
#include <QTextStream>
#include <QProcess>
#include <QDebug>
#include <QFile>

#include "define.h"


class encrypt
{
public:
    encrypt();
    bool checkID();
    void shutdownSystem();

private:
    void getChipSerial();
    void readCodeFile();
    QString sha256hexOf(const QString &raw);

    QString serialno;
    QString local_code;
};

#endif // ENCRYPT_H
