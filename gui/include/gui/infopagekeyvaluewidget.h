#ifndef INFOPAGEKEYVALUEWIDGET_H
#define INFOPAGEKEYVALUEWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include "scopy-gui_export.h"

namespace scopy {
class SCOPY_GUI_EXPORT InfoPageKeyValueWidget : public QWidget {
public:
	InfoPageKeyValueWidget(QString key, QString value, QWidget *parent = nullptr );
	void updateValue(QString value);

	QLabel *keyWidget;
	QLabel *valueWidget;
};
}
#endif // INFOPAGEKEYVALUEWIDGET_H
