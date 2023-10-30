#ifndef SCOPY_TRIGGERMENUSECTION_H
#define SCOPY_TRIGGERMENUSECTION_H

#include <QWidget>
#include "scopy-gui_export.h"
#include "utils.h"
#include "menucombo.h"
#include "menusectionwidget.h"

namespace scopy {
class SCOPY_GUI_EXPORT TriggerMenuSection : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	explicit TriggerMenuSection(QWidget *parent = nullptr);
	void selectTrigger(const QString &trigger);

public Q_SLOTS:
	void addTrigger(const QString &trigger);

Q_SIGNALS:
	void selectedTrigger(QString trigger);

private:
	void setupUi();

	MenuComboWidget *m_menuComboWidget;
	MenuSectionWidget *m_menuSectionWidget;
};
} // namespace scopy

#endif // SCOPY_TRIGGERMENUSECTION_H
