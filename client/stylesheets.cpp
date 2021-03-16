#include "stylesheets.h"

#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>

QMutex l_styleSheetCache;
QMap<QString, QString> g_styleSheetCache;

QString ThorQ::StyleSheets::tryGetStylesheet(QString styleSheetName)
{
    QMutexLocker locker(&l_styleSheetCache);

    auto it = g_styleSheetCache.find(styleSheetName);
    if (it != g_styleSheetCache.end()) {
        return it.value();
    }

    QString fileName = "stylesheets/" + styleSheetName + ".css";

    QString stylesheet;

    if (!QFile::exists(fileName)) {
        fileName.prepend(":/");
    }

    QFile file(fileName);
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream stream(&file);
        stylesheet = stream.readAll();
    }
    else
    {
        stylesheet.clear();
    }

    g_styleSheetCache.insert(styleSheetName, stylesheet);
    return stylesheet;
}
