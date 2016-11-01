#include "measure_settings.h"
#include "oscilloscope_plot.hpp"
#include "dropdown_switch_list.h"
#include "ui_measure_settings.h"

#include <QStandardItem>
#include <QTreeView>

using namespace adiscope;

MeasureSettings::MeasureSettings(CapturePlot *plot, QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::MeasureSettings),
	m_channelName(""),
	m_chnUnderlineColor(),
	m_horizMeasurements(new DropdownSwitchList(1, this)),
	m_vertMeasurements(new DropdownSwitchList(1, this)),
	m_emitActivated(true),
	m_emitStatsChanged(true),
	m_emitDeleteAll(true),
	m_are_dropdowns_filled(false),
	m_plot(plot),
	m_selectedChannel(-1)
{
	QTreeView *treeView;

	m_ui->setupUi(this);
	m_ui->hLayout_hMeasurements->addWidget(m_horizMeasurements);
	m_ui->hLayout_vMeasurements->addWidget(m_vertMeasurements);

	m_horizMeasurements->setTitle("Horizontal");
	m_horizMeasurements->setColumnTitle(0, "Name");
	m_horizMeasurements->setColumnTitle(1, "Active");
	m_horizMeasurements->setColumnTitle(2, "Stats");
	m_horizMeasurements->setMaxVisibleItems(4);
	treeView = static_cast<QTreeView *>(m_horizMeasurements->view());
	treeView->header()->resizeSection(0, 160);
	treeView->setIconSize(QSize(24, 24));

	connect(m_horizMeasurements->model(),
		SIGNAL(itemChanged(QStandardItem*)),
		SLOT(onMeasurementPropertyChanged(QStandardItem*)));

	m_vertMeasurements->setTitle("Vertical");
	m_vertMeasurements->setColumnTitle(0, "Name");
	m_vertMeasurements->setColumnTitle(1, "Active");
	m_vertMeasurements->setColumnTitle(2, "Stats");
	m_vertMeasurements->setMaxVisibleItems(4);
	treeView = static_cast<QTreeView *>(m_vertMeasurements->view());
	treeView->header()->resizeSection(0, 160);
	treeView->setIconSize(QSize(24, 24));

	connect(m_vertMeasurements->model(),
		SIGNAL(itemChanged(QStandardItem*)),
		SLOT(onMeasurementPropertyChanged(QStandardItem*)));
}

QString MeasureSettings::channelName() const
{
	return m_channelName;
}

void MeasureSettings::setChannelName(const QString& name)
{
	m_channelName = name;
	m_ui->lblChanName->setText(name);
}

QColor MeasureSettings::channelUnderlineColor() const
{
	return m_chnUnderlineColor;
}

void MeasureSettings::setChannelUnderlineColor(const QColor& color)
{
	m_chnUnderlineColor = color;
	QString stylesheet = QString("border: 2px solid %1;"
				).arg(color.name());

	m_ui->line->setStyleSheet(stylesheet);
}

void MeasureSettings::addHorizontalMeasurement(const QString& name,
	int measurement_id)
{
	setEmitActivated(false);
	m_horizMeasurements->addDropdownElement(QIcon(), name,
		QVariant(measurement_id));
	setEmitActivated(true);
}

void MeasureSettings::addVerticalMeasurement(const QString& name,
	int measurement_id)
{
	setEmitActivated(false);
	m_vertMeasurements->addDropdownElement(QIcon(), name,
		QVariant(measurement_id));
	setEmitActivated(true);
}

void MeasureSettings::setHorizMeasurementActive(int idx, bool en)
{
	QStandardItemModel *model = static_cast<QStandardItemModel *>(
					m_horizMeasurements->model());
	int val = en ? 1 : 0;

	model->item(idx, 1)->setData(QVariant(val), Qt::EditRole);
}

void MeasureSettings::setVertMeasurementActive(int idx, bool en)
{
	QStandardItemModel *model = static_cast<QStandardItemModel *>(
					m_vertMeasurements->model());
	int val = en ? 1 : 0;

	model->item(idx, 1)->setData(QVariant(val), Qt::EditRole);
}

bool MeasureSettings::emitActivated() const
{
	return m_emitActivated;
}
void MeasureSettings::setEmitActivated(bool en)
{
	m_emitActivated = en;
}

bool MeasureSettings::emitStatsChanged()const
{
	return m_emitStatsChanged;
}
void MeasureSettings::setEmitStatsChanged(bool en)
{
	m_emitStatsChanged = en;
}

QList<MeasurementItem> MeasureSettings::measurementSelection()
{
	return m_selectedMeasurements;
}

