#include "movingaverage.h"

MovingAverage::MovingAverage()
{
	sum = 0;
}

int MovingAverage::getCapacity() const
{
	return capacity;
}

void MovingAverage::setCapacity(int newCapacity)
{
	capacity = newCapacity;
}

const QVector<double> &MovingAverage::getHistory() const
{
	return history;
}

double MovingAverage::pushValueReturnAverage(double newVal){
	sum+=newVal;
	if(history.count() == capacity && capacity > 0) {
		sum-=history.front();
		history.pop_front();
	}
	history.push_back(newVal);
	return (sum/(double)history.count());
}

void MovingAverage::clearHistory() {
	history.clear();
	sum = 0;
}
