#ifndef DETACHDRAGZONE_H
#define DETACHDRAGZONE_H

#include <QWidget>
#include <QMouseEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QString>

namespace adiscope {
class DetachDragZone : public QWidget
{
	Q_OBJECT

public:
	explicit DetachDragZone(QWidget *parent = 0);
	~DetachDragZone();

	bool eventFilter(QObject *watched, QEvent *event);

Q_SIGNALS:
	void detachWidget(int);
	void changeText(QString);

private Q_SLOTS:
	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dragLeaveEvent(QDragLeaveEvent *event);
	void dropEvent(QDropEvent *event);


};
}

#endif // DETACHDRAGZONE_H
