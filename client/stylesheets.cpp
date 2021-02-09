#include "stylesheets.h"

#include <QFile>
#include <QTextStream>

QString ThorQ::StyleSheets::tryGetStylesheet(QString styleSheetName)
{
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

    return stylesheet;
}
