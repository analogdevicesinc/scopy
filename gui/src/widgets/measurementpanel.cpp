#include "widgets/measurementpanel.h"
#include <QScrollArea>
#include <QWidget>
#include <QGridLayout>

using namespace scopy;

MeasurementsPanel::MeasurementsPanel(QWidget *parent) : QWidget(parent)
{
	QVBoxLayout* lay = new QVBoxLayout(this);
	setLayout(lay);

	QScrollBar *scrollBar = new QScrollBar(this);
	scrollBar->setOrientation(Qt::Horizontal);

	lay->setMargin(0);
	lay->setSpacing(0);
	lay->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setHorizontalScrollBar(scrollBar);
	scrollBar->setVisible(false);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	QWidget *panel = new QWidget(this);
	panelLayout = new QHBoxLayout(this);
	panelLayout->setMargin(0);
	panelLayout->setSpacing(12);
	panelLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	panel->setLayout(panelLayout);
	panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	scrollArea->setWidget(panel);
	scrollArea->setWidgetResizable(true);

	panelLayout->setAlignment(Qt::AlignLeft);
	lay->addWidget(scrollBar);
	lay->addWidget(scrollArea);

	connect(scrollArea->horizontalScrollBar(), &QAbstractSlider::rangeChanged, scrollBar, [=](double min, double max) {
		auto singleStep = scrollArea->horizontalScrollBar()->singleStep();
		scrollBar->setVisible(singleStep < (max-min));
	});

	m_cursor = new QWidget(panel);
	panelLayout->addWidget(m_cursor);

	spacer = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding);
	panelLayout->addSpacerItem(spacer);

	int idx = panelLayout->indexOf(spacer);	
	m_stacks.append(new VerticalWidgetStack(this));
	panelLayout->insertWidget(idx, m_stacks.last());

}

void MeasurementsPanel::addWidget(QWidget *meas) {
	if(m_stacks.last()->full()) {
		m_stacks.append(new VerticalWidgetStack(this));
		int idx = panelLayout->indexOf(spacer);
		panelLayout->insertWidget(idx, m_stacks.last());
	}
	m_stacks.last()->addWidget(meas);
}

void MeasurementsPanel::addMeasurement(QWidget *meas) {
	addWidget(meas);
	m_labels.append(meas);
}

void MeasurementsPanel::removeMeasurement(QWidget *meas) {
	m_labels.removeAll(meas);
	update();
}

void MeasurementsPanel::sort() {
	update();
}

void MeasurementsPanel::update() {
	for(VerticalWidgetStack* stack : m_stacks) {
		stack->reparentWidgets(nullptr);
		panelLayout->removeWidget(stack);
		delete stack;
	}
	m_stacks.clear();

	int idx = panelLayout->indexOf(spacer);
	m_stacks.append(new VerticalWidgetStack(this));
	panelLayout->insertWidget(idx, m_stacks.last());

	for(QWidget *label : m_labels) {
		addWidget(label);
	}

}

QWidget *MeasurementsPanel::cursorArea() {
	return m_cursor;
}



StatsPanel::StatsPanel(QWidget *parent)
{
	QVBoxLayout* lay = new QVBoxLayout(this);
	setLayout(lay);

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
