#ifndef SEMIEXCLUSIVEBUTTONGROUP_H
#define SEMIEXCLUSIVEBUTTONGROUP_H

#include "scopy-gui_export.h"

#include <QAbstractButton>
#include <QButtonGroup>

namespace scopy {
class SCOPY_GUI_EXPORT SemiExclusiveButtonGroup : public QButtonGroup
{
	Q_OBJECT

public:
	explicit SemiExclusiveButtonGroup(QObject *parent = nullptr);

	void setExclusive(bool bExclusive);
	bool exclusive() const;

	QAbstractButton *getSelectedButton();
	QAbstractButton *getLastButton();

Q_SIGNALS:
	void buttonSelected(QAbstractButton *btn);

protected Q_SLOTS:
	void buttonClicked(QAbstractButton *button);

protected:
	bool m_exclusive;
	QAbstractButton *m_lastBtn;
};
} // namespace scopy
#endif // SEMIEXCLUSIVEBUTTONGROUP_H
