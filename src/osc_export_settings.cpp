#include "osc_export_settings.h"
#include "ui_osc_export_settings.h"

#include <QTreeView>
#include <QStandardItem>

using namespace adiscope;

ExportSettings::ExportSettings(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ExportSettings),
	exportChannels(nullptr),
	nr_channels(0)
{
	ui->setupUi(this);

	exportChannels = new DropdownSwitchList();
	exportChannels->setTitle("Channels");
	exportChannels->setColumnTitle(0, "Name");
	exportChannels->setColumnTitle(1, "Export");
	exportChannels->setMaxVisibleItems(3);

	QTreeView *treeView;
	treeView = static_cast<QTreeView *>(exportChannels->view());
	treeView->header()->resizeSection(0, 80);
	treeView->setContentsMargins(2, 2, 2, 2);

	connect(exportChannels->model(),
			SIGNAL(itemChanged(QStandardItem*)),
			SLOT(onExportChannelChanged(QStandardItem*)));

	ui->dropDownLayout->addWidget(exportChannels);
}

ExportSettings::~ExportSettings()
{
	delete ui;
}

void ExportSettings::addChannel(int id, QString name)
{
	exportChannels->addDropdownElement(QIcon(""), name,
					   QVariant(id));
	nr_channels++;
	QStandardItemModel *model =
			static_cast<QStandardItemModel *>(exportChannels->model());
	model->item(id, 1)->setData(QVariant((int) true),
				    Qt::EditRole);
}

void ExportSettings::removeChannel(int id)
{
	exportChannels->removeItem(id);
	nr_channels--;
}

void ExportSettings::clear()
{
	for (int i = 0; i < nr_channels; ++i)
		exportChannels->removeItem(i);
	nr_channels = 0;
}

void ExportSettings::onExportChannelChanged(QStandardItem *item)
{
	QStandardItemModel *model =
		static_cast<QStandardItemModel *>(exportChannels->model());
	bool en = item->data(Qt::EditRole).toBool();

	if (ui->btnExportAll->isChecked()){
		if (!en){
			ui->btnExportAll->setChecked(false);
			oldSettings.clear();
		}
	} else {
		checkIfAllActivated();
	}
}

void ExportSettings::checkIfAllActivated()
{
	QStandardItemModel *model =
		static_cast<QStandardItemModel *>(exportChannels->model());
	bool ok = true;
	for (int i = 0; i < nr_channels; i++) {
		if (!model->item(i, 1)->data(Qt::EditRole).toBool()){
			ok = false;
			break;
		}
	}
	ui->btnExportAll->setChecked(ok);
}

QPushButton* ExportSettings::getExportButton()
{
	return ui->btnExport;
}

QPushButton* ExportSettings::getExportAllButton()
{
	return ui->btnExportAll;
}

QMap<int, bool> ExportSettings::getExportConfig()
{
	QStandardItemModel *model =
		static_cast<QStandardItemModel *>(exportChannels->model());
	QMap<int, bool> result;
	for (int i = 0; i < nr_channels; i++) {
		result[i] = model->item(i, 1)->data(Qt::EditRole).toBool();
	}
	return result;
}

void ExportSettings::on_btnExportAll_clicked()
{
	QStandardItemModel *model =
		static_cast<QStandardItemModel *>(exportChannels->model());
	if (ui->btnExportAll->isChecked()){
		oldSettings = getExportConfig();
		for (int i = 0; i < model->rowCount(); i++) {
			model->item(i, 1)->setData(QVariant((int) true),
					Qt::EditRole);
		}
	} else {
		if (!oldSettings.empty())
			for (int i = 0; i < model->rowCount(); i++) {
				model->item(i, 1)->setData(QVariant((int) oldSettings[i]),
						Qt::EditRole);
		} else {
			for (int i = 0; i < model->rowCount(); i++) {
				model->item(i, 1)->setData(QVariant((int) false),
						Qt::EditRole);
			}
		}
	}
}

void ExportSettings::enableExportButton(bool on)
{
	ui->btnExport->setEnabled(on);
}

void ExportSettings::disableUIMargins()
{
	ui->verticalLayout_3->setMargin(0);
}

void ExportSettings::setTitleLabelVisible(bool enabled)
{
	ui->label->setVisible(enabled);
}

void ExportSettings::setExportAllButtonLabel(const QString& text)
{
	ui->label_3->setText(text);
}


