#include "auxadcdaciowidget.h"

#include <style.h>
#include <iioutil/connectionprovider.h>
#include <iiowidgetbuilder.h>

using namespace scopy;
using namespace pluto;

AuxAdcDacIoWidget::AuxAdcDacIoWidget(QString uri, QWidget *parent)
	: m_uri(uri)
	, QWidget{parent}
{

	m_layout = new QVBoxLayout(this);
	setLayout(m_layout);

	QWidget *auxAdcDacIoWidget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(auxAdcDacIoWidget);
	auxAdcDacIoWidget->setLayout(layout);
	layout->setMargin(0);

	QScrollArea *scrollArea = new QScrollArea(parent);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(auxAdcDacIoWidget);

	// Get connection to device
	Connection *conn = ConnectionProvider::GetInstance()->open(m_uri);
	// iio:device0: ad9361-phy
	m_device = iio_context_find_device(conn->context(), "ad9361-phy");

	layout->addWidget(tempSensorWidget(auxAdcDacIoWidget));
	layout->addWidget(auxAdcWidget(auxAdcDacIoWidget));
	layout->addWidget(auxDacWidget(auxAdcDacIoWidget));
	layout->addWidget(controlsOutWidget(auxAdcDacIoWidget));
	layout->addWidget(gposWidget(auxAdcDacIoWidget));

	layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
	m_layout->addWidget(scrollArea);
}

AuxAdcDacIoWidget::~AuxAdcDacIoWidget() { ConnectionProvider::close(m_uri); }

