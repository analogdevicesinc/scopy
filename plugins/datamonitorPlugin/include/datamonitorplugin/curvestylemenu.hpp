#ifndef CURVESTYLEMENU_HPP
#define CURVESTYLEMENU_HPP

#include <QWidget>
#include <plotchannel.h>

namespace scopy {
class CurveStyleMenu : public QWidget
{
	Q_OBJECT
public:
	explicit CurveStyleMenu(QWidget *parent = nullptr);

Q_SIGNALS:
	void curveThicknessChanged(double thickness);
	void curveStyleIndexChanged(int index);

private:
};
} // namespace scopy
#endif // CURVESTYLEMENU_HPP
