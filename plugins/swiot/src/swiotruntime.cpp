#include "swiotruntime.hpp"
#include <QtConcurrent>
#include <QFuture>
#include <string>
#include <core/logging_categories.h>

using namespace adiscope;

SwiotRuntime::SwiotRuntime(struct iio_context *ctx, Filter *filt,
			     ToolMenuItem *toolMenuItem,
			     QJSEngine *engine, ToolLauncher *parent):
	Tool(ctx, toolMenuItem, nullptr, "SwiotRuntime",parent),
	m_toolView(nullptr)
{
	run_button=nullptr;
	int devicesNumber = iio_context_get_devices_count(ctx);
	m_backBtn = createBackBtn();
	for (int i = 0; i < devicesNumber; i++) {
		struct iio_device* iioDev = iio_context_get_device(ctx, i);
		if (iioDev) {
			QString deviceName = QString(iio_device_get_name(iioDev));
			m_iioDevices[deviceName] = iioDev;
		}
	}

	bool isRuntime = isRuntimeCtx();
	if (isRuntime) {
		qInfo(CAT_SWIOT_RUNTIME) <<"runtime context";
		m_swiotRuntimeAd = new SwiotAd(this, parent, m_iioDevices[AD_NAME], channel_function);
		initAdToolView();
		connect(m_backBtn, &QPushButton::pressed, this, &SwiotRuntime::backBtnPressed);
	} else {
		qDebug(CAT_SWIOT_RUNTIME) <<"isn't runtime context";
	}
}

SwiotRuntime::~SwiotRuntime()
{
	if (m_toolView) {
		if (m_toolView->getRunBtn()->isChecked()) {
			m_toolView->getRunBtn()->setChecked(false);
		}
		delete m_toolView;
		m_toolView = nullptr;
	}
	if (m_swiotRuntimeAd) {
		delete m_swiotRuntimeAd;
		m_swiotRuntimeAd = nullptr;
	}

}

void SwiotRuntime::initAdToolView()
{
	setToolView(m_swiotRuntimeAd->getToolView());
	m_toolView->addTopExtraWidget(m_backBtn);
	setCentralWidget(m_toolView);
	qInfo(CAT_SWIOT_RUNTIME) << "Ad initialized";
}

adiscope::gui::ToolView* SwiotRuntime::getToolView()
{
	return m_toolView;
}

void SwiotRuntime::setToolView(adiscope::gui::ToolView* toolView)
{
	m_toolView = toolView;
}

bool SwiotRuntime::isRuntimeCtx()
{
	bool runtime = false;
	for (const auto &key : m_iioDevices.keys()) {
		if (iio_device_find_attr(m_iioDevices[key], "back")) {
			runtime = true;
			break;
		}
	}
	return runtime;
}

QPushButton* SwiotRuntime::createBackBtn()
{
	QPushButton* backBtn = new QPushButton();
	backBtn->setObjectName(QString::fromUtf8("backBtn"));
	backBtn->setStyleSheet(QString::fromUtf8("QPushButton{\n"
"  width: 95px;\n"
"  height: 40px;\n"
"\n"
"  font-size: 12px;\n"
"  text-align: center;\n"
"  font-weight: bold;\n"
"  padding-left: 15px;\n"
"  padding-right: 15px;\n"
"}"));
	backBtn->setProperty("blue_button", QVariant(true));
	backBtn->setText("Back");
	return backBtn;
}

void SwiotRuntime::backBtnPressed()
{
	for (const auto &key : m_iioDevices.keys()) {
		if (iio_device_find_attr(m_iioDevices[key], "back")) {
			iio_device_attr_write_bool(m_iioDevices[key], "back", 1);
		}
	}
}
