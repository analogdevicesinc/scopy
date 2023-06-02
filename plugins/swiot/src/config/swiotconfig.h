#ifndef SWIOTCONFIG_H
#define SWIOTCONFIG_H

#define AD_NAME "ad74413r"
#define MAX_NAME "max14906"

#include "configcontroller.h"
//#include "configmenu.h"
#include "ui_swiotconfig.h"

#include <gui/tool_view.hpp>
#include <QPushButton>
#include <QMap>
#include <QScrollArea>

#include "src/config/drawarea.h"

extern "C"{
struct iio_context;
struct iio_device;
struct iio_channel;
}

namespace scopy::swiot {

class SwiotConfig: public QWidget
{
	Q_OBJECT
public:
	explicit SwiotConfig(struct iio_context *ctx, QWidget *parent = nullptr);
	~SwiotConfig();

public Q_SLOTS:
	void configBtnPressed();
	void externalPowerSupply(bool ps);

Q_SIGNALS:
	void configBtn();

private:
	QMap<QString, struct iio_device*> m_iioDevices;
	struct iio_context* m_context;
	struct iio_device* m_swiotDevice;

	QVector<ConfigController*> m_controllers;
	QVector<QStringList*> m_funcAvailable;
	QPushButton* m_configBtn;
	scopy::gui::ToolView* m_toolView;

	DrawArea* m_drawArea;
	QScrollArea* m_scrollArea;
	QWidget* m_mainView;
	QLabel* m_statusLabel;
	Ui::ConfigMenu* ui;

        void setupToolView(QWidget* parent);
	void init();
        void createPageLayout();
	void setDevices(struct iio_context* ctx);
	static QPushButton* createConfigBtn();
};
}

#endif // SWIOTCONFIG_H
