#ifndef ComboBoxLineEdit_H
#define ComboBoxLineEdit_H

#include <QLineEdit>
#include <QMouseEvent>

namespace adiscope {
class ComboBoxLineEdit : public QLineEdit {
	Q_OBJECT
public:
	ComboBoxLineEdit(QWidget *parent = 0);

protected Q_SLOTS:
	void mouseReleaseEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
};
} // namespace adiscope

#endif // ComboBoxLineEdit_H
