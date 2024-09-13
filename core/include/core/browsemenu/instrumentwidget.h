#ifndef INSTRUMENTWIDGET_H
#define INSTRUMENTWIDGET_H

#include <QPushButton>
#include <QWidget>
#include <customPushButton.h>

namespace scopy {
class InstrumentWidget : public QWidget
{
	Q_OBJECT
public:
	InstrumentWidget(QString uuid, QString name, QString icon, QWidget *parent = nullptr);
	~InstrumentWidget();

	QPushButton *getToolBtn() const;
	QPushButton *getToolRunBtn() const;

	void setToolEnabled(bool disabled);
	void enableDoubleClick(bool enable);
	void setSeparator(bool top, bool bot);

	bool eventFilter(QObject *watched, QEvent *event);

	void setName(QString str);
	void hideText(bool hidden);

	QString getId() const;

	void setSelected(bool en);

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

#endif // INSTRUMENTWIDGET_H
