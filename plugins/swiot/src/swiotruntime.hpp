#ifndef SWIOTRUNTIME_HPP
#define SWIOTRUNTIME_HPP

#include "swiotad.hpp"

#define AD_NAME "ad74413r"
#define AD_TRIGGER_NAME "ad74413r-dev0"
#define TRIGGER_TIMER_MS 1000

namespace adiscope {

namespace gui {
class GenericMenu;
class ChannelManager;
}

class SwiotRuntime : public QWidget
{
public:
	explicit SwiotRuntime(struct iio_context *ctx = nullptr, QWidget *parent = nullptr);
	~SwiotRuntime();

	adiscope::gui::ToolView* getToolView();
	bool isRuntimeCtx();

	void setToolView(adiscope::gui::ToolView* toolView);
	void initAdToolView();
public Q_SLOTS:
	void onBackBtnPressed();
	void triggerPing();
Q_SIGNALS:
	void backBtnPressed();
private:
	SwiotAd* m_swiotRuntimeAd;
	adiscope::gui::ToolView* m_toolView;
	QPushButton* m_backBtn;
	QMap<QString, struct iio_device*> m_iioDevices;
	QTimer *m_triggerTimer;
private:
	QPushButton* createBackBtn();
	void createDevicesMap(struct iio_context *ctx);

	//	//Runtime A
	const QVector<QString> channel_function{"digital_input", "voltage_out", "current_out", "voltage_in",
							"diagnostic", "diagnostic", "diagnostic", "diagnostic"};
	//	//Runtime B
//		const QVector<QString> channel_function{"current_in_ext", "current_in_loop", "resistance", "digital_input",
//		"diagnostic", "diagnostic", "diagnostic", "diagnostic"};
	//	//Runtime C
//		const QVector<QString> channel_function{"digital_input_loop", "current_in_ext_hart", "current_in_ext_hart", "high_z",
//		"diagnostic", "diagnostic", "diagnostic", "diagnostic"};
	//	//Runtime D
//		const QVector<QString> channel_function{"current_out", "high_z", "high_z", "high_z",
//		"diagnostic", "diagnostic", "diagnostic", "diagnostic"};
};
}


#endif // SWIOTRUNTIME_HPP
