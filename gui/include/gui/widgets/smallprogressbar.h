#ifndef SCOPY_SMALLPROGRESSBAR_H
#define SCOPY_SMALLPROGRESSBAR_H

#include <QWidget>
#include <QTimer>
#include <QProgressBar>
#include "scopy-gui_export.h"

namespace scopy {
class SCOPY_GUI_EXPORT SmallProgressBar : public QProgressBar
{
	Q_OBJECT
public:
	explicit SmallProgressBar(QWidget *parent = nullptr);

public Q_SLOTS:
	void startProgress(int progressDurationMs = 1000, int steps = 10); // time is 1 second
	void setBarColor(QColor color);
	void resetBarColor();

Q_SIGNALS:
	void progressFinished();

private:
	QTimer *m_timer;
	int m_increment;
};
} // namespace scopy

#endif // SCOPY_SMALLPROGRESSBAR_H
