#include "testtool.h"

#include "plotaxis.h"
#include "plotwidget.h"
#include "spinbox_a.hpp"
#include <iio-widgets/iiowidgetfactory.h>

#include <QButtonGroup>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>

#include <gui/cursorcontroller.h>
#include <gui/menu_anim.hpp>
#include <gui/stylehelper.h>
#include <gui/widgets/hoverwidget.h>
#include <gui/widgets/menubigswitch.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/menuheader.h>
#include <gui/widgets/menulineedit.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/toolbuttons.h>
#include <gui/widgets/verticalchannelmanager.h>
#include <gui/widgets/errorbox.h>
#include <gui/widgets/hoverwidget.h>

using namespace scopy;

QMap<QString, QColor> colorMap;

TestTool::TestTool(QWidget *parent)
{
	initData();
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QHBoxLayout *lay = new QHBoxLayout(this);
	setLayout(lay);
	tool = new ToolTemplate(this);
	tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tool->bottomContainer()->setVisible(true);
	tool->topContainer()->setVisible(true);
	tool->leftContainer()->setVisible(true);
	tool->rightContainer()->setVisible(true);
	lay->addWidget(tool);

	StyleHelper::GetInstance()->initColorMap();
	btn3 = new OpenLastMenuBtn(dynamic_cast<MenuHAnim *>(tool->rightContainer()), false, this);
	tool->setLeftContainerWidth(200);
	tool->setRightContainerWidth(300);
	tool->openLeftContainerHelper(false);
	tool->openRightContainerHelper(false);

	PlotWidget *plot = new PlotWidget(this);
	tool->addWidgetToCentralContainerHelper(plot);
	plot->xAxis()->setInterval(0, 1);
	plot->replot();

	GearBtn *btn5 = new GearBtn(this);
	RunBtn *runBtn = new RunBtn(this);
	InfoBtn *infoBtn = new InfoBtn(this);
	SingleShotBtn *singleBtn = new SingleShotBtn(this);
	PrintBtn *printBtn = new PrintBtn(this);

	QTimer *dataRefreshTimer = new QTimer(this);
	dataRefreshTimer->setInterval(10);
	connect(runBtn, &QPushButton::toggled, this, [=](bool b) {
		if(b) {
			dataRefreshTimer->start();
		} else {
			dataRefreshTimer->stop();
		}
	});
	connect(dataRefreshTimer, &QTimer::timeout, this, &TestTool::acquireData);
	connect(dataRefreshTimer, &QTimer::timeout, plot, &PlotWidget::replot);

	MenuControlButton *channels = new MenuControlButton(this);
	channels->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
	channels->setName("Channels");
	//	ch0->setColor(StyleHelper::getColor("CH0"));
	channels->checkBox()->setVisible(false);
	channels->setChecked(false);

	QButtonGroup *channelButtonGroup = new QButtonGroup(this);

	MenuControlButton *ch1 = new MenuControlButton(this);
	ch1->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
	ch1->setName("Channel 1");
	ch1->setColor(StyleHelper::getColor("CH1"));
	ch1->button()->setVisible(false);
	ch1->checkBox()->setChecked(true);
	ch1->setChecked(true);
	channelButtonGroup->addButton(ch1);

	QPen ch1pen = QPen(QColor(StyleHelper::getColor("CH1")), 1);
	auto *ch1PlotAxis = new PlotAxis(QwtAxis::YLeft, plot, ch1pen);
	PlotChannel *ch1_plotch = new PlotChannel("Channel1", ch1pen, plot, plot->xAxis(), ch1PlotAxis, this);
	ch1_plotch->setHandle(new PlotAxisHandle(ch1pen, ch1PlotAxis, plot, QwtAxis::YLeft, this));
	plot->addPlotAxisHandle(ch1_plotch->handle());

	connect(ch1->checkBox(), &QCheckBox::toggled, ch1_plotch, &PlotChannel::setEnabled);
	connect(ch1->checkBox(), &QCheckBox::toggled, this, [=]() { plot->replot(); });
	connect(ch1, &QAbstractButton::toggled, this, [=]() { plot->selectChannel(ch1_plotch); });
	ch1_plotch->curve()->setRawSamples(xTime.data(), y1Volt.data(), xTime.size());

	MenuControlButton *ch2 = new MenuControlButton(this);
	ch2->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
	ch2->setName("Channel 2");
	ch2->setColor(StyleHelper::getColor("CH2"));
	channelButtonGroup->addButton(ch2);

	QPen ch2pen = QPen(QColor(StyleHelper::getColor("CH2")), 1);
	auto *ch2PlotAxis = new PlotAxis(QwtAxis::YLeft, plot, ch2pen);
	PlotChannel *ch2_plotch = new PlotChannel("Channel2", ch2pen, plot, plot->xAxis(), ch2PlotAxis, this);
	ch2_plotch->setHandle(new PlotAxisHandle(ch2pen, ch2PlotAxis, plot, QwtAxis::YRight, this));
	plot->addPlotAxisHandle(ch2_plotch->handle());
	connect(ch2->checkBox(), &QCheckBox::toggled, ch2_plotch, &PlotChannel::setEnabled);
	connect(ch2->checkBox(), &QCheckBox::toggled, this, [=]() { plot->replot(); });
	connect(ch2, &QAbstractButton::toggled, this, [=]() { plot->selectChannel(ch2_plotch); });
	ch2_plotch->curve()->setRawSamples(xTime.data(), y2Volt.data(), xTime.size());

	MenuControlButton *cursor = new MenuControlButton(this);
	cursor->setName("Cursors");
	cursor->setOpenMenuChecksThis(true);
	cursor->setDoubleClickToOpenMenu(true);
	cursor->checkBox()->setVisible(false);
	cursor->setCheckBoxStyle(MenuControlButton::CS_SQUARE);

	MenuControlButton *measure = new MenuControlButton(this);
	measure->setName("Measure");
	measure->setOpenMenuChecksThis(true);
	measure->setDoubleClickToOpenMenu(true);
	measure->checkBox()->setVisible(false);

	CursorController *cursorController = new CursorController(plot, this);

	HoverWidget *hoverSettings = new HoverWidget(cursorController->getCursorSettings(), cursor, tool);
	hoverSettings->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	hoverSettings->setContentPos(HoverPosition::HP_TOPLEFT);
	hoverSettings->setAnchorOffset(QPoint(0, -10));

	connect(cursor->button(), &QAbstractButton::toggled, hoverSettings, &HoverWidget::setVisible);
	connect(cursor, &QAbstractButton::toggled, cursorController, &CursorController::setVisible);

	tool->addWidgetToTopContainerMenuControlHelper(btn3, TTA_RIGHT);
	tool->addWidgetToTopContainerMenuControlHelper(btn5, TTA_LEFT);

	tool->addWidgetToTopContainerHelper(runBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(singleBtn, TTA_RIGHT);

	tool->addWidgetToTopContainerHelper(infoBtn, TTA_LEFT);
	tool->addWidgetToTopContainerHelper(printBtn, TTA_LEFT);

	tool->addWidgetToBottomContainerHelper(channels, TTA_LEFT);
	tool->addWidgetToBottomContainerHelper(ch1, TTA_LEFT);
	tool->addWidgetToBottomContainerHelper(ch2, TTA_LEFT);

	tool->addWidgetToBottomContainerHelper(cursor, TTA_RIGHT);
	tool->addWidgetToBottomContainerHelper(measure, TTA_RIGHT);

	QWidget *wch0 = createMenu(tool);
	QLabel *wch1 = new QLabel("Channel1Label");

	//		auto *wch2 = iioWidgetsSettingsHelper();
	auto *wch2 = new QLabel("Channel2Label");
	// wch2->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::MinimumExpanding));

	tool->rightStack()->add("ch0", wch0);
	tool->rightStack()->add("ch1", wch1);
	tool->rightStack()->add("ch2", wch2);

	connect(channels->button(), &QAbstractButton::pressed, this, [=]() { tool->requestMenu("ch0"); });
	connect(ch1->button(), &QAbstractButton::pressed, this, [=]() { tool->requestMenu("ch1"); });
	connect(ch2->button(), &QAbstractButton::pressed, this, [=]() { tool->requestMenu("ch2"); });

	auto grp = static_cast<OpenLastMenuBtn *>(btn3)->getButtonGroup();
	grp->addButton(channels->button());
	grp->addButton(ch1->button());
	grp->addButton(ch2->button());
	grp->addButton(btn5);

	VerticalChannelManager *vcm = new VerticalChannelManager(tool->leftContainer());
	CollapsableMenuControlButton *dev1 = new CollapsableMenuControlButton(this);
	dev1->getControlBtn()->setName("cf-ad9361-lpc");
	MenuControlButton *ad9361_ch0 = new MenuControlButton(this);
	ad9361_ch0->setName("voltage0");
	ad9361_ch0->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
	ad9361_ch0->setColor(StyleHelper::getColor("CH0"));
	MenuControlButton *ad9361_ch1 = new MenuControlButton(this);
	ad9361_ch1->setName("voltage1");
	ad9361_ch1->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
	ad9361_ch1->setColor(StyleHelper::getColor("CH1"));

	MenuControlButton *fileChannel = new MenuControlButton(this);
	fileChannel->setName("test.wav");
	fileChannel->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
	fileChannel->setColor(StyleHelper::getColor("CH0"));
	Util::retainWidgetSizeWhenHidden(fileChannel->checkBox());
	fileChannel->checkBox()->setVisible(false);

	dev1->add(ad9361_ch0);
	dev1->add(ad9361_ch1);

	vcm->add(dev1);
	vcm->add(fileChannel);
	vcm->setFixedSize(260, 350);

	HoverWidget *hv = new HoverWidget(nullptr, channels, tool);
	hv->setContent(vcm);
	hv->setAnchorOffset(QPoint(0, -10));
	hv->setAnchorPos(HoverPosition::HP_TOPLEFT);
	hv->setContentPos(HoverPosition::HP_TOPRIGHT);

	plot->leftHandlesArea()->setVisible(true);
	plot->rightHandlesArea()->setVisible(true);
	plot->bottomHandlesArea()->setVisible(true);
	plot->topHandlesArea()->setVisible(true);

	connect(channels, &QAbstractButton::toggled, this, [=](bool b) {
		qInfo() << "setVisible: " << b;
		hv->setVisible(b);
		hv->raise();
	});
}

QWidget *TestTool::createMenu(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	QVBoxLayout *lay = new QVBoxLayout(w);
	lay->setMargin(0);
	lay->setSpacing(10);
	w->setLayout(lay);

	MenuHeaderWidget *header = new MenuHeaderWidget("channel 0", QPen(StyleHelper::getColor("CH0")), w);
	MenuComboWidget *yscale = new MenuComboWidget(tr("Y-Axis"), w);
	MenuSectionWidget *vdiv = new MenuSectionWidget(w);

	QLabel *txt = new QLabel("VDiv", vdiv);
	StyleHelper::MenuSmallLabel(txt, "vdivLabel");
	QComboBox *cbb = new QComboBox(vdiv);
	cbb->addItem("Lorem");
	cbb->addItem("Ipsum");
	cbb->addItem("Corectipsum");
	StyleHelper::MenuComboBox(cbb, "vdivCombo");

	PositionSpinButton *ssb =
		new PositionSpinButton({{"Hz", 1e0}, {"kHz", 1e3}, {"MHz", 1e6}}, "Volts", 0, 1000, true, false, vdiv);
	ssb->setMaxValue(1000);
	StyleHelper::MenuSpinBox(ssb, "vdivSpin");

	MenuOnOffSwitch *autoscale = new MenuOnOffSwitch(tr("AUTOSCALE"), vdiv, false);

	MenuCollapseSection *section1 = new MenuCollapseSection("SECTION1", MenuCollapseSection::MHCW_ARROW, vdiv);
	section1->contentLayout()->addWidget(txt);
	section1->contentLayout()->addWidget(cbb);
	MenuCollapseSection *section2 = new MenuCollapseSection("SECTION2", MenuCollapseSection::MHCW_ONOFF, vdiv);
	section2->contentLayout()->addWidget(ssb);
	section2->contentLayout()->addWidget(autoscale);

	//	MenuBigSwitch *bigsw = new MenuBigSwitch("Yes", "No", vdiv);
	QLabel *lbl = new QLabel("AUTOSCALE");
	StyleHelper::MenuSmallLabel(lbl, "edit");
	MenuLineEdit *edit = new MenuLineEdit(vdiv);

	vdiv->contentLayout()->addWidget(section1);
	vdiv->contentLayout()->addWidget(section2);
	//	vdiv->contentLayout()->addWidget(bigsw);
	vdiv->contentLayout()->addWidget(lbl);
	vdiv->contentLayout()->addWidget(edit);

	yscale->combo()->addItem("ADC Counts");
	yscale->combo()->addItem("% Full Scale");

	lay->addWidget(header);
	lay->addWidget(yscale);
	lay->addWidget(vdiv);
	lay->addSpacerItem(new QSpacerItem(40, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	QPushButton *btn = new QPushButton("TESTBtn");
	StyleHelper::BlueButton(btn, "TestBtn");
	vdiv->contentLayout()->addWidget(btn);
	vdiv->contentLayout()->setSpacing(4);

	return w;
}

void TestTool::initData()
{
	for(int i = 0; i < testDataSize; i++) {
		xTime.push_back((i / (double)testDataSize));
		y1Volt.push_back(amplitude * sin(2 * 10 * 3.1416 * i / (double)testDataSize));
		y2Volt.push_back(amplitude * sin(2 * 20 * 3.1416 * i / (double)testDataSize));
	}
}

void TestTool::acquireData()
{
	y1Volt.clear();
	y2Volt.clear();
	for(int i = 0; i < testDataSize; i++) {
		y1Volt.push_back(amplitude * sin(2 * 10 * 3.1416 * i / (double)testDataSize + phase));
		y2Volt.push_back(amplitude * sin(2 * 17 * 3.1416 * i / (double)testDataSize + 2 * phase));
	}
	phase++;
}

QWidget *TestTool::iioWidgetsSettingsHelper()
{
	// struct iio_context *context = iio_create_context_from_uri("ip:127.0.0.1");
	// struct iio_device *device = iio_context_find_device(context, "ad74413r");
	// struct iio_channel *attrChannel = iio_device_find_channel(device, "voltage7", false);
	auto *wch2Scroll = new QScrollArea(this);

	wch2Scroll->setWidgetResizable(true);
	wch2Scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	wch2Scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	auto wch2 = new QWidget(this);
	auto channelAttrMenu =
		new MenuCollapseSection("Channel attributes", scopy::MenuCollapseSection::MHCW_ARROW, wch2);
	channelAttrMenu->contentLayout()->setContentsMargins(0, 0, 0, 0);
	channelAttrMenu->contentLayout()->setSpacing(10);
	channelAttrMenu->setObjectName("this object");
	wch2Scroll->setWidget(wch2);
	wch2->setLayout(new QVBoxLayout(wch2));
	wch2->layout()->setContentsMargins(0, 0, 0, 0);
	QList<IIOWidget *> attrWidgets = {}; // IIOWidgetFactory::buildAllAttrsForChannel(attrChannel);

	//	attrWidgets.append(attrFactory->buildSingle(
	//		AttrFactory::AFH::ExternalSave | AttrFactory::AFH::SwitchUi | AttrFactory::AFH::AttrData,
	//		{.channel = attrChannel, .data = "sampling_frequency", .iioDataOptions =
	//"sampling_frequency_available"}));
	//
	//	attrWidgets.append(attrFactory->buildSingle(
	//		AttrFactory::AFH::TimeSave | AttrFactory::AFH::RangeUi | AttrFactory::AFH::AttrData,
	//		{.channel = attrChannel, .data = "raw", .iioDataOptions = "raw_available"}));
	//
	//	attrWidgets.append(attrFactory->buildSingle(AttrFactory::AFH::InstantSave | AttrFactory::AFH::EditableUi
	//| 							    AttrFactory::AFH::AttrData,
	//						    {.channel = attrChannel, .data = "offset"}));
	//	attrWidgets.append(IIOWidgetFactory::buildSingle(
	//		IIOWidgetFactory::TimeSave | IIOWidgetFactory::ComboUi | IIOWidgetFactory::TriggerData,
	//		{.context = context, .device = device, .data = "device_to_set_trigger_on"}));
	//	attrWidgets.append(IIOWidgetFactory::buildSingle(IIOWidgetFactory::TimeSave | IIOWidgetFactory::ComboUi
	//| 								 IIOWidgetFactory::FileDemoData,
	//							 {.data = "The Office Cast"}));

	StyleHelper::IIOWidgetElement(channelAttrMenu, "IIOWidget");
	for(auto item : attrWidgets) {
		if(item) {
			auto container = new QFrame(channelAttrMenu);
			auto header = new QWidget(channelAttrMenu);
			auto title = new QLabel(item->getRecipe().data.replace("_", " ").toUpper(), header);
			auto errorBox = new ErrorBox(header);
			header->setLayout(new QHBoxLayout(header));
			header->layout()->setContentsMargins(0, 0, 0, 0);
			header->layout()->addWidget(title);
			header->layout()->addWidget(errorBox);
			errorBox->changeColor(ErrorBox::AvailableColors::Green);
			connect(item, &IIOWidget::currentStateChanged, this,
				[errorBox](IIOWidget::State state, QString explanation) {
					if(state == IIOWidget::Busy) {
						errorBox->changeColor(ErrorBox::Yellow);
					} else if(state == IIOWidget::Correct) {
						errorBox->changeColor(ErrorBox::Green);
					} else if(state == IIOWidget::Error) {
						errorBox->changeColor(ErrorBox::Red);
					}
					errorBox->setToolTip(explanation);
				});

			container->setLayout(new QVBoxLayout(container));
			container->layout()->setContentsMargins(0, 0, 0, 0);
			container->layout()->addWidget(header);
			container->layout()->addWidget(item);

			StyleHelper::IIOWidget(container, "iioWidgetElement");
			channelAttrMenu->contentLayout()->addWidget(container);
		}
	}
	wch2->layout()->addWidget(channelAttrMenu);
	wch2->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::MinimumExpanding));

	return wch2Scroll;
}
