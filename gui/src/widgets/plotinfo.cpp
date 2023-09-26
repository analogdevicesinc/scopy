#include "plotinfo.h"

using namespace scopy;
TimePlotHDivInfo::TimePlotHDivInfo(QWidget *parent) {
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	StyleHelper::TimePlotHDivInfo(this);
	m_mpf = new MetricPrefixFormatter(this);

}

TimePlotHDivInfo::~TimePlotHDivInfo()
{

}

void TimePlotHDivInfo::update(double val) {
	setText(m_mpf->format(val, "s", 2) + "/div");
}

TimePlotSamplingInfo::TimePlotSamplingInfo(QWidget *parent) {
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	StyleHelper::TimePlotSamplingInfo(this);
	m_mpf = new MetricPrefixFormatter(this);
}

TimePlotSamplingInfo::~TimePlotSamplingInfo()
{

}

void TimePlotSamplingInfo::update(int ps, int bs, double sr) {
	QString text;
	text = QString("%1").arg(m_mpf->format(ps, "samples", 2)).arg(m_mpf->format(bs, "samples", 2));
//	if(sr != 1.0)
		text += QString(" at %2").arg(m_mpf->format(sr, "sps", 2));

	setText(text);
}
