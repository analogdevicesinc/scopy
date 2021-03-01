#include <scopy/gui/two_buttons_widget.hpp>

#include "ui_two_buttons_widget.h"

#include <QFile>
#include <QPushButton>
#include <QVBoxLayout>

using namespace scopy::gui;

TwoButtonsWidget::TwoButtonsWidget(const QString& text, QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::TwoButtonsWidget)
{
	m_ui->setupUi(this);

	//	Q_INIT_RESOURCE(resources);

	auto lay = new QVBoxLayout();
	this->setLayout(lay);
	auto btn1 = new QPushButton(text, this);

	QFile ss(":/twobuttonsstylesheet");
	ss.open(QFile::ReadOnly);
	QString sss = QLatin1String(ss.readAll());
	btn1->setStyleSheet(sss);

	lay->addWidget(btn1);
	lay->addWidget(new QPushButton(text, this));
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
}

TwoButtonsWidget::~TwoButtonsWidget() { delete m_ui; }
