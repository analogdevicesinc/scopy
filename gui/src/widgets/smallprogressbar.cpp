#include "smallprogressbar.h"
#include <style.h>

using namespace scopy;

SmallProgressBar::SmallProgressBar(QWidget *parent)
	: QProgressBar(parent)
	, m_timer(new QTimer(this))
	, m_increment(10)
{
	setTextVisible(false);
	setFixedHeight(1);
	setMinimum(0);
	setMaximum(100);
	setValue(maximum());
	resetBarColor(); // set initial color

	connect(m_timer, &QTimer::timeout, this, [this]() {
		if(value() + m_increment <= maximum()) {
			setValue(value() + m_increment);
		} else {
			setValue(maximum());
			m_timer->stop();
			Q_EMIT progressFinished();
		}
	});
}

void SmallProgressBar::startProgress(int progressDuration, int steps)
{
	setValue(minimum());
	m_increment = maximum() / steps;
	m_timer->start(progressDuration / steps);
}

void SmallProgressBar::setBarColor(QColor color)
{
	QString style = QString("QProgressBar::chunk {background-color: %1;}").arg(color.name());
	setStyleSheet(style);
}

void SmallProgressBar::resetBarColor() { setBarColor(Style::getAttribute(json::theme::content_subtle)); }

#include "moc_smallprogressbar.cpp"

