#ifndef DRAGZONE_H
#define DRAGZONE_H

#include <QWidget>
#include <QMouseEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>

namespace Ui {
class DragZone;
}

namespace adiscope{
class DragZone : public QWidget
{
	Q_OBJECT

public:
	explicit DragZone(QWidget *parent = 0);
	~DragZone();

	bool eventFilter(QObject *watched, QEvent *event);

	int getPosition() const;
	void setPosition(int value);

Q_SIGNALS:
	void requestPositionChange(int, int, bool);
	void highlightLastSeparator(bool);

private Q_SLOTS:
	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dragLeaveEvent(QDragLeaveEvent *event);
	void dropEvent(QDropEvent *event);

private:
	Ui::DragZone *ui;
	int position;
};
}
#endif // DRAGZONE_H
