#ifndef PLUGINENABLEWIDGET_H
#define PLUGINENABLEWIDGET_H

#include <qwidget.h>
#include "qcheckbox.h"
#include "qlabel.h"

namespace scopy {

class PluginEnableWidget : public QWidget {
	Q_OBJECT
public:
	explicit PluginEnableWidget(QWidget *parent = nullptr);
	~PluginEnableWidget();

	void paintEvent(QPaintEvent *e) override;
	void setDescription(QString description);
	QCheckBox *checkBox() const;

private:
	QCheckBox *m_checkBox;
	QLabel *m_descriptionLabel;
};

}


#endif // PLUGINENABLEWIDGET_H
