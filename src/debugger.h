#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iio.h>

/* Qt includes */
#include <QMainWindow>

/* Local includes */
#include "debug.h"
#include "bitfieldwidget.h"
#include "registerwidget.h"
#include "filter.hpp"
#include "tool.hpp"

class QJSEngine;

namespace Ui {
class Debugger;
}

namespace adiscope {


class Debugger : public Tool
{
	Q_OBJECT

public:
	explicit Debugger(struct iio_context *ctx, Filter *filt,
	                  QJSEngine *engine, ToolLauncher *parent = 0);
	~Debugger();

public Q_SLOTS:
	void updateChannelComboBox(QString devName);
	void updateAttributeComboBox(QString channel);
	void updateFilename(int index);
	void updateValueWidget(QString attribute);
	void updateRegMap(void);

	void on_ReadButton_clicked();
	void on_WriteButton_clicked();

	void on_readRegPushButton_clicked();
	void on_writeRegPushButton_clicked();

private Q_SLOTS:
	void on_detailedRegMapCheckBox_stateChanged(int arg1);

private:
	void updateSources(void);

private:
	Ui::Debugger *ui;

	Debug debug;

	RegisterWidget *reg;
	QVector<BitfieldWidget *> bitfieldsVector;
};
}

#endif // MAINWINDOW_H
