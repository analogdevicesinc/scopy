#ifndef SCOPY_VERSIONCHECKMESSAGE_H
#define SCOPY_VERSIONCHECKMESSAGE_H

#include <QPushButton>
#include <QWidget>

#include <popupwidget.h>

namespace scopy {
class VersionCheckMessage : public QWidget
{
	Q_OBJECT

public:
	explicit VersionCheckMessage(QWidget *parent = nullptr);
	~VersionCheckMessage();

Q_SIGNALS:
	void setCheckVersion(bool allowed);

private Q_SLOTS:
	void saveCheckVersion(bool allowed);
};
} // namespace scopy

#endif // SCOPY_VERSIONCHECKMESSAGE_H
