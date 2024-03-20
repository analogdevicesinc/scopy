#include <widgets/plotcursorreadouts.h>

using namespace scopy;

PlotCursorReadouts::PlotCursorReadouts(QWidget *parent)
	: QWidget(parent)
	, vFormatter(nullptr)
	, hFormatter(nullptr)
	, hUnit("")
	, vUnit("")
{
	initContent();
}

PlotCursorReadouts::~PlotCursorReadouts() {}

void PlotCursorReadouts::setV1(double val)
{
	v1 = val;
	update();
}

void PlotCursorReadouts::setV2(double val)
{
	v2 = val;
	update();
}

void PlotCursorReadouts::setH1(double val)
{
	h1 = val;
	update();
}

void PlotCursorReadouts::setH2(double val)
{
	h2 = val;
	update();
}

void PlotCursorReadouts::update()
{
	const uint precision = 3;

	if(hFormatter && !hUnit.isEmpty()) {
		H1_val->setText(hFormatter->format(h1, hUnit, precision));
		H2_val->setText(hFormatter->format(h2, hUnit, precision));
		deltaH_val->setText(hFormatter->format(h2 - h1, hUnit, precision));
		invDeltaH_val->setText(hFormatter->format(1 / (h2 - h1), hUnit, precision));
	} else {
		H1_val->setText(QString::number(h1, 'g', precision) + " " + hUnit);
		H2_val->setText(QString::number(h2, 'g', precision) + " " + hUnit);
		deltaH_val->setText(QString::number(h2 - h1, 'g', precision) + " " + hUnit);
		invDeltaH_val->setText(QString::number(1 / (h2 - h1), 'g', precision) + " " + hUnit);
	}

	if(vFormatter && !vUnit.isEmpty()) {
		V1_val->setText(vFormatter->format(v1, vUnit, precision));
		V2_val->setText(vFormatter->format(v2, vUnit, precision));
		deltaV_val->setText(vFormatter->format(v2 - v1, vUnit, precision));
	} else {
		V1_val->setText(QString::number(v1, 'g', precision) + " " + vUnit);
		V2_val->setText(QString::number(v2, 'g', precision) + " " + vUnit);
		deltaV_val->setText(QString::number(v2 - v1, 'g', precision) + " " + vUnit);
	}
}

void PlotCursorReadouts::initContent()
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
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
	V1_val = new QLabel("", this);
	vert_contents_lay->addWidget(V1_val, 0, 1);

	vert_contents_lay->addWidget(new QLabel("V2:", vert_contents), 1, 0);
	V2_val = new QLabel("", this);
	vert_contents_lay->addWidget(V2_val, 1, 1);

	vert_contents_lay->addWidget(new QLabel("ΔV:", vert_contents), 2, 0);
	deltaV_val = new QLabel("", this);
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
	H1_val = new QLabel("", this);
	horiz_contents_lay->addWidget(H1_val, 0, 1);

	horiz_contents_lay->addWidget(new QLabel("H2:", horiz_contents), 1, 0);
	H2_val = new QLabel("", this);
	horiz_contents_lay->addWidget(H2_val, 1, 1);

	horiz_contents_lay->addWidget(new QLabel("ΔH:", horiz_contents), 2, 0);
	deltaH_val = new QLabel("", this);
	horiz_contents_lay->addWidget(deltaH_val, 2, 1);

	horiz_contents_lay->addWidget(new QLabel("1/ΔH:", horiz_contents), 3, 0);
	invDeltaH_val = new QLabel("", this);
	horiz_contents_lay->addWidget(invDeltaH_val, 3, 1);

	for(auto w : this->findChildren<QWidget *>()) {
		StyleHelper::TransparentWidget(w);
	}
}

void PlotCursorReadouts::horizSetVisible(bool visible) { horiz_contents->setVisible(visible); }

void PlotCursorReadouts::vertSetVisible(bool visible) { vert_contents->setVisible(visible); }

bool PlotCursorReadouts::horizIsVisible() { return horiz_contents->isVisible(); }

bool PlotCursorReadouts::vertIsVisible() { return vert_contents->isVisible(); }

void PlotCursorReadouts::setVertUnits(QString unit)
{
	vUnit = unit;
	update();
}

void PlotCursorReadouts::setHorizUnits(QString unit)
{
	hUnit = unit;
	update();
}

void PlotCursorReadouts::setHorizFromatter(PrefixFormatter *formatter)
{
	hFormatter = formatter;
	update();
}

void PlotCursorReadouts::setVertFromatter(PrefixFormatter *formatter)
{
	vFormatter = formatter;
	update();
}

#include "moc_plotcursorreadouts.cpp"
