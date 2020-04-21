#ifndef INFO_WIDGET_H
#define INFO_WIDGET_H

#include <QWidget>

namespace Ui {
class InfoWidget;
}
namespace adiscope {
class InfoWidget : public QWidget {
	Q_OBJECT

public:
	explicit InfoWidget(QWidget *parent = 0);
	~InfoWidget();

private:
	Ui::InfoWidget *ui;
	QTimer *timer;

public Q_SLOTS:
	void enable(bool);
	void setText(QString);

private Q_SLOTS:
	void updatePosition();
};
} // namespace adiscope

#endif // INFO_WIDGET_H
