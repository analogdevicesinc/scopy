#ifndef SCOPY_SWIOTINFOPAGE_H
#define SCOPY_SWIOTINFOPAGE_H

#include <QWidget>
#include <infopage.h>

namespace scopy::swiot {
class SwiotInfoPage : public InfoPage {
	Q_OBJECT
public:
	explicit SwiotInfoPage(QWidget* parent = nullptr);
};
}

#endif //SCOPY_SWIOTINFOPAGE_H
