#include "swiotruntime.hpp"
#include <string>
#include <core/logging_categories.h>

using namespace adiscope;

SwiotRuntime::SwiotRuntime(struct iio_context *ctx, QWidget *parent):
	QWidget(parent),
	m_swiotRuntimeAd(nullptr), m_toolView(nullptr), m_triggerTimer(nullptr)
{
	m_backBtn = createBackBtn();
	createDevicesMap(ctx);

	bool isRuntime = isRuntimeCtx();
	if (isRuntime) {
		qInfo(CAT_SWIOT_RUNTIME) <<"runtime context";
		if (m_iioDevices[AD_TRIGGER_NAME]) {
			int retCode = iio_device_set_trigger(m_iioDevices[AD_NAME], m_iioDevices[AD_TRIGGER_NAME]);
			qDebug(CAT_SWIOT_RUNTIME) <<"Trigger has been set: " + QString::number(retCode);
		}

		m_swiotRuntimeAd = new SwiotAd(this, m_iioDevices[AD_NAME], channel_function);
		initAdToolView();

		m_triggerTimer = new QTimer();
		m_triggerTimer->start(TRIGGER_TIMER_MS);
		connect(m_backBtn, &QPushButton::pressed, this, &SwiotRuntime::onBackBtnPressed);
		connect(m_triggerTimer, SIGNAL(timeout()), this, SLOT(triggerPing()));
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
	if (m_triggerTimer) {
		m_triggerTimer->stop();
		delete m_triggerTimer;
		m_triggerTimer = nullptr;
	}

}

void SwiotRuntime::initAdToolView()
{
	setToolView(m_swiotRuntimeAd->getToolView());
	m_toolView->addTopExtraWidget(m_backBtn);
//	setCentralWidget(m_toolView);
	this->setLayout(new QVBoxLayout());
	this->layout()->addWidget(m_toolView);
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

void SwiotRuntime::onBackBtnPressed()
{
	for (const auto &key : m_iioDevices.keys()) {
		if (iio_device_find_attr(m_iioDevices[key], "back")) {
			iio_device_attr_write_bool(m_iioDevices[key], "back", 1);
		}
	}

	Q_EMIT backBtnPressed();
}

void SwiotRuntime::triggerPing()
{
	if (m_iioDevices[AD_NAME]) {
		const iio_device* triggerDevice = nullptr;
		int ret = iio_device_get_trigger(m_iioDevices[AD_NAME], &triggerDevice);

		if (ret < 0 && m_iioDevices.contains(AD_TRIGGER_NAME)) {
			iio_device_set_trigger(m_iioDevices[AD_NAME], m_iioDevices[AD_TRIGGER_NAME]);
			qDebug(CAT_SWIOT_RUNTIME) << "Trigger has been set";
		} else {
			qDebug(CAT_SWIOT_RUNTIME) << QString::number(ret) +" returned";
		}
	}
}

void SwiotRuntime::createDevicesMap(struct iio_context *ctx)
{
	if (ctx) {
		int devicesNumber = iio_context_get_devices_count(ctx);
		for (int i = 0; i < devicesNumber; i++) {
			struct iio_device* iioDev = iio_context_get_device(ctx, i);
			if (iioDev) {
				QString deviceName = QString(iio_device_get_name(iioDev));
				m_iioDevices[deviceName] = iioDev;
			}
		}
	}
}
