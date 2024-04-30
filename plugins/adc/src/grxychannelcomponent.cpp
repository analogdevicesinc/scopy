#include "grxychannelcomponent.h"

#include <pluginbase/preferences.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menuheader.h>

#include <gr-util/griiofloatchannelsrc.h>
#include <gr-util/grsignalpath.h>

#include <iio-widgets/iiowidget.h>
#include <iio-widgets/iiowidgetfactory.h>

Q_LOGGING_CATEGORY(CAT_GRXYCHANNELCOMPONENT, "GRXyChannelComponent");

using namespace scopy;
using namespace scopy::grutil;
using namespace scopy::adc;

GRXyChannelComponent::GRXyChannelComponent(GRIIOFloatChannelNode *node, PlotComponent *plotComponent, QPen pen,
					       QWidget *parent)
	: ChannelComponent(node->name(), plotComponent, pen, parent)

{

	m_node = node;
	m_grch = m_node->src();

	setupSignalPath();

	int yPlotAxisPosition = Preferences::get("adc_plot_yaxis_label_position").toInt();
	int yPlotAxisHandle = Preferences::get("adc_plot_yaxis_handle_position").toInt();

	m_running = false;
	m_enabled = false;

	m_scaleAvailable = true; // query from GRIIOFloatChannel;

	m_channelName = m_grch->getChannelName();
	auto plot = plotComponent->plot();

	m_plotAxis = plot->yAxis();
	m_plotCh = new PlotChannel(m_channelName, pen, plot, plot->xAxis(), m_plotAxis, this);
	/*m_plotAxisHandle = new PlotAxisHandle(pen, m_plotAxis, plot, yPlotAxisHandle, this);
	m_plotCh->setHandle(m_plotAxisHandle);
	plot->addPlotAxisHandle(m_plotAxisHandle);*/
	plot->addPlotChannel(m_plotCh);
	m_plotCh->xAxis()->setUnits("s");

	widget = createMenu();

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	auto m_lay = new QVBoxLayout(this);
	m_lay->setMargin(0);
	m_lay->setSpacing(0);
	widget = createMenu(this);
	m_lay->addWidget(widget);
	setLayout(m_lay);

	createMenuControlButton();
}

GRXyChannelComponent::~GRXyChannelComponent() {}

void GRXyChannelComponent::setupSignalPath()
{
	m_signalPath = new GRSignalPath(m_plotComponent->name()
						+ m_grch->getDeviceSrc()->deviceName()
						+ m_grch->getChannelName()
					, this);
	m_signalPath->append(m_grch);
	m_scOff = new GRScaleOffsetProc(m_signalPath);
	m_signalPath->append(m_scOff);
	m_scOff->setOffset(0);
	m_scOff->setScale(1);
	m_signalPath->setEnabled(false);
	m_node->top()->src()->registerSignalPath(m_signalPath);
}

GRSignalPath *GRXyChannelComponent::signalPath() const { return m_signalPath; }

QWidget *GRXyChannelComponent::createYAxisMenu(QWidget *parent)
{
	MenuSectionWidget *yaxiscontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *yaxis = new MenuCollapseSection("Y-AXIS", MenuCollapseSection::MHCW_NONE, yaxiscontainer);

	       // Y-MODE
	m_ymodeCb = new MenuCombo("YMODE", yaxis);
	auto cb = m_ymodeCb->combo();
	cb->addItem("ADC Counts", YMODE_COUNT);
	cb->addItem("% Full Scale", YMODE_FS);

	if(m_scaleAvailable) {
		cb->addItem(m_unit, YMODE_SCALE);
	}
	m_plotAxis->setUnits("V");

	yaxis->contentLayout()->addWidget(m_ymodeCb);
	yaxiscontainer->contentLayout()->addWidget(yaxis);

	connect(cb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		auto mode = cb->itemData(idx).toInt();
		setYMode(static_cast<YMode>(mode));
	});

	return yaxiscontainer;
}

/*
QPushButton *GRXyChannelComponent::createSnapshotButton(QWidget *parent)
{
	QPushButton *snapBtn = new QPushButton("Snapshot", parent);
	StyleHelper::BlueButton(snapBtn);

     connect(snapBtn, &QPushButton::clicked, this, [=]() {
	     std::vector<float> x, y;
	     auto data = m_plotCh->curve()->data();
	     for(int i = 0; i < data->size(); i++) {
		     x.push_back(data->sample(i).x());
		     y.push_back(data->sample(i).y());
	     }
	     SnapshotProvider::SnapshotRecipe rec{x, y, m_channelName};
	     Q_EMIT addNewSnapshot(rec);
     });

     snapBtn->setEnabled(false);
     return snapBtn;
}
*/

