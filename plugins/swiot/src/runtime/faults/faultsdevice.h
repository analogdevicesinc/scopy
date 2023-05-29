#ifndef FAULTSDEVICE_H
#define FAULTSDEVICE_H

#include <QWidget>
#include <QTextEdit>
#include "ui_faultsdevice.h"
#include "faultsgroup.h"
#include <gui/subsection_separator.hpp>
#include <iio.h>


namespace scopy::swiot {
class FaultsGroup;

class FaultsDevice : public QWidget {
	Q_OBJECT
public:
	explicit FaultsDevice(const QString& name, QString path, struct iio_device* device, struct iio_device* swiot, QWidget* parent = nullptr);
	~FaultsDevice() override;

	void update();

public Q_SLOTS:
	void resetStored();
	void updateExplanations();

private:
	Ui::FaultsDevice *ui;
	QWidget *m_faults_explanation;
	scopy::gui::SubsectionSeparator *m_subsectionSeparator;

	FaultsGroup* m_faultsGroup;
	QVector<QWidget*> m_faultExplanationWidgets;

	QString m_name;
	QString m_path;

	struct iio_device* m_device;
	struct iio_device* m_swiot;

	uint32_t m_faultNumeric;

	void initFaultExplanations();
	void connectSignalsAndSlots();
	void readFaults();
	QMap<int, QString>* getSpecialFaults();

private Q_SLOTS:
	void updateMinimumHeight();
};

} // scopy::swiot

#endif //FAULTSDEVICE_H
