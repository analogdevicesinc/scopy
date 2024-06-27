#ifndef BUFFERDACADDON_H_
#define BUFFERDACADDON_H_

#include <QWidget>
#include "dacaddon.h"

namespace scopy {
class DacDataModel;
class BufferDacAddon : public DacAddon
{
	Q_OBJECT
public:
	BufferDacAddon(DacDataModel *model, QWidget *parent=nullptr);
	virtual ~BufferDacAddon();

private:
	DacDataModel *m_model;
};
} // namespace scopy

#endif // BUFFERDACADDON_H_
