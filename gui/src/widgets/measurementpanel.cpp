#include "widgets/measurementpanel.h"
#include "ui_measure_panel.h"
#include <QWidget>
#include <QGridLayout>

using namespace scopy::gui;

MeasurementsPanel::MeasurementsPanel(QWidget *parent) : QWidget(parent)
{
	setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	ui = new Ui::MeasurementsPanel();
	ui->setupUi(this);

	connect(ui->scrollArea->horizontalScrollBar(), &QScrollBar::rangeChanged,
			ui->scrollArea_2->horizontalScrollBar(), &QScrollBar::setRange);

	connect(ui->scrollArea_2->horizontalScrollBar(), &QScrollBar::valueChanged,
			ui->scrollArea->horizontalScrollBar(), &QScrollBar::setValue);
	connect(ui->scrollArea->horizontalScrollBar(), &QScrollBar::valueChanged,
			ui->scrollArea_2->horizontalScrollBar(), &QScrollBar::setValue);

	connect(ui->scrollArea->horizontalScrollBar(), &QScrollBar::rangeChanged,
			[=](double v1, double v2){
				ui->scrollArea_2->widget()->setFixedWidth(ui->scrollAreaWidgetContents->width());
			});

	wrap = new RowColumnWrappingWidget(this);
	wrap->setWrappingDirection(4, RowColumnWrappingWidget::HORIZONTAL);
	ui->horizontalLayout_3->addWidget(wrap);
	ui->horizontalLayout_3->setSpacing(0);
}

void MeasurementsPanel::addMeasurement(QWidget *meas) {
	m_labels.append(meas);
	wrap->addWidget(meas);
}

void MeasurementsPanel::removeMeasurement(QWidget *meas) {
	m_labels.removeAll(meas);
	wrap->removeWidget(meas);
//	update();
}

void MeasurementsPanel::sort() {
	update();
}

void MeasurementsPanel::update() {
	wrap->update();

}

QWidget *MeasurementsPanel::cursorArea()
{
	return ui->cursorReadouts;
}



StatsPanel::StatsPanel(QWidget *parent)
{
	QVBoxLayout* lay = new QVBoxLayout(this);
	setLayout(lay);

//	QFrame *frame = new QFrame(this);
//	lay->addWidget(frame);
//	frame->setFrameShape(QFrame::HLine);
//	frame->setLineWidth(1);
//	frame->setStyleSheet("color: rgba(255,255,255,60);");


	lay->setMargin(0);
	lay->setSpacing(0);
	lay->setAlignment(Qt::AlignTop);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	QWidget *panel = new QWidget(this);
	panelLayout = new QHBoxLayout(this);
	panelLayout->setMargin(0);
	panelLayout->setSpacing(6);
	panel->setLayout(panelLayout);
	scrollArea->setWidget(panel);
	scrollArea->setWidgetResizable(true);

	panelLayout->setAlignment(Qt::AlignLeft);
	lay->addWidget(scrollArea);
}

StatsPanel::~StatsPanel()
{

}

void StatsPanel::addStat(QWidget *stat)
{
	m_labels.append(stat);
	panelLayout->addWidget(stat);
}

void StatsPanel::removeStat(QWidget *stat)
{
	m_labels.removeAll(stat);
	panelLayout->removeWidget(stat);
}

void StatsPanel::clear()
{
}

void StatsPanel::sort()
{

}
