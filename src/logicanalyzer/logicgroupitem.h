#ifndef LOGICGROUPITEM_H
#define LOGICGROUPITEM_H

#include "basemenuitem.h"

#include <QLabel>

namespace adiscope {
class LogicGroupItem : public BaseMenuItem
{
	Q_OBJECT
public:
	LogicGroupItem(const QString &name, QWidget *parent = nullptr);

	QString getName() const;
	void setName(const QString &name);

Q_SIGNALS:
	void deleteBtnClicked();

private:
	void buildUi();

private:
	QLabel *m_nameLabel;
};
}

#endif // LOGICGROUPITEM_H
