#ifndef TITLESPINBOX_H
#define TITLESPINBOX_H

#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>
namespace scopy {
namespace regmap {
class TitleSpinBox : public QWidget
{
	Q_OBJECT

public:
	explicit TitleSpinBox(QString title, QWidget *parent = nullptr);
	~TitleSpinBox();

	void setTitle(QString title);
	QPushButton *getSpinBoxUpButton();
	QPushButton *getSpinBoxDownButton();
	QSpinBox *getSpinBox();

private:
	QPushButton *spinBoxUpButton;
	QPushButton *spinBoxDownButton;
	QLabel *titleLabel;
	QSpinBox *spinBox = nullptr;
};
} // namespace regmap
} // namespace scopy
#endif // TITLESPINBOX_H
