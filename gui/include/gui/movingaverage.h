#ifndef MOVINGAVERAGE_H
#define MOVINGAVERAGE_H

#include <QVector>
#include "scopy-gui_export.h"

class SCOPY_GUI_EXPORT MovingAverage
{
public:
	MovingAverage();
	double pushValueReturnAverage(double newVal);

	int getCapacity() const;
	void setCapacity(int newCapacity);
	void clearHistory();
	const QVector<double> &getHistory() const;

private:
	QVector<double> history;
	int capacity;
	double sum;

};

#endif // MOVINGAVERAGE_H