QWidget *AuxAdcDacIoWidget::tempSensorWidget(QWidget *parent)
{
	QWidget *tempSensorWidget = new QWidget(parent);
	QVBoxLayout *tempSensorLayout = new QVBoxLayout(tempSensorWidget);
	tempSensorWidget->setLayout(tempSensorLayout);

	Style::setStyle(tempSensorWidget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(tempSensorWidget, json::theme::background_primary);

	tempSensorLayout->addWidget(new QLabel("Temp Snesor", tempSensorWidget));

	// adi,temp-sense-measurement-interval-ms
	IIOWidget *tempSenseMeasurementInterval = IIOWidgetBuilder(tempSensorWidget)
							  .device(m_device)
							  .attribute("adi,temp-sense-measurement-interval-ms")
							  .buildSingle();
	tempSensorLayout->addWidget(tempSenseMeasurementInterval);

	// adi,temp-sense-offset-signed
	IIOWidget *tempSenseOffset = IIOWidgetBuilder(tempSensorWidget)
					     .device(m_device)
					     .attribute("adi,temp-sense-offset-signed")
					     .buildSingle();
	tempSensorLayout->addWidget(tempSenseOffset);

	// adi,temp-sense-decimation
	IIOWidget *tempSenseDecimation = IIOWidgetBuilder(tempSensorWidget)
						 .device(m_device)
						 .attribute("adi,temp-sense-decimation")
						 .buildSingle();
	tempSensorLayout->addWidget(tempSenseDecimation);

	// adi,temp-sense-periodic-measurement-enable
	IIOWidget *tempSensePeriodicMeasurement = IIOWidgetBuilder(tempSensorWidget)
							  .device(m_device)
							  .attribute("adi,temp-sense-periodic-measurement-enable")
							  .buildSingle();
	tempSensorLayout->addWidget(tempSensePeriodicMeasurement);

	return tempSensorWidget;
}

QWidget *AuxAdcDacIoWidget::auxAdcWidget(QWidget *parent)
{
	QWidget *auxAdcWidget = new QWidget(parent);
	QVBoxLayout *auxAdcWidgetLayout = new QVBoxLayout(auxAdcWidget);
	auxAdcWidget->setLayout(auxAdcWidgetLayout);

	Style::setStyle(auxAdcWidget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(auxAdcWidget, json::theme::background_primary);

	auxAdcWidgetLayout->addWidget(new QLabel("Aux ADC", auxAdcWidget));

	// adi,aux-adc-rate
	IIOWidget *auxAdcRate =
		IIOWidgetBuilder(auxAdcWidget).device(m_device).attribute("adi,aux-adc-rate").buildSingle();
	auxAdcWidgetLayout->addWidget(auxAdcRate);

	// adi,aux-adc-decimation
	IIOWidget *auxAdcDecimation =
		IIOWidgetBuilder(auxAdcWidget).device(m_device).attribute("adi,aux-adc-decimation").buildSingle();
	auxAdcWidgetLayout->addWidget(auxAdcDecimation);

	return auxAdcWidget;
}

QWidget *AuxAdcDacIoWidget::auxDacWidget(QWidget *parent)
{

	QWidget *auxDacWidget = new QWidget(parent);
	QVBoxLayout *auxDacWidgetLayout = new QVBoxLayout(auxDacWidget);
	auxDacWidget->setLayout(auxDacWidgetLayout);

	Style::setStyle(auxDacWidget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(auxDacWidget, json::theme::background_primary);

	// adi,aux-dac-manual-mode-enable

	IIOWidget *auxDacManualMode = IIOWidgetBuilder(auxDacWidget)
					      .device(m_device)
					      .attribute("adi,aux-dac-manual-mode-enable")
					      .buildSingle();
	auxDacWidgetLayout->addWidget(auxDacManualMode);

	// getAuxAdcDac
	QHBoxLayout *auxDacLayout = new QHBoxLayout();

	// TODO add check for dac counter
	auxDacLayout->addWidget(getAuxDac("1", auxDacWidget));
	auxDacLayout->addWidget(getAuxDac("2", auxDacWidget));

	auxDacWidgetLayout->addLayout(auxDacLayout);

	return auxDacWidget;
}

QWidget *AuxAdcDacIoWidget::getAuxDac(QString dacx, QWidget *parent)
{
	QWidget *auxDacWidget = new QWidget(parent);
	QGridLayout *layout = new QGridLayout(auxDacWidget);
	auxDacWidget->setLayout(layout);

	Style::setStyle(auxDacWidget, style::properties::widget::border_interactive);

	QLabel *dacLabel = new QLabel("DAC" + dacx, auxDacWidget);
	layout->addWidget(dacLabel, 0, 0);

	// Get connection to device
	Connection *conn = ConnectionProvider::GetInstance()->open(m_uri);
	// iio:device0: ad9361-phy
	iio_device *m_device = iio_context_find_device(conn->context(), "ad9361-phy");

	layout->addWidget(new QLabel("Default Value (mV)", auxDacWidget), 1, 0);

	// adi,aux-dacx-default-value-mV
	IIOWidget *dacDefaultVlue = IIOWidgetBuilder(auxDacWidget)
					    .device(m_device)
					    .attribute("adi,aux-dac" + dacx + "-default-value-mV")
					    .buildSingle();
	layout->addWidget(dacDefaultVlue, 1, 1);

	// adi,aux-dacx-active-in-alert-enable
	IIOWidget *dacActiveInAlert = IIOWidgetBuilder(auxDacWidget)
					      .device(m_device)
					      .attribute("adi,aux-dac" + dacx + "-active-in-alert-enable")
					      .buildSingle();
	layout->addWidget(dacActiveInAlert, 2, 0);

	layout->addWidget(new QLabel("RX/TX Delay (us)", auxDacWidget), 2, 1);

	// adi,aux-dacx-active-in-rx-enable
	IIOWidget *dacActiveInRx = IIOWidgetBuilder(auxDacWidget)
					   .device(m_device)
					   .attribute("adi,aux-dac" + dacx + "-active-in-rx-enable")
					   .buildSingle();
	layout->addWidget(dacActiveInRx, 3, 0);

	// adi,aux-dacx-active-in-tx-enable
	IIOWidget *dacActiveInTx = IIOWidgetBuilder(auxDacWidget)
					   .device(m_device)
					   .attribute("adi,aux-dac" + dacx + "-active-in-tx-enable")
					   .buildSingle();
	layout->addWidget(dacActiveInTx, 4, 0);

	// adi,aux-dacx-rx-delay-us
	IIOWidget *rxDelay = IIOWidgetBuilder(auxDacWidget)
				     .device(m_device)
				     .attribute("adi,aux-dac" + dacx + "-rx-delay-us")
				     .buildSingle();
	layout->addWidget(rxDelay, 3, 1);

	// adi,aux-dacx-tx-delay-us
	IIOWidget *txDelay = IIOWidgetBuilder(auxDacWidget)
				     .device(m_device)
				     .attribute("adi,aux-dac" + dacx + "-tx-delay-us")
				     .buildSingle();
	layout->addWidget(txDelay, 4, 1);

	return auxDacWidget;
}

QWidget *AuxAdcDacIoWidget::controlsOutWidget(QWidget *parent)
{
	QWidget *controlsOutWidget = new QWidget(parent);
	QVBoxLayout *controlsOutWidgetLayout = new QVBoxLayout(controlsOutWidget);
	controlsOutWidget->setLayout(controlsOutWidgetLayout);

	Style::setStyle(controlsOutWidget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(controlsOutWidget, json::theme::background_primary);

	controlsOutWidgetLayout->addWidget(new QLabel("Control OUTS", controlsOutWidget));

	// adi,ctrl-outs-index
	IIOWidget *ctrlOutsIndex =
		IIOWidgetBuilder(controlsOutWidget).device(m_device).attribute("adi,ctrl-outs-index").buildSingle();
	controlsOutWidgetLayout->addWidget(ctrlOutsIndex);

	// adi,ctrl-outs-enable-mask
	IIOWidget *ctrlOutsMask = IIOWidgetBuilder(controlsOutWidget)
					  .device(m_device)
					  .attribute("adi,ctrl-outs-enable-mask")
					  .buildSingle();
	controlsOutWidgetLayout->addWidget(ctrlOutsMask);

	return controlsOutWidget;
}

QWidget *AuxAdcDacIoWidget::gposWidget(QWidget *parent)
{

	QWidget *gposWidget = new QWidget(parent);
	QVBoxLayout *gposWidgetLayout = new QVBoxLayout(gposWidget);
	gposWidget->setLayout(gposWidgetLayout);

	Style::setStyle(gposWidget, style::properties::widget::border_interactive);
	Style::setBackgroundColor(gposWidget, json::theme::background_primary);

	// adi,gpo-manual-mode-enable
	IIOWidget *gpoManualMode =
		IIOWidgetBuilder(parent).device(m_device).attribute("adi,gpo-manual-mode-enable").buildSingle();
	gposWidgetLayout->addWidget(gpoManualMode);

	for(int i = 0; i < 4; i++) {
		gposWidgetLayout->addWidget(gpoWidget(QString::number(i), parent));
	}

	return gposWidget;
}

QWidget *AuxAdcDacIoWidget::gpoWidget(QString gpox, QWidget *parent)
{
	MenuSectionWidget *gpoContainer = new MenuSectionWidget(parent);

	Style::setStyle(gpoContainer, style::properties::widget::border_interactive);

	MenuCollapseSection *gpoSection = new MenuCollapseSection("GPO " + gpox, MenuCollapseSection::MHCW_NONE,
								  MenuCollapseSection::MHW_BASEWIDGET, gpoContainer);

	gpoContainer->contentLayout()->addWidget(gpoSection);

	QWidget *gpoContent = new QWidget(gpoSection);
	QGridLayout *layout = new QGridLayout(gpoContent);
	gpoContent->setLayout(layout);

	// TODO FIND WHAT THIS DOES
	// adi,elna-rx1-gpo0-control-enable
	// IIOWidget *elnaRxGpoControl = IIOWidgetBuilder(gpoContent)
	//                                         .device(m_device)
	//                                         .attribute("adi,elna-rx1-gpo" +gpox+"-control-enable")
	//                                         .buildSingle();
	// layout->addWidget(interactiveState, 0, 0);

	// adi,gpoX-inactive-state-high-enable
	IIOWidget *interactiveState = IIOWidgetBuilder(gpoContent)
					      .device(m_device)
					      .attribute("adi,gpo" + gpox + "-inactive-state-high-enable")
					      .buildSingle();
	layout->addWidget(interactiveState, 0, 0);

	// adi,gpoX-slave-rx-enable
	IIOWidget *stateRx = IIOWidgetBuilder(gpoContent)
				     .device(m_device)
				     .attribute("adi,gpo" + gpox + "-slave-rx-enable")
				     .buildSingle();
	layout->addWidget(stateRx, 1, 0);

	// adi,gpoX-slave-tx-enable
	IIOWidget *stateTx = IIOWidgetBuilder(gpoContent)
				     .device(m_device)
				     .attribute("adi,gpo" + gpox + "-slave-tx-enable")
				     .buildSingle();
	layout->addWidget(stateTx, 2, 0);

	QLabel *rxTxDelay = new QLabel("RX/TX Delay (us)", gpoContent);
	layout->addWidget(rxTxDelay, 0, 1);

	// adi,gpoX-rx-delay-us
	IIOWidget *rxDelay = IIOWidgetBuilder(gpoContent)
				     .device(m_device)
				     .attribute("adi,gpo" + gpox + "-rx-delay-us")
				     .buildSingle();
	layout->addWidget(rxDelay, 1, 1);

	// adi,gpoX-tx-delay-us
	IIOWidget *txDelay = IIOWidgetBuilder(gpoContent)
				     .device(m_device)
				     .attribute("adi,gpo" + gpox + "-tx-delay-us")
				     .buildSingle();
	layout->addWidget(txDelay, 2, 1);

	gpoSection->contentLayout()->addWidget(gpoContent);

	return gpoContainer;
}
