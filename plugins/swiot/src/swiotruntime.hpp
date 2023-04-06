#ifndef SWIOTRUNTIME_HPP
#define SWIOTRUNTIME_HPP
#include <filter.hpp>
#include <tool_launcher.hpp>
#include "swiotad.hpp"
#include "tool.hpp"

#define AD_NAME "ad74413r"

namespace adiscope {

namespace gui {
class GenericMenu;
class ChannelManager;
}

class SwiotRuntime : public Tool
{
public:
	SwiotRuntime(struct iio_context *ctx, Filter *filt,
		      ToolMenuItem *toolMenuItem,
		      QJSEngine *engine, ToolLauncher *parent);
	~SwiotRuntime();

	adiscope::gui::ToolView* getToolView();
	bool isRuntimeCtx();

	void setToolView(adiscope::gui::ToolView* toolView);
	void initAdToolView();
public Q_SLOTS:
	void backBtnPressed();
private:
	SwiotAd* m_swiotRuntimeAd;
	adiscope::gui::ToolView* m_toolView;
	QPushButton* m_backBtn;
	QMap<QString, struct iio_device*> m_iioDevices;
private:
	QPushButton* createBackBtn();

	//	//Runtime A
//	const QVector<QString> channel_function{"digital_input", "voltage_out", "current_out", "voltage_in",
//							"diagnostic", "diagnostic", "diagnostic", "diagnostic"};
	//	//Runtime B
		const QVector<QString> channel_function{"current_in_ext", "current_in_loop", "resistance", "digital_input",
		"diagnostic", "diagnostic", "diagnostic", "diagnostic"};
	//	//Runtime C
//		const QVector<QString> channel_function{"digital_input_loop", "current_in_ext_hart", "current_in_ext_hart", "high_z",
//		"diagnostic", "diagnostic", "diagnostic", "diagnostic"};
	//	//Runtime D
//		const QVector<QString> channel_function{"current_out", "high_z", "high_z", "high_z",
//		"diagnostic", "diagnostic", "diagnostic", "diagnostic"};
};
}


#endif // SWIOTRUNTIME_HPP
