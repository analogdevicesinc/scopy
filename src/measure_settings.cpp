#include "measure_settings.h"
#include "dropdown_switch_list.h"
#include "ui_measure_settings.h"

#include <QStandardItem>
#include <QTreeView>

using namespace adiscope;

MeasureSettings::MeasureSettings(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::MeasureSettings),
	m_channelName(""),
	m_chnUnderlineColor(),
	m_horizMeasurements(new DropdownSwitchList(1, this)),
	m_vertMeasurements(new DropdownSwitchList(1, this)),
	m_emitActivated(true),
	m_emitStatsChanged(true)
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

void MeasureSettings::addHorizontalMeasurement(const QString& name)
{
	setEmitActivated(false);
	m_horizMeasurements->addDropdownElement(QIcon(), name);
	setEmitActivated(true);
}

void MeasureSettings::addVerticalMeasurement(const QString& name)
{
	setEmitActivated(false);
	m_vertMeasurements->addDropdownElement(QIcon(), name);
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

void MeasureSettings::onMeasurementPropertyChanged(QStandardItem *item)
{
	bool en = item->data(Qt::EditRole).toBool();

	QStandardItem *nameItem = item->model()->item(item->row(), 0);
	QString name = nameItem->data(Qt::EditRole).toString();

	if (item->column() == 1)
		if (m_emitActivated)
			Q_EMIT measurementActiveChanged(name, en);
	else if (item->column() == 2)
		if (m_emitStatsChanged)
			Q_EMIT measurementStatsChanged(name, en);
}

void MeasureSettings::setColumnData(QStandardItemModel *model, int column,
		bool en)
{
	int val = en ? 1 : 0;

	for (int i = 0; i < model->rowCount(); i++)
		model->item(i, column)->setData(QVariant(val), Qt::EditRole);
}

void MeasureSettings::setAllMeasurements(int col, bool en)
{
	QStandardItemModel *model;

	model = static_cast<QStandardItemModel *>(m_horizMeasurements->model());
	setColumnData(model, col, en);

	model = static_cast<QStandardItemModel *>(m_vertMeasurements->model());
	setColumnData(model, col, en);
}

void MeasureSettings::on_button_MeasurementsEn_toggled(bool checked)
{
	setAllMeasurements(1, checked);
}
