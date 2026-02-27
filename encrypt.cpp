#include "encrypt.h"

encrypt::encrypt() {

}

bool encrypt::checkID() {
    getChipSerial();
    readCodeFile();

    if (local_code.isEmpty() || local_code != sha256hexOf(serialno + PRESET_ID)) {
        return false;
    }
    return true;
}

void encrypt::getChipSerial() {
    QFile f(SYS_INFO_PATH);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith(SUNXI_SERIAL_HEAD)) {
            QStringList parts = line.split(':');
            if (parts.size() >= 2)
                serialno = parts[1].trimmed();
        }
    }
}

QString encrypt::sha256hexOf(const QString &raw) {
    return QString(QCryptographicHash::hash(raw.toUtf8(), QCryptographicHash::Sha256).toHex());
}

void encrypt::readCodeFile() {
    QFile f(ENCRYPT_CODE_PATH);
    if ((!f.exists()) || (!f.open(QIODevice::ReadOnly | QIODevice::Text))) {
        return;
    }

    QTextStream in(&f);
    local_code = in.readAll().trimmed().toLower();
}

void encrypt::shutdownSystem() {
    qDebug() << "Err in Code";
//    QProcess::execute("poweroff");
}
