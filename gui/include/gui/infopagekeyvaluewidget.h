#ifndef INFOPAGEKEYVALUEWIDGET_H
#define INFOPAGEKEYVALUEWIDGET_H

#include "scopy-gui_export.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

namespace scopy {
class SCOPY_GUI_EXPORT InfoPageKeyValueWidget : public QWidget
{
public:
	InfoPageKeyValueWidget(QString key, QString value, QWidget *parent = nullptr);
	void updateValue(QString value);

	QLabel *keyWidget;
	QLabel *valueWidget;
};
} // namespace scopy
#endif // INFOPAGEKEYVALUEWIDGET_H
