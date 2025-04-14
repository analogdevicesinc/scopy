#ifndef AD936X_H
#define AD936X_H

#include "scopy-plutoplugin_export.h"
#include <QBoxLayout>
#include <QWidget>
#include <tooltemplate.h>

#include <iio-widgets/iiowidgetbuilder.h>

namespace scopy {
namespace pluto {
class SCOPY_PLUTOPLUGIN_EXPORT AD936X : public QWidget
{
	Q_OBJECT
public:
	AD936X(QString uri, QWidget *parent = nullptr);
	~AD936X();

private:
	QString m_uri;
	ToolTemplate *m_tool;
	QVBoxLayout *m_mainLayout;
	QWidget *m_controlsWidget;
	QWidget *m_blockDiagramWidget;

	QWidget *generateGlobalSettingsWidget(QWidget *parent);

	QWidget *generateRxWidget(QWidget *parent);

	QWidget *generateTxWidget(QWidget *parent);
};
} // namespace pluto
} // namespace scopy
#endif // AD936X_H
