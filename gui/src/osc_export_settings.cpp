/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "ui_osc_export_settings.h"

#include <QHeaderView>
#include <QStandardItem>
#include <QTreeView>

#include <scopy/gui/osc_export_settings.hpp>

using namespace scopy::gui;

ExportSettings::ExportSettings(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::ExportSettings)
	, m_exportChannels(nullptr)
	, m_nrChannels(0)
{
	m_ui->setupUi(this);

	m_exportChannels = new DropdownSwitchList();
	m_exportChannels->setTitle(tr("Channels"));
	m_exportChannels->setColumnTitle(0, tr("Name"));
	m_exportChannels->setColumnTitle(1, tr("Export"));
	m_exportChannels->setMaxVisibleItems(3);

	QTreeView* treeView;
	treeView = static_cast<QTreeView*>(m_exportChannels->view());
	treeView->header()->resizeSection(0, 80);
	treeView->setContentsMargins(2, 2, 2, 2);

	connect(m_exportChannels->model(), SIGNAL(itemChanged(QStandardItem*)),
		SLOT(onExportChannelChanged(QStandardItem*)));

	m_ui->vLayoutDropDown->addWidget(m_exportChannels);

	m_ui->widgetSubsSeparator->setLabel("EXPORT");
	m_ui->widgetSubsSeparator->setButtonVisible(false);
}

ExportSettings::~ExportSettings() { delete m_ui; }

void ExportSettings::addChannel(int id, QString name)
{
	m_exportChannels->addDropdownElement(QIcon(""), name, QVariant(id));
	m_nrChannels++;
	QStandardItemModel* model = static_cast<QStandardItemModel*>(m_exportChannels->model());
	model->item(id, 1)->setData(QVariant((int)true), Qt::EditRole);
}

void ExportSettings::removeChannel(int id)
{
	m_exportChannels->removeItem(id);
	m_nrChannels--;
}

void ExportSettings::clear()
{
	for (int i = 0; i < m_nrChannels; ++i)
		m_exportChannels->removeItem(i);
	m_nrChannels = 0;
}

void ExportSettings::onExportChannelChanged(QStandardItem* item)
{
	bool en = item->data(Qt::EditRole).toBool();

	if (m_ui->btnExportAll->isChecked()) {
		if (!en) {
			m_ui->btnExportAll->setChecked(false);
			m_oldSettings.clear();
		}
	} else {
		checkIfAllActivated();
	}
}

void ExportSettings::checkIfAllActivated()
{
	QStandardItemModel* model = static_cast<QStandardItemModel*>(m_exportChannels->model());
	bool ok = true;
	for (int i = 0; i < m_nrChannels; i++) {
		if (!model->item(i, 1)->data(Qt::EditRole).toBool()) {
			ok = false;
			break;
		}
	}
	m_ui->btnExportAll->setChecked(ok);
}

QPushButton* ExportSettings::getExportButton() { return m_ui->btnExport; }

QPushButton* ExportSettings::getExportAllButton() { return m_ui->btnExportAll; }

QMap<int, bool> ExportSettings::getExportConfig()
{
	QStandardItemModel* model = static_cast<QStandardItemModel*>(m_exportChannels->model());
	QMap<int, bool> result;
	for (int i = 0; i < m_nrChannels; i++) {
		result[i] = model->item(i, 1)->data(Qt::EditRole).toBool();
	}
	return result;
}

void ExportSettings::setExportConfig(QMap<int, bool> config)
{
	QStandardItemModel* model = static_cast<QStandardItemModel*>(m_exportChannels->model());
	for (int key : config.keys()) {
		model->item(key, 1)->setData(QVariant((int)config[key]), Qt::EditRole);
	}
}

void ExportSettings::onBtnExportAllClicked()
{
	QStandardItemModel* model = static_cast<QStandardItemModel*>(m_exportChannels->model());
	if (m_ui->btnExportAll->isChecked()) {
		m_oldSettings = getExportConfig();
		for (int i = 0; i < model->rowCount(); i++) {
			model->item(i, 1)->setData(QVariant((int)true), Qt::EditRole);
		}
	} else {
		if (!m_oldSettings.empty())
			for (int i = 0; i < model->rowCount(); i++) {
				model->item(i, 1)->setData(QVariant((int)m_oldSettings[i]), Qt::EditRole);
			}
		else {
			for (int i = 0; i < model->rowCount(); i++) {
				model->item(i, 1)->setData(QVariant((int)false), Qt::EditRole);
			}
		}
	}
}

void ExportSettings::enableExportButton(bool on) { m_ui->btnExport->setEnabled(on); }

void ExportSettings::disableUIMargins() { m_ui->vLayout_3->setMargin(0); }

void ExportSettings::setTitleLabelVisible(bool enabled)
{
	m_ui->widgetSubsSeparator->setLabelVisible(enabled);
	m_ui->widgetSubsSeparator->setLineVisible(enabled);
}

void ExportSettings::setExportAllButtonLabel(const QString& text) { m_ui->lbl_3->setText(text); }
