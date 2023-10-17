#ifndef SCOPY_STATUSMESSAGE_H
#define SCOPY_STATUSMESSAGE_H

#include <QLabel>
#include "scopy-pluginbase_export.h"

class SCOPY_PLUGINBASE_EXPORT StatusMessage : public QWidget
{
	Q_OBJECT
public:
	explicit StatusMessage(QString text, int ms = -1, QWidget *parent = nullptr);
	~StatusMessage();

	QString getText();
	void setText(QString text);

	QWidget *getWidget();
	void setWidget(QWidget *widget);

	int getDisplayTime();
	void setDisplayTime(int ms);

private:
	int m_ms; // display time
	bool m_permanent;
	QString m_text;	   // display text
	QWidget *m_widget; // widget, optional
};

#endif // SCOPY_STATUSMESSAGE_H
