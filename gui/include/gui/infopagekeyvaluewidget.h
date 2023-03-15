#ifndef INFOPAGEKEYVALUEWIDGET_H
#define INFOPAGEKEYVALUEWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include "scopygui_export.h"

namespace adiscope {
class SCOPYGUI_EXPORT InfoPageKeyValueWidget : public QWidget {
public:
	InfoPageKeyValueWidget(QString key, QString value, QWidget *parent = nullptr );
	void updateValue(QString value);

	QLabel *keyWidget;
	QLabel *valueWidget;
};
}
#endif // INFOPAGEKEYVALUEWIDGET_H
