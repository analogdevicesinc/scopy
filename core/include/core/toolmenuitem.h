#ifndef TOOLMENUITEM_H
#define TOOLMENUITEM_H

#include <QPushButton>
#include <QWidget>
#include <customPushButton.h>

namespace scopy {
class ToolMenuItem : public QWidget
{
	Q_OBJECT
public:
	ToolMenuItem(QString uuid, QString name, QString icon, QWidget *parent = nullptr);
	~ToolMenuItem();

	QPushButton *getToolBtn() const;
	QPushButton *getToolRunBtn() const;

	void enableDoubleClick(bool enable);
	bool eventFilter(QObject *watched, QEvent *event);

	void setName(QString str);
	void setSelected(bool en);

	QString getId() const;
Q_SIGNALS:
	void doubleclick();

public Q_SLOTS:
	void setDisabled(bool disabled);
	void updateItem();

protected:
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);

private:
	QPushButton *m_toolBtn;
	CustomPushButton *m_toolRunBtn;

	QString m_uuid;
	QString m_name;
	QString m_icon;
};
} // namespace scopy

#endif // TOOLMENUITEM_H
