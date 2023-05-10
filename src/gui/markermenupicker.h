#ifndef MARKERMENUPICKER_H
#define MARKERMENUPICKER_H

#include <QWidget>

namespace Ui {
class MarkerMenuPicker;
}

class MarkerMenuPicker : public QWidget
{
	Q_OBJECT

public:
	explicit MarkerMenuPicker(QWidget *parent = nullptr);
	~MarkerMenuPicker();

private:
	Ui::MarkerMenuPicker *ui;
};

#endif // MARKERMENUPICKER_H
