#ifndef SWIOTCONFIG_HPP
#define SWIOTCONFIG_HPP
#define AD_NAME "ad74413r"
#define MAX_NAME "max14906"

#include "configcontroller.hpp"
#include "configmenu.hpp"
#include "src/refactoring/tool/tool_view.hpp"
#include <QPushButton>
#include <QMap>
#include <QScrollArea>

#include "src/config/drawarea.h"

extern "C"{
	struct iio_context;
	struct iio_device;
	struct iio_channel;
}

namespace adiscope::swiot {

class SwiotConfig: public QWidget
{
public:
	explicit SwiotConfig(struct iio_context *ctx, QWidget *parent = nullptr);
	~SwiotConfig();

public Q_SLOTS:
	void configBtnPressed();

private:
	QMap<QString, struct iio_device*> m_iioDevices;
	QVector<ConfigController*> m_controllers;
	QVector<QStringList*> m_funcAvailable;
	QPushButton* m_configBtn;
	ConfigMenu* m_configMenu;
        adiscope::gui::ToolView* m_toolView;

        DrawArea* m_drawArea;
        QScrollArea* m_scrollArea;

protected:
        bool eventFilter(QObject *object, QEvent * event) override;

private:
	void init();
	bool isConfigCtx();
	void setDevices(struct iio_context* ctx);
	static QPushButton* createConfigBtn();
};
}


#endif // SWIOTCONFIG_HPP
