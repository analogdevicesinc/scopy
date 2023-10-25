#ifndef SCOPY_STATUSMESSAGE_H
#define SCOPY_STATUSMESSAGE_H

#include <QLabel>
#include "scopy-pluginbase_export.h"

#define TIMESTAMP_FORMAT "[hh:mm ap] "

namespace scopy {
class StatusMessage : public QWidget
{
public:
	virtual ~StatusMessage() = default;

	virtual QString getText() = 0;
	virtual QWidget *getWidget() = 0;
	virtual int getDisplayTime() = 0;
};

class SCOPY_PLUGINBASE_EXPORT StatusMessageText : public StatusMessage
{
	Q_OBJECT
public:
	StatusMessageText(QString text, int ms = -1, QWidget *parent = nullptr);
	~StatusMessageText();

	QString getText() override;
	QWidget *getWidget() override;
	int getDisplayTime() override;

private:
	void prependDateTime();

	int m_ms;	   // display time
	QString m_text;	   // display text
	QWidget *m_widget; // widget
};

class SCOPY_PLUGINBASE_EXPORT StatusMessageWidget : public StatusMessage
{
	Q_OBJECT
public:
	StatusMessageWidget(QWidget *widget, QString description, int ms = -1, QWidget *parent = nullptr);
	~StatusMessageWidget();

	QString getText() override;
	QWidget *getWidget() override;
	int getDisplayTime() override;

private:
	int m_ms;	   // display time
	QString m_text;	   // display text
	QWidget *m_widget; // widget
};
}

#endif // SCOPY_STATUSMESSAGE_H
