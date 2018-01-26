#ifndef HOMEPAGE_CONTROLS_H
#define HOMEPAGE_CONTROLS_H

#include <QWidget>
#include <QString>

namespace Ui {
class HomepageControls;
}

namespace adiscope {
class HomepageControls : public QWidget
{
	Q_OBJECT

public:
	explicit HomepageControls(QWidget *parent = 0);
	~HomepageControls();

	void updatePosition();
	virtual bool eventFilter(QObject *, QEvent *);

Q_SIGNALS:
	void goLeft();
	void goRight();
	void openFile();

private:
	Ui::HomepageControls *ui;
};
}
#endif // HOMEPAGE_CONTROLS_H
