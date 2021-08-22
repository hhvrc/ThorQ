#include "errorlabel.h"

ThorQ::UI::ErrorLabel::ErrorLabel(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)
{}

ThorQ::UI::ErrorLabel::ErrorLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
    : QLabel(text, parent, f)
{}

ThorQ::UI::ErrorLabel::~ErrorLabel()
{}
