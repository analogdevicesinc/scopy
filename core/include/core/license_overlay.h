#ifndef LICENSEOVERLAY_H
#define LICENSEOVERLAY_H

#include "qpushbutton.h"
#include "scopy-core_export.h"
#include "tintedoverlay.h"

#include <QMainWindow>

namespace Ui {
class LicenseDialogOverlay;
}
namespace scopy {
class SCOPY_CORE_EXPORT LicenseOverlay : public QWidget
{
	Q_OBJECT
public:
	LicenseOverlay(QWidget *parent = nullptr);
	QString static getLicense();
	void showOverlay();
	QPushButton *getContinueBtn();

private:
	QWidget *parent;
	gui::TintedOverlay *overlay;
	Ui::LicenseDialogOverlay *ui;
};
} // namespace scopy
#endif // LICENSEOVERLAY_H
