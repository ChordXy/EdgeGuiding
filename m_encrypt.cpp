#include "m_encrypt.h"


bool checkID() {
    QString serialno = getChipSerial();
    QString local_code = readCodeFile();

    if (local_code.isEmpty() || local_code != sha256hexOf(serialno + PRESET_ID)) {
        return false;
    }
    return true;
}

QString getChipSerial() {
    QFile f(SYS_INFO_PATH);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();

    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith(SUNXI_SERIAL_HEAD)) {
            QStringList parts = line.split(':');
            if (parts.size() >= 2)
                return parts[1].trimmed();
        }
    }
    return QString();
}

QString sha256hexOf(const QString &raw) {
    return QString(QCryptographicHash::hash(raw.toUtf8(), QCryptographicHash::Sha256).toHex());
}

QString readCodeFile() {
    QFile f(ENCRYPT_CODE_PATH);
    if ((!f.exists()) || (!f.open(QIODevice::ReadOnly | QIODevice::Text))) {
        return QString();
    }

    QTextStream in(&f);
    return in.readAll().trimmed().toLower();
}

void shutdownSystem() {
    QProcess::execute("poweroff");
}
