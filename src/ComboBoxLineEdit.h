#ifndef ComboBoxLineEdit_H
#define ComboBoxLineEdit_H

#include <QLineEdit>
#include <QMouseEvent>

namespace adiscope {
class ComboBoxLineEdit : public QLineEdit
{
	Q_OBJECT
public:
	ComboBoxLineEdit(QWidget *parent = 0);

protected:
	void mouseReleaseEvent(QMouseEvent *event);

private:
	bool opened;
};
}

#endif // ComboBoxLineEdit_H
