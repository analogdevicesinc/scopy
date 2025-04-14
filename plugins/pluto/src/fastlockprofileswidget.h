#ifndef FASTLOCKPROFILESWIDGET_H
#define FASTLOCKPROFILESWIDGET_H

#include <QWidget>
#include "scopy-plutoplugin_export.h"

#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <iio.h>

namespace scopy {
namespace pluto {
class SCOPY_PLUTOPLUGIN_EXPORT FastlockProfilesWidget : public QWidget
{
	Q_OBJECT
public:
	explicit FastlockProfilesWidget(iio_channel *chn, QWidget *parent = nullptr);

	QComboBox *fastlockProfiles() const;

	QString title() const;
	void setTitle(QString *newTitle);

Q_SIGNALS:
	void recallCalled();

private:
	QLabel *m_title;
	QComboBox *m_fastlockProfiles;
	QPushButton *m_storeBtn;
	QPushButton *m_recallBtn;
};
} // namespace pluto
} // namespace scopy
#endif // FASTLOCKPROFILESWIDGET_H
