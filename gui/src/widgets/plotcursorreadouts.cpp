#include <widgets/plotcursorreadouts.h>

using namespace scopy;

PlotCursorReadouts::PlotCursorReadouts(QWidget *parent) : QWidget(parent),
	formatter(static_cast<PrefixFormatter *>(new MetricPrefixFormatter))
{
	initContent();
}

PlotCursorReadouts::~PlotCursorReadouts()
{

}

void PlotCursorReadouts::setV1(double val) { v1 = val; update();}

void PlotCursorReadouts::setV2(double val) { v2 = val; update();}

void PlotCursorReadouts::setH1(double val) { h1 = val; update();}

void PlotCursorReadouts::setH2(double val) { h2 = val; update();}

void PlotCursorReadouts::update()
{
	V1_val->setText(formatter->format(v1, "Hz", 3));
	V2_val->setText(formatter->format(v2, "Hz", 3));
	H1_val->setText(formatter->format(h1, "s", 3));
	H2_val->setText(formatter->format(h2, "s", 3));
	deltaV_val->setText(formatter->format(v2-v1, "Hz", 3));
	deltaH_val->setText(formatter->format(h2-h1, "s", 3));
	invDeltaH_val->setText(formatter->format(1/(h2-h1), "s", 3));
}

void PlotCursorReadouts::initContent()
{
	QHBoxLayout *content_lay = new QHBoxLayout(this);
	setLayout(content_lay);
	content_lay->setMargin(10);
	content_lay->setSpacing(10);

	// vertical readouts
	vert_contents = new QWidget(this);
	vert_contents->setFixedWidth(120);
	QGridLayout *vert_contents_lay = new QGridLayout(vert_contents);
	vert_contents_lay->setMargin(0);
	vert_contents_lay->setSpacing(2);
	vert_contents->setLayout(vert_contents_lay);
	content_lay->addWidget(vert_contents);

	vert_contents_lay->addWidget(new QLabel("V1:", vert_contents), 0, 0);
	V1_val = new QLabel("",this);
	vert_contents_lay->addWidget(V1_val, 0, 1);

	vert_contents_lay->addWidget(new QLabel("V2:", vert_contents), 1, 0);
	V2_val = new QLabel("",this);
	vert_contents_lay->addWidget(V2_val, 1, 1);

	vert_contents_lay->addWidget(new QLabel("ΔV:", vert_contents), 2, 0);
	deltaV_val = new QLabel("",this);
	vert_contents_lay->addWidget(deltaV_val, 2, 1);

	vert_contents_lay->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 3, 0);

	// horizontal readouts
	horiz_contents = new QWidget(this);
	horiz_contents->setFixedWidth(120);
	QGridLayout *horiz_contents_lay = new QGridLayout(horiz_contents);
	horiz_contents_lay->setMargin(0);
	horiz_contents_lay->setSpacing(2);
	horiz_contents->setLayout(horiz_contents_lay);
	content_lay->addWidget(horiz_contents);

	horiz_contents_lay->addWidget(new QLabel("H1:", horiz_contents), 0, 0);
	H1_val = new QLabel("",this);
	horiz_contents_lay->addWidget(H1_val, 0, 1);

	horiz_contents_lay->addWidget(new QLabel("H2:", horiz_contents), 1, 0);
	H2_val = new QLabel("",this);
	horiz_contents_lay->addWidget(H2_val, 1, 1);

	horiz_contents_lay->addWidget(new QLabel("ΔH:", horiz_contents), 2, 0);
	deltaH_val = new QLabel("",this);
	horiz_contents_lay->addWidget(deltaH_val, 2, 1);

	horiz_contents_lay->addWidget(new QLabel("1/ΔH:", horiz_contents), 3, 0);
	invDeltaH_val = new QLabel("",this);
	horiz_contents_lay->addWidget(invDeltaH_val, 3, 1);

	for (auto w: this->findChildren<QWidget*>()) {
		StyleHelper::TransparentWidget(w);
	}
	resize(sizeHint());
}

void PlotCursorReadouts::horizSetVisible(bool visible)
{
	horiz_contents->setVisible(visible);
	resize(sizeHint());
}

void PlotCursorReadouts::vertSetVisible(bool visible)
{
	vert_contents->setVisible(visible);
	resize(sizeHint());
}

bool PlotCursorReadouts::horizIsVisible()
{
	return horiz_contents->isVisible();
}

bool PlotCursorReadouts::vertIsVisible()
{
	return vert_contents->isVisible();
}
