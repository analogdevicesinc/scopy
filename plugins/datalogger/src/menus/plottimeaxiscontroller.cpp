#include "menus/plottimeaxiscontroller.hpp"

#include <QDateTimeEdit>
#include <datamonitorutils.hpp>
#include <menucollapsesection.h>
#include <menuonoffswitch.h>
#include <menusectionwidget.h>
#include <timemanager.hpp>

using namespace scopy;
using namespace datamonitor;

PlotTimeAxisController::PlotTimeAxisController(MonitorPlot *m_plot, QWidget *parent)
	: m_plot(m_plot)
	, QWidget{parent}
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(10);
	this->setLayout(layout);

	MenuSectionWidget *xAxisContainer = new MenuSectionWidget(parent);
	MenuCollapseSection *xAxisSection =
		new MenuCollapseSection("X-AXIS", MenuCollapseSection::MHCW_NONE, xAxisContainer);

	xAxisContainer->contentLayout()->addWidget(xAxisSection);
	xAxisSection->contentLayout()->setSpacing(10);

	realTimeToggle = new MenuOnOffSwitch(tr("Use UTC date and time"), xAxisSection, false);
	livePlottingToggle = new MenuOnOffSwitch(tr("Live plotting"), xAxisContainer, false);
	livePlottingToggle->onOffswitch()->setChecked(true);

	dateEdit = new QDateEdit(QDate::currentDate(), xAxisContainer);
	dateEdit->setCalendarPopup(true);
	dateEdit->setVisible(false);
	timeEdit = new QTimeEdit(QTime::currentTime(), xAxisContainer);
	timeEdit->setDisplayFormat("hh:mm:ss");
	timeEdit->setVisible(false);

	m_xdelta = new PositionSpinButton(
		{
			{"s", 1},
			{"min", 60},
			{"hour", 3600},
		},
		"Delta", (double)((long)(-1 << 31)), (double)((long)1 << 31), false, false, xAxisContainer);
	m_xdelta->setValue(DataMonitorUtils::getAxisDefaultMaxValue());

	auto &&timeTracker = TimeManager::GetInstance();

	connect(timeTracker, &TimeManager::timeout, this, [=, this]() {
		if(livePlottingToggle->onOffswitch()->isChecked()) {
			// plot using current date time as starting point
			timeEdit->setTime(QTime::currentTime());
			dateEdit->setDate(QDate::currentDate());
			updatePlotStartPoint();
		}
	});

	connect(realTimeToggle->onOffswitch(), &QAbstractButton::toggled, m_plot, &MonitorPlot::setIsRealTime);
	connect(livePlottingToggle->onOffswitch(), &QAbstractButton::toggled, this,
		&PlotTimeAxisController::togglePlotNow);

	connect(dateEdit, &QDateEdit::dateChanged, this, &PlotTimeAxisController::updatePlotStartPoint);
	connect(timeEdit, &QTimeEdit::timeChanged, this, &PlotTimeAxisController::updatePlotStartPoint);

	connect(m_xdelta, &PositionSpinButton::valueChanged, this,
		[=, this](double value) { m_plot->updateXAxisIntervalMax(value); });

	xAxisSection->contentLayout()->addWidget(realTimeToggle);
	xAxisSection->contentLayout()->addWidget(livePlottingToggle);
	xAxisSection->contentLayout()->addWidget(dateEdit);
	xAxisSection->contentLayout()->addWidget(timeEdit);
	xAxisSection->contentLayout()->addWidget(m_xdelta);

	layout->addWidget(xAxisContainer);
}

void PlotTimeAxisController::togglePlotNow(bool toggled)
{
	dateEdit->setVisible(!toggled);
	timeEdit->setVisible(!toggled);
	updatePlotStartPoint();
}

void PlotTimeAxisController::updatePlotStartPoint()
{
	double time = QwtDate::toDouble(QDateTime(dateEdit->date(), timeEdit->time()));
	double delta = m_xdelta->value();

	m_plot->updatePlotStartingPoint(time, delta);
}
