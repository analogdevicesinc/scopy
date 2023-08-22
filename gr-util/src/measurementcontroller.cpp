#include "measurementcontroller.h"
#include "gui/widgets/measurementlabel.h"
#include <QLoggingCategory>
#include <grtimechanneladdon.h>

Q_LOGGING_CATEGORY(CAT_MEASUREMENT_CONTROLLER, "MeasurementController");


namespace scopy::grutil {

MeasurementController::MeasurementController(QPen pen, MeasureModel *msr, QObject *parent)
	: QObject(parent),
	m_measure(msr),
	m_pen(pen)
{
	connect(m_measure, &MeasureModel::newMeasurementsAvailable, this, [=]() {
		for(auto lbl : m_labels) {
			lbl->setValue(m_measure->measurement(lbl->name())->value());
		}
	} );
}

MeasurementController::~MeasurementController() {

}

MeasurementLabel *MeasurementController::findMeasurementLabel(QString name)
{
	for(auto &lbl : m_labels) {
		if(lbl->name() == name) {
			return lbl;
		}
	}
	return nullptr;
}



MeasurementLabel* MeasurementController::enableMeasurement(QString name)
{
	for(auto &meas : m_availableMeasurements) {
		if(meas.name == name) {

			if(findMeasurementLabel(name) != nullptr) {
				qWarning() << "Measurement label already added";
				return nullptr;
			}


			MeasurementLabel *lbl = new MeasurementLabel();
			lbl->setName(meas.name);
			lbl->setUnit(meas.unit);
			lbl->setColor(m_pen.color());
			if(meas.formatter == "time") {
				lbl->setMeasurementValueFormatter(new TimePrefixFormatter(lbl));
			} else if(meas.formatter == "metric") {
				lbl->setMeasurementValueFormatter(new MetricPrefixFormatter(lbl));
			}

			m_labels.append(lbl);



			qInfo() << "Measurement " << name << "added";
			Q_EMIT measurementEnabled(lbl);
			return lbl;
		}
	}
	return nullptr;
}

void MeasurementController::disableMeasurement(QString name)
{
	auto lbl = findMeasurementLabel(name);
	Q_EMIT measurementDisabled(lbl);

	if(lbl == nullptr) {
		qWarning() << "Measurement label was not added to the panel";
		return;
	}
	m_labels.removeAll(lbl);
	delete lbl;
	qInfo() << "Measurement " << name << "removed";
}

void MeasurementController::addMeasurement(MeasurementInfo v)
{
	m_availableMeasurements.push_back(v);
}

QList<MeasurementInfo> MeasurementController::availableMeasurements() const
{
	return m_availableMeasurements;
}

TimeChannelMeasurementController::TimeChannelMeasurementController(TimeMeasureModel* msr, QPen m_pen, QObject *parent)
	: MeasurementController(m_pen, msr, parent) {
	addMeasurement({"Period",":/gui/icons/measurements/period.svg","", "time", "Horizontal"});
	addMeasurement({"Frequency",":/gui/icons/measurements/frequency.svg","Hz", "metric", "Horizontal"});
	addMeasurement({"Min",":/gui/icons/measurements/min.svg","V", "metric", "Vertical"});
	addMeasurement({"Max",":/gui/icons/measurements/max.svg","V", "metric", "Vertical"});
	addMeasurement({"Peak-Peak",":/gui/icons/measurements/peak_to_peak.svg","V", "metric", "Vertical"});
	addMeasurement({"Cycle Mean",":/gui/icons/measurements/cycle_mean.svg","V", "metric", "Vertical"});
//	addMeasurement({"RMS",":/gui/icons/measurements/rms.svg","V", "metric", "Vertical"});
//	addMeasurement({"Cycle RMS",":/gui/icons/measurements/cycle_rms.svg","V", "metric", "Vertical"});
//	addMeasurement({"AC RMS",":/gui/icons/measurements/rms.svg","V", "metric", "Vertical"});
//	addMeasurement({"Area",":/gui/icons/measurements/area.svg","Vs", "metric", "Vertical"});
//	addMeasurement({"Cycle Area",":/gui/icons/measurements/cycle_area.svg","Vs", "metric", "Vertical"});
//	addMeasurement({"Low",":/gui/icons/measurements/low.svg","V", "metric", "Vertical"});
//	addMeasurement({"High",":/gui/icons/measurements/high.svg","V", "metric", "Vertical"});
//	addMeasurement({"Amplitude",":/gui/icons/measurements/amplitude.svg","V", "metric", "Vertical"});
//	addMeasurement({"Middle",":/gui/icons/measurements/middle.svg","V", "metric", "Vertical"});
//	addMeasurement({"+Over",":/gui/icons/measurements/p_overshoot.svg","%", "metric", "Vertical"});
//	addMeasurement({"-Over",":/gui/icons/measurements/n_overshoot.svg","%", "metric", "Vertical"});
//	addMeasurement({"Rise",":/gui/icons/measurements/rise_time.svg","s", "metric", "Horizontal"});
//	addMeasurement({"Fall",":/gui/icons/measurements/fall_time.svg","s", "metric", "Horizontal"});
//	addMeasurement({"+Width",":/gui/icons/measurements/p_width.svg","s", "metric", "Horizontal"});
//	addMeasurement({"-Width",":/gui/icons/measurements/n_width.svg","s", "metric", "Horizontal"});
//	addMeasurement({"+Duty",":/gui/icons/measurements/p_duty.svg","%", "metric", "Horizontal"});
//	addMeasurement({"-Duty",":/gui/icons/measurements/n_duty.svg","%", "metric", "Horizontal"});
}
/*

static const std::map<int, QString> icons_spect = {
	{M2kMeasure::NOISE_FLOOR, ":/gui/icons/measurements/period.svg"},
	{M2kMeasure::SINAD, ":/gui/icons/measurements/frequency.svg"},
	{M2kMeasure::SNR, ":/gui/icons/measurements/frequency.svg"},
	{M2kMeasure::THD, ":/gui/icons/measurements/frequency.svg"},
	{M2kMeasure::THDN, ":/gui/icons/measurements/frequency.svg"},
	{M2kMeasure::SFDR, ":/gui/icons/measurements/frequency.svg"},
	};
*/

}
