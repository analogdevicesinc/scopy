#ifndef DACADDON_H_
#define DACADDON_H_

#include <QWidget>

namespace scopy {
class DacAddon : public QWidget
{
	Q_OBJECT
public:
	DacAddon(QString color, QWidget *parent=nullptr);
	virtual ~DacAddon();
};
} // namespace scopy

#endif // DACADDON_H_
