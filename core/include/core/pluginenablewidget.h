#ifndef PLUGINENABLEWIDGET_H
#define PLUGINENABLEWIDGET_H

#include "gui/utils.h"
#include "qcheckbox.h"
#include "qlabel.h"

#include <qwidget.h>

namespace scopy {

class PluginEnableWidget : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	explicit PluginEnableWidget(QWidget *parent = nullptr);
	~PluginEnableWidget();

	void setDescription(QString description);
	QCheckBox *checkBox() const;

private:
	QCheckBox *m_checkBox;
	QLabel *m_descriptionLabel;
};

} // namespace scopy

#endif // PLUGINENABLEWIDGET_H
