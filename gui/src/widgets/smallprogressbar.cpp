#include "smallprogressbar.h"

using namespace scopy;

SmallProgressBar::SmallProgressBar(QWidget *parent)
	: QProgressBar(parent)
	, m_timer(new QTimer(this))
	, m_increment(10)
{
	setTextVisible(false);
	setFixedHeight(3);
	setMinimum(0);
	setMaximum(100);

	connect(m_timer, &QTimer::timeout, this, [this]() {
		if(value() + m_increment <= maximum()) {
			setValue(value() + m_increment);
		} else {
			setValue(minimum());
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

#include "moc_smallprogressbar.cpp"
