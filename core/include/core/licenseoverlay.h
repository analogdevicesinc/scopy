#ifndef LICENSEOVERLAY_H
#define LICENSEOVERLAY_H

#include "qpushbutton.h"
#include "scopy-core_export.h"
#include <widgets/popupwidget.h>

namespace scopy {
class SCOPY_CORE_EXPORT LicenseOverlay : public QWidget
{
	Q_OBJECT
public:
	LicenseOverlay(QWidget *parent = nullptr);
	~LicenseOverlay();
	QString static getLicense();
	void showOverlay();
	QPushButton *getContinueBtn();

private:
	QWidget *parent;
	PopupWidget *m_popupWidget;
};
} // namespace scopy
#endif // LICENSEOVERLAY_H
