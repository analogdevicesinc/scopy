#ifndef TITLESPINBOX_H
#define TITLESPINBOX_H

#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>

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

#endif // TITLESPINBOX_H