void MeasureSettings::onMeasurementPropertyChanged(QStandardItem *item)
{
	bool en = item->data(Qt::EditRole).toBool();
	QStandardItem *nameItem = item->model()->item(item->row(), 0);
	QString name = nameItem->data(Qt::EditRole).toString();
	int id = nameItem->data(Qt::UserRole).toInt();

	if (item->column() == 1)
		if (m_emitActivated) {
			onMeasurementActivated(id, en);
		}
	else if (item->column() == 2)
		if (m_emitStatsChanged)
			Q_EMIT measurementStatsChanged(name, en);

	// Switch from Recover to Delete All if a measurement state gets changed
	if (m_emitActivated && m_ui->button_measDeleteAll->isChecked()) {
		m_emitDeleteAll = false;
		m_ui->button_measDeleteAll->setChecked(false);
	}

	// Disable Display All if a measurement state gets changed
	if (m_emitActivated && m_ui->button_measDisplayAll->isChecked()) {
			m_ui->button_measDisplayAll->setChecked(false);
	}
}

void MeasureSettings::on_button_measDisplayAll_toggled(bool checked)
{
	m_ui->button_measDeleteAll->setEnabled(!checked);

	if (checked)
		displayAllMeasurements();
	else
		disableDisplayAllMeasurements();
}

void MeasureSettings::on_button_measDeleteAll_toggled(bool checked)
{
	QPushButton *button = static_cast<QPushButton*>(QObject::sender());
	if (checked)
		button->setText("Recover");
	else
		button->setText("Delete All");

	if (!m_emitDeleteAll) {
		m_emitDeleteAll = true;
	} else {
		if (checked)
			deleteAllMeasurements();
		else
			recoverAllMeasurements();
	}
}

void MeasureSettings::onChannelAdded(int chnIdx)
{
	// Use the measurements of the 1st channel to construct the dropdowns.
	// All channels have the same set of measurements.
	if (!m_are_dropdowns_filled) {
		buildDropdownElements(chnIdx);
		m_are_dropdowns_filled = true;
	}
}

void MeasureSettings::onChannelRemoved(int chnIdx)
{
	QMutableListIterator<MeasurementItem> i(m_selectedMeasurements);
	while (i.hasNext())
	{
		i.next();
		if (i.value().channel_id() == chnIdx) {
			int id = i.value().id();
			int chn_id = i.value().channel_id();
			i.remove();
			Q_EMIT measurementDeactivated(id, chn_id);
		}
	}
}

void MeasureSettings::setSelectedChannel(int chnIdx)
{
	if (m_selectedChannel != chnIdx) {
		m_selectedChannel = chnIdx;
		loadMeasurementStatesFromData();
	}
}

void MeasureSettings::buildDropdownElements(int chnIdx)
{
	auto measurements = m_plot->measurements(chnIdx);

	for (int i = 0; i < measurements.size(); i++) {
		if (measurements[i]->axis() == MeasurementData::HORIZONTAL)
			addHorizontalMeasurement(measurements[i]->name(), i);
		else if (measurements[i]->axis() == MeasurementData::VERTICAL)
			addVerticalMeasurement(measurements[i]->name(), i);
	}
}

void MeasureSettings::loadMeasurementStatesFromData()
{
	auto measurements = m_plot->measurements(m_selectedChannel);
	int h_idx = 0;
	int v_idx = 0;

	m_emitActivated = false;
	for (int i = 0; i < measurements.size(); i++) {
		int axis = measurements[i]->axis();
		int state = measurements[i]->enabled();

		if (axis == MeasurementData::HORIZONTAL)
			setHorizMeasurementActive(h_idx++, state);
		else if (axis == MeasurementData::VERTICAL)
			setVertMeasurementActive(v_idx++, state);
	}
	m_emitActivated = true;
}

void MeasureSettings::deleteAllMeasurements()
{
	m_deleteAllBackup = m_selectedMeasurements;
	m_selectedMeasurements.clear();
	Q_EMIT measurementSelectionListChanged();
	loadMeasurementStatesFromData();
}

void MeasureSettings::recoverAllMeasurements()
{
	m_selectedMeasurements = m_deleteAllBackup;
	m_deleteAllBackup.clear();
	Q_EMIT measurementSelectionListChanged();
	loadMeasurementStatesFromData();
}

void MeasureSettings::displayAllMeasurements()
{
	m_displayAllBackup = m_selectedMeasurements;
	m_selectedMeasurements.clear();
	auto measurements = m_plot->measurements(m_selectedChannel);
	for (int i = 0; i < measurements.size(); i++) {
		MeasurementItem item(i, measurements[i]->channel());
		m_selectedMeasurements.push_back(MeasurementItem(item));
	}
	Q_EMIT measurementSelectionListChanged();
	loadMeasurementStatesFromData();
}

void MeasureSettings::disableDisplayAllMeasurements()
{
	m_selectedMeasurements = m_displayAllBackup;
	Q_EMIT measurementSelectionListChanged();
	loadMeasurementStatesFromData();
}

void MeasureSettings::onMeasurementActivated(int id, bool en)
{
	if (m_selectedChannel < 0)
		return;

	auto measurements = m_plot->measurements(m_selectedChannel);
	MeasurementItem mItem(id, measurements[id]->channel());
	if (en) {
		m_selectedMeasurements.push_back(mItem);
		Q_EMIT measurementActivated(mItem.id(), mItem.channel_id());
	} else {
		m_selectedMeasurements.removeOne(mItem);
		Q_EMIT measurementDeactivated(mItem.id(), mItem.channel_id());
	}
}
