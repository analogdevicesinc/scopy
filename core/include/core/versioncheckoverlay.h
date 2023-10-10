#ifndef SCOPY_VERSIONCHECKOVERLAY_H
#define SCOPY_VERSIONCHECKOVERLAY_H

#include <QPushButton>
#include <QWidget>

#include <popupwidget.h>

namespace scopy {
class VersionCheckOverlay : public PopupWidget
{
	Q_OBJECT

public:
	explicit VersionCheckOverlay(QWidget *parent = nullptr);
	~VersionCheckOverlay();
	void showOverlay();
};
} // namespace scopy

#endif // SCOPY_VERSIONCHECKOVERLAY_H
