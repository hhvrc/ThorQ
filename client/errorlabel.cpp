#include "errorlabel.h"

ErrorLabel::ErrorLabel(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)
{}

ErrorLabel::ErrorLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
    : QLabel(text, parent, f)
{}

ErrorLabel::~ErrorLabel()
{}
