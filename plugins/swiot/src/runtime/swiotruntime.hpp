#ifndef SWIOTRUNTIME_HPP
#define SWIOTRUNTIME_HPP

#include "src/runtime/ad74413r/ad74413r.hpp"

namespace adiscope {
namespace gui {
class GenericMenu;
class ChannelManager;
}

#define AD_NAME "ad74413r"
#define AD_TRIGGER_NAME "ad74413r-dev0"
#define TRIGGER_TIMER_MS 1000

class SwiotRuntime : public QWidget
{
	Q_OBJECT
public:
	explicit SwiotRuntime(struct iio_context *ctx = nullptr, QWidget *parent = nullptr);
	~SwiotRuntime();

	adiscope::gui::ToolView* getToolView();
	bool isRuntimeCtx();

	void setToolView(adiscope::gui::ToolView* toolView);
	void initAdToolView();
//	void triggerPing();
public Q_SLOTS:
	void onBackBtnPressed();
//	void onTriggerTimeout();

Q_SIGNALS:
	void backBtnPressed();
private:
	swiot::Ad74413r* m_swiotRuntimeAd;
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
