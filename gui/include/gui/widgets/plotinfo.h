#ifndef PLOTINFO_H
#define PLOTINFO_H

#include <QWidget>
#include "utils.h"
#include <QLabel>
#include <stylehelper.h>
#include <plot_utils.hpp>
#include <scopy-gui_export.h>

namespace scopy {

class SCOPY_GUI_EXPORT TimePlotHDivInfo : public QLabel {
	Q_OBJECT
public:
	TimePlotHDivInfo(QWidget *parent = nullptr);
	virtual ~TimePlotHDivInfo();

public Q_SLOTS:
	void update(double val);

private:
	MetricPrefixFormatter *m_mpf;
};

class SCOPY_GUI_EXPORT TimePlotSamplingInfo : public QLabel {
	Q_OBJECT
public:
	TimePlotSamplingInfo(QWidget *parent = nullptr);
	virtual ~TimePlotSamplingInfo();

public Q_SLOTS:
	void update(int ps, int bs, double sr);

private:
	MetricPrefixFormatter *m_mpf;
};


class SCOPY_GUI_EXPORT TimePlotStatusInfo : public QWidget {

};

class SCOPY_GUI_EXPORT TimePlotVDivInfo : public QWidget {

};

}

#endif // PLOTINFO_H
