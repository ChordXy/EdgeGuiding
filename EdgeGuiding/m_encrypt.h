#ifndef M_ENCRYPT_H
#define M_ENCRYPT_H


#include <QCryptographicHash>
#include <QTextStream>
#include <QProcess>
#include <QDebug>
#include <QFile>

#include "define.h"

bool checkID();
void shutdownSystem();
QString getChipSerial();
QString readCodeFile();
QString sha256hexOf(const QString &raw);

#endif // M_ENCRYPT_H