QWidget *GRXyChannelComponent::createMenu(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	QVBoxLayout *lay = new QVBoxLayout();

	QScrollArea *scroll = new QScrollArea(parent);
	QWidget *wScroll = new QWidget(scroll);
	QVBoxLayout *layScroll = new QVBoxLayout();
	layScroll->setMargin(0);
	layScroll->setSpacing(10);

	wScroll->setLayout(layScroll);
	scroll->setWidgetResizable(true);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	// if ScrollBarAlwaysOn - layScroll->setContentsMargins(0,0,6,0);

	scroll->setWidget(wScroll);

	lay->setMargin(0);
	lay->setSpacing(10);
	w->setLayout(lay);

	MenuHeaderWidget *header = new MenuHeaderWidget(m_channelName, m_pen, w);
	QWidget *yaxismenu = createYAxisMenu(w);
	QWidget *curvemenu = createCurveMenu(w);
	QWidget *attrmenu = createAttrMenu(w);
	//m_snapBtn = createSnapshotButton(w);

	lay->addWidget(header);
	lay->addWidget(scroll);
	layScroll->addWidget(yaxismenu);
	layScroll->addWidget(curvemenu);
	layScroll->addWidget(attrmenu);
	//layScroll->addWidget(m_snapBtn);

	layScroll->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	return w;
}

QWidget *GRXyChannelComponent::createAttrMenu(QWidget *parent)
{
	MenuSectionWidget *attrcontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *attr =
		new MenuCollapseSection("ATTRIBUTES", MenuCollapseSection::MHCW_NONE, attrcontainer);
	QList<IIOWidget *> attrWidgets = IIOWidgetFactory::buildAllAttrsForChannel(grch()->channel());

	auto layout = new QVBoxLayout();
	layout->setSpacing(10);
	layout->setMargin(0);
	layout->setContentsMargins(0, 0, 0, 10); // bottom margin

	for(auto w : attrWidgets) {
		layout->addWidget(w);
	}

	attr->contentLayout()->addLayout(layout);
	attrcontainer->contentLayout()->addWidget(attr);
	attr->header()->setChecked(false);
	return attrcontainer;
}

void GRXyChannelComponent::enableChannel()
{
	m_signalPath->setEnabled(true);
	ChannelComponent::enableChannel();
}

void GRXyChannelComponent::disableChannel()
{
	m_signalPath->setEnabled(false);
	ChannelComponent::disableChannel();
}

void GRXyChannelComponent::onStart()
{
	m_running = true;
}

void GRXyChannelComponent::onStop()
{
	m_running = false;
}

void GRXyChannelComponent::setYMode(YMode mode)
{
	double scale = 1;
	double offset = 0;
	double ymin = -1;
	double ymax = 1;
	const iio_data_format *fmt = m_grch->getFmt();

	switch(mode) {
	case YMODE_COUNT:
		scale = 1;
		if(fmt->is_signed) {
			ymin = -(float)((int64_t)1 << (fmt->bits - 1));
			ymax = (float)((int64_t)1 << (fmt->bits - 1));
		} else {
			ymin = 0;
			ymax = 1 << (fmt->bits);
		}
		m_plotCh->yAxis()->setUnits("Counts");
		break;
	case YMODE_FS:
		scale = 1.0 / ((float)((uint64_t)1 << fmt->bits));
		if(fmt->is_signed) {
			ymin = -0.5;
			ymax = 0.5;
		} else {
			ymin = 0;
			ymax = 1;
		}
		m_plotCh->yAxis()->setUnits("");
		break;
	case YMODE_SCALE:

		m_plotCh->yAxis()->setUnits(m_unit);
		break;
	default:
		break;
	}

	m_scOff->setScale(scale);
	m_scOff->setOffset(offset);
}

void GRXyChannelComponent::enable()
{
	m_ctrl->setVisible(true);
	if(m_enabled) {
		enableChannel();
	}
	ChannelComponent::enable();
}

void GRXyChannelComponent::disable()
{
	m_ctrl->setVisible(false);
	disableChannel();
	ChannelComponent::disable();
}

GRIIOFloatChannelSrc *GRXyChannelComponent::grch() const { return m_grch; }

MenuControlButton *GRXyChannelComponent::ctrl() {
	return m_ctrl;
}

void GRXyChannelComponent::onInit() {
	auto v = Preferences::get("adc_default_y_mode").toInt();
	m_ymodeCb->combo()->setCurrentIndex(v);
	setYMode(static_cast<YMode>(v));
}

void GRXyChannelComponent::onDeinit() {}

void GRXyChannelComponent::onNewData(const float *xData, const float *yData, int size) {
	//m_snapBtn->setEnabled(true);
}

void GRXyChannelComponent::createMenuControlButton(QWidget *parent) {
	m_ctrl = new MenuControlButton(parent);
	setupChannelMenuControlButtonHelper(m_ctrl);

}

void GRXyChannelComponent::setupChannelMenuControlButtonHelper(MenuControlButton *btn){
	btn->setName(m_channelName);
	btn->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
	btn->setOpenMenuChecksThis(true);
	btn->setDoubleClickToOpenMenu(true);
	btn->setColor(pen().color());
	btn->button()->setVisible(false);
	btn->setCheckable(true);

	connect(btn, &QAbstractButton::clicked, this, [=](){
		m_plotComponent->plot()->selectChannel(m_plotCh);
	});

	connect(btn->checkBox(), &QCheckBox::toggled, this, [=](bool b) {
		if(b)
			enableChannel();
		else
			disableChannel();
	});
	btn->checkBox()->setChecked(true);
}
