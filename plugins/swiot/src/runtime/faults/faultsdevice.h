#ifndef FAULTSDEVICE_H
#define FAULTSDEVICE_H

#include <QWidget>
#include <QTextEdit>
#include "ui_faultsdevice.h"
#include "faultsgroup.h"
#include <gui/subsection_separator.hpp>
#include <iio.h>
#include <iioutil/commandqueue.h>

namespace scopy::swiot {
class FaultsGroup;

class FaultsDevice : public QWidget {
	Q_OBJECT
public:
	explicit FaultsDevice(const QString& name, QString path, struct iio_device* device,
			      struct iio_device* swiot, struct iio_context *context,
			      QWidget* parent = nullptr);
	~FaultsDevice();

	void update();

public Q_SLOTS:
	void resetStored();
	void updateExplanations();

Q_SIGNALS:
	void specialFaultsUpdated(int index, QString channelFunction);

private:
	Ui::FaultsDevice *ui;
	QWidget *m_faults_explanation;
	scopy::gui::SubsectionSeparator *m_subsectionSeparator;

	CommandQueue *m_cmdQueue;

	FaultsGroup* m_faultsGroup;
	QVector<QWidget*> m_faultExplanationWidgets;

	QString m_name;
	QString m_path;

	struct iio_device* m_device;
	struct iio_device* m_swiot;
	struct iio_channel* m_faultsChannel;
	struct iio_context* m_context;

	uint32_t m_faultNumeric;
	Command *m_readFaultCommand;
	QVector<Command*> m_deviceConfigCmds;
	QVector<Command*> m_functionConfigCmds;

	void initFaultExplanations();
	void connectSignalsAndSlots();
	void readFaults();
	void initSpecialFaults();

private Q_SLOTS:
	void updateMinimumHeight();
	void deviceConfigCmdFinished(scopy::Command *cmd);
	void functionConfigCmdFinished(scopy::Command *cmd);
};

} // scopy::swiot

#endif //FAULTSDEVICE_H
