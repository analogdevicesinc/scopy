#include "infopagekeyvaluewidget.h"

using namespace scopy;
InfoPageKeyValueWidget::InfoPageKeyValueWidget(QString key, QString value, QWidget *parent)
	:

	QWidget(parent)
{
	QHBoxLayout *lay = new QHBoxLayout(this);

	lay->setContentsMargins(0, 0, 0, 0);
	lay->setMargin(0);
	lay->setSpacing(0);

	keyWidget = new QLabel(key, this);
	valueWidget = new QLabel(value, this);
	lay->addWidget(keyWidget);
	lay->addWidget(valueWidget);
	lay->setStretch(0, 1);
	lay->setStretch(1, 3);
}

void InfoPageKeyValueWidget::updateValue(QString value) { valueWidget->setText(value); }
