/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "harmonicsinstrument.h"
#include "plotaxis.h"
#include "qheaderview.h"

#include <gui/stylehelper.h>
#include <gui/widgets/menuheader.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/verticalchannelmanager.h>
#include <gui/widgets/menucontrolbutton.h>
#include <QDateTime>
#include <QFileDialog>
#include <menulineedit.h>
#include <menusectionwidget.h>

using namespace scopy::pqm;

HarmonicsInstrument::HarmonicsInstrument(ToolMenuEntry *tme, QString uri, QWidget *parent)
	: QWidget(parent)
	, m_tme(tme)
	, m_uri(uri)
	, m_running(false)
{
	initData();
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QHBoxLayout *instrumentLayout = new QHBoxLayout(this);
	setLayout(instrumentLayout);
	StyleHelper::GetInstance()->initColorMap();

	ToolTemplate *tool = new ToolTemplate(this);
	tool->topContainer()->setVisible(true);
	tool->centralContainer()->setVisible(true);
	tool->topContainerMenuControl()->setVisible(false);
	tool->rightContainer()->setVisible(true);
	tool->setRightContainerWidth(280);
	instrumentLayout->addWidget(tool);

	// central widget components
	m_thdWidget = createThdWidget();
	tool->addWidgetToCentralContainerHelper(m_thdWidget);

	m_table = new QTableWidget(MAX_CHNLS, NUMBER_OF_HARMONICS, this);
	initTable();
	tool->addWidgetToCentralContainerHelper(m_table);

	m_plot = new PlotWidget(this);
	initPlot();
	setupPlotChannels();
	tool->addWidgetToCentralContainerHelper(m_plot);

	// instrument menu
	GearBtn *settingsMenuBtn = new GearBtn(this);
	settingsMenuBtn->setChecked(true);
	tool->rightStack()->add("settings", createSettingsMenu(this));
	connect(settingsMenuBtn, &QPushButton::toggled, this, [=, this](bool b) { tool->openRightContainerHelper(b); });

	m_runBtn = new RunBtn(this);
	m_singleBtn = new SingleShotBtn(this);
	tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(m_singleBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(settingsMenuBtn, TTA_RIGHT);

	connect(m_tme, &ToolMenuEntry::runClicked, m_runBtn, &QAbstractButton::setChecked);
	connect(this, &HarmonicsInstrument::enableTool, m_tme, &ToolMenuEntry::setRunning);
	connect(m_runBtn, &QAbstractButton::toggled, m_singleBtn, &QAbstractButton::setDisabled);
	connect(m_runBtn, SIGNAL(toggled(bool)), this, SLOT(toggleHarmonics(bool)));
	connect(m_singleBtn, &QAbstractButton::toggled, m_runBtn, &QAbstractButton::setDisabled);
	connect(m_singleBtn, SIGNAL(toggled(bool)), this, SLOT(toggleHarmonics(bool)));
}

HarmonicsInstrument::~HarmonicsInstrument()
{
	m_xTime.clear();
	m_yValues.clear();
	m_labels.clear();
	m_plotChnls.clear();
}

void HarmonicsInstrument::showThdWidget(bool show) { m_thdWidget->setVisible(show); }

void HarmonicsInstrument::initData()
{
	for(int i = 0; i < NUMBER_OF_HARMONICS; i++) {
		m_xTime.push_back(i);
	}
	for(const QString &ch : m_chnls) {
		m_yValues[ch] = std::vector<double>(NUMBER_OF_HARMONICS, 0);
	}
}

void HarmonicsInstrument::initTable()
{
	m_table->setVerticalHeaderLabels(m_chnls.keys());
	m_table->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
	m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_table->setFixedHeight(220);
	// Init horizontal header
	QStringList horHeaderValues;
	for(int i = 0; i < NUMBER_OF_HARMONICS; i++) {
		horHeaderValues.push_back(QString::number(i));
	}
	m_table->setHorizontalHeaderLabels(horHeaderValues);
	for(int i = 0; i < HARMONICS_MIN_DEGREE; i++) {
		m_table->horizontalHeader()->hideSection(i);
	}
	StyleHelper::TableViewWidget(m_table->parentWidget(), "HarmonicsTable");
	for(int i = 0; i < MAX_CHNLS; i++) {
		for(int j = 0; j < NUMBER_OF_HARMONICS; j++) {
			QTableWidgetItem *tableItem = new QTableWidgetItem();
			tableItem->setFlags(tableItem->flags() ^ Qt::ItemIsEditable);
			m_table->setItem(i, j, tableItem);
		}
	}
}

void HarmonicsInstrument::initPlot()
{
	m_plot->yAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	m_plot->yAxis()->scaleDraw()->setFloatPrecision(2);
	m_plot->yAxis()->scaleDraw()->setUnitType("%");
	m_plot->yAxis()->setInterval(0, 100);

	m_plot->xAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	m_plot->xAxis()->scaleDraw()->setFloatPrecision(0);
	m_plot->xAxis()->scaleDraw()->setUnitType("");
	m_plot->xAxis()->setInterval(HARMONICS_MIN_DEGREE, HARMONICS_MAX_DEGREE);

	m_plot->setShowYAxisLabels(true);
	m_plot->setShowXAxisLabels(true);
	m_plot->replot();
}

void HarmonicsInstrument::setupPlotChannels()
{
	int chNumber = 0;
	bool first = true;
	for(const QString &ch : m_chnls) {
		QPen chPen = QPen(QColor(StyleHelper::getColor("CH" + QString::number(chNumber))), 1);
		PlotChannel *plotCh = new PlotChannel(m_chnls.key(ch), chPen, m_plot->xAxis(), m_plot->yAxis(), this);
		m_plot->addPlotChannel(plotCh);
		plotCh->setStyle(PlotChannel::PCS_STICKS);
		plotCh->setThickness(10);
		plotCh->curve()->setRawSamples(m_xTime.data(), m_yValues[ch].data(), m_xTime.size());
		m_plotChnls[ch] = plotCh;
		if(first) {
			plotCh->setEnabled(true);
			m_plot->selectChannel(plotCh);
			first = false;
		}
		chNumber++;
	}
}

QWidget *HarmonicsInstrument::createThdWidget()
{
	QWidget *thdWidget = new QWidget(this);
	thdWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	QGridLayout *layout = new QGridLayout(thdWidget);

	QLabel *title = new QLabel("THD:", this);
	layout->addWidget(title);
	int row = 1;
	int chnlIdx = 0;
	for(const QString &ch : m_chnls) {
		MeasurementLabel *ml = new MeasurementLabel(this);
		QString color = StyleHelper::getColor("CH" + QString::number(chnlIdx));
		ml->setColor(QColor(color));
		ml->setName(m_chnls.key(ch));
		ml->setPrecision(7);
		m_labels[ch] = ml;
		layout->addWidget(ml, row, chnlIdx);
		chnlIdx++;
	}
	return thdWidget;
}

QWidget *HarmonicsInstrument::createSettingsMenu(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setMargin(0);
	layout->setSpacing(10);

	MenuHeaderWidget *header = new MenuHeaderWidget("Settings", QPen(StyleHelper::getColor("ScopyBlue")), widget);
	QWidget *generalSection = createMenuGeneralSection(widget);
	QWidget *logSection = createMenuLogSection(widget);

	layout->addWidget(header);
	layout->addWidget(generalSection);
	layout->addWidget(logSection);
	layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	return widget;
}

QWidget *HarmonicsInstrument::createMenuGeneralSection(QWidget *parent)
{
	MenuSectionCollapseWidget *generalSection = new MenuSectionCollapseWidget(
		"GENERAL", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, parent);
	generalSection->contentLayout()->setSpacing(10);

	MenuCombo *harmonicType = new MenuCombo(tr("Harmonics Type"), generalSection);
	harmonicType->combo()->addItem("harmonics");
	harmonicType->combo()->addItem("inter_harmonics");
	m_harmonicsType = harmonicType->combo()->currentText();
	connect(harmonicType->combo(), &QComboBox::currentTextChanged, this,
		[=, this](QString h) { m_harmonicsType = h; });

	MenuCombo *activeChnlCb = new MenuCombo(tr("Active channel"), generalSection);
	for(const QString &ch : m_chnls) {
		activeChnlCb->combo()->addItem(m_chnls.key(ch));
	}
	m_table->selectRow(0);
	connect(activeChnlCb->combo(), &QComboBox::currentTextChanged, this,
		&HarmonicsInstrument::onActiveChnlChannged);
	connect(activeChnlCb->combo(), QOverload<int>::of(&QComboBox::activated), m_table, &QTableView::selectRow);
	connect(m_table->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
		[=, this](const QModelIndex &current) { activeChnlCb->combo()->setCurrentIndex(current.row()); });
	connect(m_table->selectionModel(), &QItemSelectionModel::selectionChanged, this,
		&HarmonicsInstrument::onSelectionChanged);

	generalSection->add(harmonicType);
	generalSection->add(activeChnlCb);

	return generalSection;
}

QWidget *HarmonicsInstrument::createMenuLogSection(QWidget *parent)
{
	MenuSectionCollapseWidget *logSection = new MenuSectionCollapseWidget(
		"LOG", MenuCollapseSection::MHCW_ONOFF, MenuCollapseSection::MHW_BASEWIDGET, parent);
	logSection->contentLayout()->setSpacing(10);
	logSection->setCollapsed(true);

	QWidget *browseWidget = new QWidget(logSection);
	browseWidget->setLayout(new QHBoxLayout(browseWidget));
	browseWidget->layout()->setMargin(0);

	MenuLineEdit *logFilePath = new MenuLineEdit(browseWidget);
	logFilePath->edit()->setPlaceholderText("Select log directory");
	QPushButton *browseBtn = new QPushButton("...", browseWidget);
	StyleHelper::BrowseButton(browseBtn);

	browseWidget->layout()->addWidget(logFilePath);
	browseWidget->layout()->addWidget(browseBtn);

	connect(this, &HarmonicsInstrument::enableTool, this, [this, logFilePath, logSection](bool en) {
		logSection->setDisabled(en);
		QString dirPath = logFilePath->edit()->text();
		QDir logDir = QDir(dirPath);
		if(dirPath.isEmpty())
			logSection->setCollapsed(true);
		if(en && logDir.exists() && !logSection->collapsed())
			Q_EMIT logData(PqmDataLogger::Harmonics, dirPath);
		else
			Q_EMIT logData(PqmDataLogger::None, "");
	});
	connect(browseBtn, &QPushButton::clicked, this, [this, logFilePath]() { browseFile(logFilePath->edit()); });

	logSection->add(browseWidget);

	return logSection;
}

void HarmonicsInstrument::updateTable()
{
	int i = 0;
	for(const QString &ch : m_chnls) {
		for(int j = 0; j < NUMBER_OF_HARMONICS; j++) {
			QTableWidgetItem *item = m_table->item(i, j);
			item->setText(QString::number(m_yValues[ch][j]));
		}
		i++;
	}
}

void HarmonicsInstrument::onActiveChnlChannged(QString chnlId)
{
	QString ctxCh = m_chnls[chnlId];
	if(m_plotChnls.contains(ctxCh)) {
		PlotChannel *ch = m_plotChnls[ctxCh];
		if(m_plot->selectedChannel()) {
			m_plot->selectedChannel()->setEnabled(false);
		}
		ch->setEnabled(true);
		m_plot->selectChannel(ch);
		m_plot->replot();
	}
}

void HarmonicsInstrument::stop() { m_runBtn->setChecked(false); }

void HarmonicsInstrument::toggleHarmonics(bool en)
{
	m_running = en;
	if(en) {
		ResourceManager::open("pqm" + m_uri, this);
	} else {
		ResourceManager::close("pqm" + m_uri);
	}
	Q_EMIT enableTool(en);
}
bool HarmonicsInstrument::selectedFromSameCol(QModelIndexList list)
{
	int rowsNumber = m_chnls.size();
	int selectedIndexNumber = list.size();
	if(selectedIndexNumber > rowsNumber) {
		return false;
	}
	int firstIndexColumn = list.first().column();
	int cellsWithSameColumn = std::count_if(list.begin(), list.end(), [&firstIndexColumn](const QModelIndex &item) {
		return item.column() == firstIndexColumn;
	});
	return cellsWithSameColumn == selectedIndexNumber;
}

void HarmonicsInstrument::onSelectionChanged()
{
	QModelIndexList selectedIndexList = m_table->selectionModel()->selectedIndexes();
	if(selectedIndexList.size() <= 1 || selectedFromSameCol(selectedIndexList)) {
		m_plot->xAxis()->setInterval(HARMONICS_MIN_DEGREE, HARMONICS_MAX_DEGREE);
		return;
	}
	int firstColumnSelected = selectedIndexList.front().column();
	int lastColumnSelected = selectedIndexList.back().column();
	int lowerIdx = std::min(firstColumnSelected, lastColumnSelected);
	int minDegree = (lowerIdx < HARMONICS_MIN_DEGREE) ? HARMONICS_MIN_DEGREE : lowerIdx;
	m_plot->xAxis()->setInterval(minDegree, std::max(firstColumnSelected, lastColumnSelected));
}

void HarmonicsInstrument::onAttrAvailable(QMap<QString, QMap<QString, QString>> attr)
{
	if(!m_running) {
		return;
	}
	QString h = m_harmonicsType;
	for(const QString &ch : m_chnls) {
		QStringList harmonics = attr[ch][h].split(" ");
		m_yValues[ch].clear();
		for(const QString &val : qAsConst(harmonics)) {
			m_yValues[ch].push_back(val.toDouble());
		}
		// thd labels update
		m_labels[ch]->setValue(attr[ch]["thd"].toDouble());
	}
	updateTable();
	m_plot->replot();
	if(m_singleBtn->isChecked()) {
		m_singleBtn->setChecked(false);
	}
}

void HarmonicsInstrument::browseFile(QLineEdit *lineEditPath)
{
	QString dirPath =
		QFileDialog::getExistingDirectory(this, "Select a directory", "directoryToOpen",
						  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	lineEditPath->setText(dirPath);
}

#include "moc_harmonicsinstrument.cpp"
