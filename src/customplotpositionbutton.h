#ifndef CUSTOMPLOTPOSITIONBUTTON_H
#define CUSTOMPLOTPOSITIONBUTTON_H

#include "coloredQWidget.hpp"

#include <QButtonGroup>
#include <QWidget>

namespace Ui {
class CustomPlotPositionButton;
}

namespace adiscope {
class CustomPlotPositionButton : public ColoredQWidget {
	Q_OBJECT

public:
	enum ReadoutsPosition { topLeft, topRight, bottomLeft, bottomRight };

	explicit CustomPlotPositionButton(QWidget *parent = 0);
	~CustomPlotPositionButton();
	void setPosition(ReadoutsPosition position);

Q_SIGNALS:
	void
	positionChanged(CustomPlotPositionButton::ReadoutsPosition position);

private:
	Ui::CustomPlotPositionButton *ui;
	QButtonGroup *btns;
};
} // namespace adiscope

#endif // CUSTOMPLOTPOSITIONBUTTON_H
