#include "gui/dynamicWidget.hpp"
#include "logging_categories.h"
#include "plugin/irightmenuplugin.h"
#include <FftDisplayPlot.h>
#include <newinstrument.hpp>
#include <ui_cursors_settings.h>


using namespace adiscope;
using namespace gui;
class CursorsRightMenuPlugin : public IRightMenuPlugin
{
private:
	std::vector<DisplayPlot *> plotList;
	Ui::CursorsSettings *cr_ui;
	CustomMenuButton* menuBtn;

	bool hasHorizontal;
	bool hasVertical;

public:
	CursorsRightMenuPlugin(QWidget *parent, ToolView* toolView, bool dockable, bool enableHorizontal = true, bool enableVertical = true) :
		IRightMenuPlugin(parent, toolView, dockable),
		hasHorizontal(enableHorizontal),
		hasVertical(enableVertical)
	{
		init();
	}

	~CursorsRightMenuPlugin()
	{
		delete cr_ui;
	}

	void init() override
	{
		qDebug(CAT_CURSOR_RIGHTMENU_PLUGIN) << "init()";

		auto instrument = dynamic_cast<NewInstrument *>(parent);
		auto plots = instrument->getPlotList();

		for (auto plot: *plots) {
			if (dynamic_cast<FftDisplayPlot*>(plot) != nullptr) {
				plotList.push_back(dynamic_cast<FftDisplayPlot*>(plot));
			}
		}

		menu = new GenericMenu(new QWidget());
		menu->initInteractiveMenu();
		menuBtn = toolView->buildNewInstrumentMenu(menu, dockable, "Cursors", true, false);

		initCursors();
		menu->setMenuWidget(cr_ui->scrollArea);
	}

	void initCursors()
	{
		cr_ui = new Ui::CursorsSettings;
		cr_ui->setupUi(menu);
		setDynamicProperty(cr_ui->btnLockHorizontal, "use_icon", true);
		setDynamicProperty(cr_ui->btnLockVertical, "use_icon", true);

		auto cursorsPositionButton = new CustomPlotPositionButton(cr_ui->posSelect);

		for (auto plot: plotList) {
			connect(cr_ui->btnLockHorizontal, &QPushButton::toggled,
				plot, &FftDisplayPlot::setHorizCursorsLocked);
			connect(cr_ui->btnLockVertical, &QPushButton::toggled,
				plot, &FftDisplayPlot::setVertCursorsLocked);

			connect(cr_ui->hCursorsEnable, SIGNAL(toggled(bool)), plot, SLOT(setVertCursorsEnabled(bool)));
			connect(cr_ui->vCursorsEnable, SIGNAL(toggled(bool)), plot, SLOT(setHorizCursorsEnabled(bool)));

			connect(cr_ui->horizontalSlider, &QSlider::valueChanged, [=](int value){
				cr_ui->transLabel->setText(tr("Transparency ") + QString::number(value) + "%");
				plot->setCursorReadoutsTransparency(value);
			});

			connect(cursorsPositionButton, &CustomPlotPositionButton::positionChanged,
				[=](CustomPlotPositionButton::ReadoutsPosition position){
				plot->moveCursorReadouts(position);
			});
		}

		connect(cr_ui->btnNormalTrack, &QPushButton::toggled,
			this, &CursorsRightMenuPlugin::toggleCursorsMode);

		cr_ui->horizontalSlider->setMaximum(100);
		cr_ui->horizontalSlider->setMinimum(0);
		cr_ui->horizontalSlider->setSingleStep(1);
		cr_ui->horizontalSlider->setSliderPosition(0);

		connect(menuBtn->getCheckBox(), &QCheckBox::stateChanged, this, &CursorsRightMenuPlugin::menuCheckboxClicked);

		// hide widgets if not required
		cr_ui->widgetHorizontal->setVisible(hasHorizontal);
		cr_ui->widgetHorizontalBtns->setVisible(hasHorizontal);

		cr_ui->widgetVertical->setVisible(hasVertical);
		cr_ui->widgetVerticalBtns->setVisible(hasVertical);
	}

	void toggleCursorsMode(bool toggled)
	{
		cr_ui->hCursorsEnable->setEnabled(toggled && hasHorizontal);
		cr_ui->vCursorsEnable->setEnabled(toggled && hasVertical);

		for (auto plot: plotList) {
			if (toggled) {
				plot->setVertCursorsEnabled(cr_ui->hCursorsEnable->isChecked() && hasHorizontal);
				plot->setHorizCursorsEnabled(cr_ui->vCursorsEnable->isChecked() && hasVertical);
			} else {
				plot->setVertCursorsEnabled(hasHorizontal);
				plot->setHorizCursorsEnabled(hasVertical);
			}

			cr_ui->btnLockVertical->setEnabled(toggled);
			plot->trackModeEnabled(toggled);
		}
	}

	void menuCheckboxClicked(int on)
	{
		for (auto plot: plotList) {
			plot->setHorizCursorsEnabled(on ? cr_ui->vCursorsEnable->isChecked() : false);
			plot->setVertCursorsEnabled(on ? cr_ui->hCursorsEnable->isChecked() : false);

			plot->trackModeEnabled(on ? cr_ui->btnNormalTrack->isChecked() : true);

			if (on) {
				plot->setCursorReadoutsVisible(true);
			}
		}

		if (!hasHorizontal) {
			Q_EMIT cr_ui->hCursorsEnable->toggled(false);
			cr_ui->hCursorsEnable->setChecked(false);
			cr_ui->hCursorsEnable->blockSignals(true);
		}
		if (!hasVertical) {
			Q_EMIT cr_ui->vCursorsEnable->toggled(false);
			cr_ui->vCursorsEnable->setChecked(false);
			cr_ui->vCursorsEnable->blockSignals(true);
		}
	}
};
