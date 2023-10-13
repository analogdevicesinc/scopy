#ifndef IRECYCLERVIEWADAPTER_HPP
#define IRECYCLERVIEWADAPTER_HPP

#include <QObject>

namespace scopy::regmap {
class IRecyclerViewAdapter : public QObject
{
	Q_OBJECT
public:
	virtual void generateWidget(int index) = 0;
Q_SIGNALS:
	void widgetGenerated(int index, QWidget *widget);
};
} // namespace scopy::regmap
#endif // IRECYCLERVIEWADAPTER_HPP
