#ifndef SCOPY_ATTRWIDGET_H
#define SCOPY_ATTRWIDGET_H

#include <QWidget>
#include "attrdata.h"
#include "utils.h"
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menusectionwidget.h>
#include "guistrategy/guistrategyinterface.h"
#include "savestrategy/savestrategyinterface.h"
#include "datastrategy/datastrategyinterface.h"
#include <scopy-gr-util_export.h>

namespace scopy {
namespace attr {
class AttrUiStrategyInterface;
class SaveStrategyInterface;
class DataStrategyInterface;
} // namespace attr

class SCOPY_GR_UTIL_EXPORT AttrWidget : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	AttrWidget(QString title, attr::AttrUiStrategyInterface *uiStrategy, attr::SaveStrategyInterface *saveStrategy,
		   attr::DataStrategyInterface *dataStrategy, QWidget *parent = nullptr);

	attr::SaveStrategyInterface *getSaveStrategy();
	attr::AttrUiStrategyInterface *getUiStrategy();
	attr::DataStrategyInterface *getDataStrategy();

protected:
	attr::SaveStrategyInterface *m_saveStrategy;
	attr::AttrUiStrategyInterface *m_uiStrategy;
	attr::DataStrategyInterface *m_dataStrategy;

	MenuCollapseSection *m_collapseSection;
	MenuSectionWidget *m_menuSectionWidget;
};
} // namespace scopy

#endif // SCOPY_ATTRWIDGET_H
