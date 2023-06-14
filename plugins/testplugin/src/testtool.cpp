#include "testtool.h"
#include <QHBoxLayout>
#include <gui/menu_anim.hpp>
#include <QDebug>
#include <gui/stylehelper.h>
#include <QButtonGroup>
#include <hoverwidget.h>

#include <QwtPlot>
#include <QwtPlotLayout>
#include <QwtPlotSeriesItem>
#include <gui/DisplayPlot.h>
#include <gui/customqwtscaledraw.hpp>

using namespace scopy;

QMap<QString, QColor> colorMap;

float vals[4] = {1,2,3,4};
class Plot : public QwtPlot {
public:
	Plot(QWidget *parent = nullptr) : QwtPlot(parent) {



		setAxisScale(QwtAxisId(QwtAxis::XBottom,0),-10,10,2);
		setAxisScale(QwtAxisId(QwtAxis::YLeft,0),-10,10,2);

		OscScaleDraw *xScaleDraw = new OscScaleDraw(new MetricPrefixFormatter(),"");
		OscScaleDraw *yScaleDraw = new OscScaleDraw(new MetricPrefixFormatter(),"");

//		xScaleDraw->enableDeltaLabel(true);
//		yScaleDraw->enableDeltaLabel(true);
//		yScaleDraw->setLabelRotation(66);

		plotLayout()->setAlignCanvasToScales( true );

		axisWidget(QwtAxisId(QwtAxis::YLeft,0))->setMargin(0);

		setAxisScaleDraw(QwtAxisId(QwtAxis::XBottom,0),xScaleDraw);
		setAxisScaleDraw(QwtAxisId(QwtAxis::YLeft,0), yScaleDraw);
		setAxisVisible(QwtAxisId(QwtAxis::XBottom,0), false);
		setAxisVisible(QwtAxisId(QwtAxis::YLeft,0), false);
		// Plot needs a grid
		EdgelessPlotGrid *d_grid = new EdgelessPlotGrid();
		QColor majorPenColor("#353537");
		d_grid->setMajorPen(majorPenColor, 1.0, Qt::DashLine);
		d_grid->attach(this);

		QwtPlotMarker *d_origin = new QwtPlotMarker();
		d_origin->setLineStyle( QwtPlotMarker::Cross );
		d_origin->setValue( 0, 0.0 );
		d_origin->setLinePen( Qt::gray, 0.0, Qt::DashLine );
		d_origin->attach( this );

		QwtPlotCurve *c1 = new QwtPlotCurve();
		c1->setVisible(true);
		c1->setStyle( QwtPlotCurve::Lines );
		c1->setPen(QColor("red"),1);
		c1->setRenderHint( QwtPlotItem::RenderAntialiased, true );
		c1->setPaintAttribute( QwtPlotCurve::ClipPolygons, false );

		c1->attach(this);

		c1->setSamples(vals,vals,4);





		};
	~Plot() {};
};


TestTool::TestTool(QWidget *parent)
{
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
	btn3 = new OpenLastMenuBtn(dynamic_cast<MenuAnim*>(tool->rightContainer()),false,this);
	tool->setLeftContainerWidth(200);
	tool->setRightContainerWidth(300);
	tool->openLeftContainerHelper(false);
	tool->openRightContainerHelper(false);

	GearBtn *btn5 = new GearBtn(this);
	RunBtn *runBtn = new RunBtn(this);
	InfoBtn *infoBtn = new InfoBtn(this);
	SingleShotBtn *singleBtn = new SingleShotBtn(this);
	PrintBtn *printBtn = new PrintBtn(this);

	MenuControlButton *ch0 = new MenuControlButton(this);
	ch0->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
	ch0->setName("Channels");
//	ch0->setColor(StyleHelper::getColor("CH0"));
	ch0->checkBox()->setVisible(false);
	ch0->setChecked(false);

	MenuControlButton *ch1 = new MenuControlButton(this);
	ch1->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
	ch1->setName("Channel 1");
	ch1->setColor(StyleHelper::getColor("CH1"));
	ch1->button()->setVisible(false);
	ch1->checkBox()->setChecked(true);
	ch1->setChecked(true);

	MenuControlButton *ch2 = new MenuControlButton(this);
	ch2->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
	ch2->setName("Channel 2");
	ch2->setColor(StyleHelper::getColor("CH2"));

	MenuControlButton *cursor = new MenuControlButton(this);
	cursor->setName("Cursors");
	cursor->setCheckBoxStyle(MenuControlButton::CS_SQUARE);
	MenuControlButton *measure = new MenuControlButton(this);
	measure->setName("Measure");

	tool->addWidgetToTopContainerMenuControlHelper(btn3,TTA_RIGHT);
	tool->addWidgetToTopContainerMenuControlHelper(btn5,TTA_LEFT);

	tool->addWidgetToTopContainerHelper(runBtn,TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(singleBtn,TTA_RIGHT);

	tool->addWidgetToTopContainerHelper(infoBtn,TTA_LEFT);
	tool->addWidgetToTopContainerHelper(printBtn,TTA_LEFT);

	tool->addWidgetToBottomContainerHelper(ch0, TTA_LEFT);
	tool->addWidgetToBottomContainerHelper(ch1, TTA_LEFT);
	tool->addWidgetToBottomContainerHelper(ch2, TTA_LEFT);

	tool->addWidgetToBottomContainerHelper(cursor, TTA_RIGHT);
	tool->addWidgetToBottomContainerHelper(measure, TTA_RIGHT);

	QLabel *wch0 = new QLabel("Channel0Label");
	QLabel *wch1 = new QLabel("Channel1Label");
	QLabel *wch2 = new QLabel("Channel2Label");

	tool->rightStack()->add("ch0",wch0);
	tool->rightStack()->add("ch1",wch1);
	tool->rightStack()->add("ch2",wch2);

	connect(ch0->button(),&QAbstractButton::pressed,this,[=](){tool->requestMenu("ch0");});
	connect(ch1->button(),&QAbstractButton::pressed,this,[=](){tool->requestMenu("ch1");});
	connect(ch2->button(),&QAbstractButton::pressed,this,[=](){tool->requestMenu("ch2");});

	auto grp = static_cast<OpenLastMenuBtn*>(btn3)->getButtonGroup();
	grp->addButton(ch0->button());
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
	vcm->setFixedSize(260,350);

	HoverWidget *hv = new HoverWidget(vcm, ch0, tool);
	hv->setAnchorOffset(QPoint(0,-10));
	hv->setAnchorPos(HoverPosition::HP_TOPLEFT);
	hv->setContentPos(HoverPosition::HP_TOPRIGHT);


	connect(ch0, &QAbstractButton::toggled, this, [=](bool b) {
		qInfo()<<"setVisible: "<<b;
		hv->setVisible(b);
		hv->raise();

	});

	setupPlot();



}


void TestTool::setupPlot() {
	Plot *plot = new Plot(this);
	tool->addWidgetToCentralContainerHelper(plot);
	plot->replot();

}


