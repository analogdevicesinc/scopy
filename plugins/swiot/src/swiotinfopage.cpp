#include "swiotinfopage.h"
#include "smallOnOffSwitch.h"
#include <QWidget>

using namespace scopy::swiot;

SwiotInfoPage::SwiotInfoPage(QWidget* parent) : InfoPage(parent)
{
	// not enough attributes for correct display with normal size policies, there is too much spacing without this
	this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}
