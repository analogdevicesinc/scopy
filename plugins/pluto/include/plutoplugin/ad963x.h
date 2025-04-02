#ifndef AD936X_H
#define AD936X_H

#include "scopy-plutoplugin_export.h"
#include <QBoxLayout>
#include <QWidget>
#include <tooltemplate.h>

namespace scopy::pluto {
class SCOPY_PLUTOPLUGIN_EXPORT AD936X : public QWidget
{
	Q_OBJECT
public:
	AD936X(QWidget *parent = nullptr);
	~AD936X();

private:
	ToolTemplate *m_tool;
	QVBoxLayout *m_mainLayout;
	QWidget *m_controlsWidget;
	QWidget *m_blockDiagramWidget;
};
} // namespace scopy::ad963xplugin
#endif // AD936X_H
