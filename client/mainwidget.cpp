#include "mainwidget.h"

#include "userlistitem.h"
#include "userlistmodel.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QListView>

ThorQ::MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
	, m_vlayout(new QVBoxLayout(this))
	, m_hlayout(new QHBoxLayout())
{
	setLayout(m_vlayout);
}
