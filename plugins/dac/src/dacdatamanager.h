
#ifndef DACDATAMANAGER_H
#define DACDATAMANAGER_H

#include <QWidget>
#include <QBoxLayout>
#include <QString>
#include <QColor>

#include <mapstackedwidget.h>

#include <iio.h>

namespace scopy {
class MenuCombo;
class DacDataModel;
class DacAddon;
class DacDataManager : public QWidget
{
	Q_OBJECT
public:
	typedef enum
	{
		DAC_DISABLED,
		DAC_BUFFER,
		DAC_DDS
	} DacMode;

	DacDataManager(DacDataModel *model, QWidget *parent = nullptr);
	virtual ~DacDataManager();

	QString getName() const;
	QWidget *getWidget();

	QColor getColor() const;
	void setColor(QColor newColor);
private:
	struct iio_device *m_dev;
	QHBoxLayout *m_layout;
	DacDataModel *m_model;
	QWidget *m_widget;
	QColor m_color;
	MapStackedWidget *dacAddonStack;
	QList<DacAddon*> m_dacAddons;
	MenuCombo *m_mode;

	void setupBufferDac();
	void setupDdsDac();
	void setupDacMode(QString mode_name, unsigned int mode);
};

} // namespace scopy

#endif // DACDATAMANAGER_H
