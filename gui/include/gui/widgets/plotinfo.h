#ifndef PLOTINFO_H
#define PLOTINFO_H

#include "hoverwidget.h"
#include <QWidget>
#include <scopy-gui_export.h>

class QLabel;
namespace scopy {

enum InfoPosition
{
	IP_LEFT,
	IP_RIGHT
};

class SCOPY_GUI_EXPORT PlotInfo : public QWidget
{
	Q_OBJECT
public:
	PlotInfo(QWidget *parent = nullptr);
	virtual ~PlotInfo();

	void addCustomInfo(QWidget *info, InfoPosition pos);
	QLabel *addLabelInfo(InfoPosition pos);

	void removeInfo(uint index, InfoPosition pos);
	QWidget *getInfo(uint index, InfoPosition pos);

protected:
	void initLayouts();

private:
	QWidget *m_parent;
	int m_margin;
	int m_spacing;

	QWidget *m_leftInfo;
	HoverWidget *m_leftHover;
	QVBoxLayout *m_leftLayout;

	QWidget *m_rightInfo;
	HoverWidget *m_rightHover;
	QVBoxLayout *m_rightLayout;
};

} // namespace scopy

#endif // PLOTINFO_H
