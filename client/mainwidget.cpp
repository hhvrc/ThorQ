#include "mainwidget.h"

#include "userlistitem.h"
#include "userlistmodel.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QListView>

ThorQ::MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , m_vlayout(new QVBoxLayout())
    , m_hlayout(new QHBoxLayout())
{/*
    m_vlayout->setParent(this);
    m_hlayout->setParent(this);
	setLayout(m_vlayout);
*/}
