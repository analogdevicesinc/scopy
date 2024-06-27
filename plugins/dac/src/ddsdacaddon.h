#ifndef DDSDACADDON_H_
#define DDSDACADDON_H_

#include <QWidget>
#include "dacaddon.h"

namespace scopy {
class DacDataModel;
class DdsDacAddon : public DacAddon
{
	Q_OBJECT
public:
	DdsDacAddon(DacDataModel *model, QWidget *parent=nullptr);
	virtual ~DdsDacAddon();

private:
	DacDataModel *m_model;
};
} // namespace scopy

#endif // DDSDACADDON_H_
