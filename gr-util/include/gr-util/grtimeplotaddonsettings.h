#ifndef GRTIMEPLOTADDONSETTINGS_H
#define GRTIMEPLOTADDONSETTINGS_H

#include "tooladdon.h"
#include "scopy-gr-util_export.h"
#include <QLabel>
#include "grtimeplotaddon.h"

namespace scopy::grutil {
class SCOPY_GR_UTIL_EXPORT GRTimePlotAddonSettings : public QObject, public ToolAddon {
	Q_OBJECT
public:
	GRTimePlotAddonSettings(GRTimePlotAddon *p, QObject *parent = nullptr) :
		  QObject(parent),p(p) {
		name = p->getName()+"_settings";
		widget = new QLabel(name);
	}
	~GRTimePlotAddonSettings() {}

	QString getName() override { return name;}
	QWidget* getWidget() override { return widget;}

public Q_SLOTS:
	void enable() override {}
	void disable() override {}
	void onStart() override {}
	void onStop() override {}
	void onAdd() override {}
	void onRemove() override {}
	void onChannelAdded(ToolAddon*) override {}
	void onChannelRemoved(ToolAddon*) override {}


private:
	GRTimePlotAddon* p;
	QString name;
	QWidget *widget;
};
}

#endif // GRTIMEPLOTADDONSETTINGS_H
