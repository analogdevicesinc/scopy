
#ifndef DACDATAMANAGER_H
#define DACDATAMANAGER_H

#include <QWidget>
#include <QList>
#include <QPushButton>
#include <QBoxLayout>
#include <QString>
#include <QColor>

#include <gui/widgets/menucombo.h>
#include <gui/mapstackedwidget.h>
#include <gui/widgets/menucontrolbutton.h>

#include <iio.h>

namespace scopy {
namespace dac {
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

	DacDataManager(struct iio_device *dev, QWidget *parent = nullptr);
	virtual ~DacDataManager();

	QString getName() const;
	QWidget *getWidget();

	QColor getColor() const;
	void setColor(QColor newColor);

	MapStackedWidget *getRightMenuStack() const;
	QList<MenuControlButton *> getMenuControlBtns() const;

	bool isBufferCapable() const;
	bool isDds() const;

	void toggleCyclicBuffer(bool toggled);
	void toggleBufferMode();
	void toggleDdsMode();

private Q_SLOTS:
	void handleChannelMenuRequest(QString uuid);
Q_SIGNALS:
	void requestMenu();

private:
	QVBoxLayout *m_layout;
	DacDataModel *m_model;
	QWidget *m_widget;
	QColor m_color;
	MapStackedWidget *dacAddonStack;
	MapStackedWidget *rightMenuStack;
	MenuCombo *m_mode;
	QList<MenuControlButton *> m_menuControlBtns;

	void setupDdsDac();
	void setupDacMode(QString mode_name, unsigned int mode);
	QWidget *createMenu();
	QWidget *createAttrMenu(QWidget *parent);
};
} // namespace dac
} // namespace scopy

#endif // DACDATAMANAGER_H
