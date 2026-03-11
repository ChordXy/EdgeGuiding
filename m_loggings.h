#ifndef LOGGINGS_H
#define LOGGINGS_H

#include <QDateTime>
#include <QString>
#include <QDebug>


#define DEBUG

#ifdef DEBUG
    #define QDB qDebug().noquote() <<
#else
    #define QDB
#endif

inline void logging(const QString & info, const QString & pfrom=""){
    QString wt = "";
    if (!pfrom.isEmpty())
        wt = " [" + pfrom + "] ";
    QString timeString = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz] ") + wt + info;
    QDB timeString;
}


#endif // LOGGINGS_H
