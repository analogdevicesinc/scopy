#include "ComboBoxLineEdit.h"
#include <QComboBox>

using namespace adiscope;

ComboBoxLineEdit::ComboBoxLineEdit(QWidget *parent):
	QLineEdit(parent),
	opened(false)
{
}

void ComboBoxLineEdit::mouseReleaseEvent(QMouseEvent *event)
{
	if (!rect().contains(event->localPos().toPoint()))
		return;
	QComboBox *comboBox = dynamic_cast<QComboBox *>(parent());
	if (comboBox){
		if (!opened)
			comboBox->showPopup();
		else
			comboBox->hidePopup();
		opened = !opened;
	}
}
