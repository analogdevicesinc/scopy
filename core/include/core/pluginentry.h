#ifndef PLUGINENTRY_H
#define PLUGINENTRY_H

#include <qwidget.h>
#include "qcheckbox.h"
#include "qlabel.h"

namespace scopy {

class PluginEntry : public QWidget {
	Q_OBJECT
public:
	explicit PluginEntry(QWidget *parent = nullptr);
	~PluginEntry();

	void paintEvent(QPaintEvent *e) override;
	void setDescription(QString description);
	QCheckBox *checkBox() const;

private:
	QCheckBox *m_checkBox;
	QLabel *m_descriptionLabel;
};

}


#endif // PLUGINENTRY_H
