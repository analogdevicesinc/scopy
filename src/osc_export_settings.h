#ifndef OSC_EXPORT_SETTINGS_H
#define OSC_EXPORT_SETTINGS_H

/* Qt includes */
#include <QWidget>
#include <QString>
#include <QStandardItem>
#include <QPushButton>
#include <QMap>

/* Local includes */
#include "dropdown_switch_list.h"

namespace Ui {
class ExportSettings;
}

namespace adiscope {
class ExportSettings : public QWidget
{
	Q_OBJECT

public:
	explicit ExportSettings(QWidget *parent = 0);
	~ExportSettings();

public Q_SLOTS:
	void addChannel(int id, QString name);
	void removeChannel(int id);
	void onExportChannelChanged(QStandardItem*);

	QPushButton *getExportButton();
	QMap<int, bool> getExportConfig();
	void on_btnExportAll_clicked();
	void enableExportButton(bool on);
	void disableUIMargins();

private:
	Ui::ExportSettings *ui;
	DropdownSwitchList *exportChannels;
	void checkIfAllActivated();
	int nr_channels;
	QMap<int, bool> oldSettings;
};
}

#endif // OSC_EXPORT_SETTINGS_H
