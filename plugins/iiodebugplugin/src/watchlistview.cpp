#include "watchlistview.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLoggingCategory>
#include <QHeaderView>
#include <QScrollBar>

#define NAME_POS 0
#define VALUE_POS 1
#define PATH_POS 2
#define CLOSE_BTN_POS 3
#define DIVISION_REGION 4

Q_LOGGING_CATEGORY(CAT_WATCHLISTVIEW, "WatchListView")
using namespace scopy::iiodebugplugin;

WatchListView::WatchListView(QWidget *parent)
	: QTableWidget(parent)
	, m_apiObject(new WatchListView_API(this))
{
	setupUi();
	connectSignalsAndSlots();
	m_apiObject->setObjectName("WatchListView");
}

void WatchListView::setupUi()
{
	QStringList headers = {"Name", "Value", "Path", ""};
	setColumnCount(headers.size());
	setHorizontalHeaderLabels(headers);
	verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	verticalHeader()->setDefaultSectionSize(12);
	verticalHeader()->hide();
	horizontalScrollBar()->setDisabled(true);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	// the header setup has to happen after setting the QTableWidget
	auto *header = new QHeaderView(Qt::Horizontal, this);
	setHorizontalHeader(header);
	header->setCascadingSectionResizes(true);
	header->setSectionResizeMode(NAME_POS, QHeaderView::Interactive);
	header->setSectionResizeMode(VALUE_POS, QHeaderView::Interactive);
	header->setSectionResizeMode(PATH_POS, QHeaderView::Interactive);
	header->setSectionResizeMode(CLOSE_BTN_POS, QHeaderView::Interactive);

	// TODO: move this to stylehelper ( eventually )
	QString style = QString(R"css(
				QHeaderView::section {
					font: 11pt;
					border: none;
					background-color:&&ScopyBackground&&;
					font-family: Open Sans;
				}
				QTableWidget::item {
					border-left: 1px solid &&UIElementHighlight&&;
					font-family: Open Sans;
				}
				QTableWidget::item::selected {
					background-color: &&ScopyBlue&&;
					font-family: Open Sans;
				}
				QHeaderView::section {
					border-left: 1px solid &&UIElementHighlight&&;
					font-family: Open Sans;
				}
				)css");
	style.replace("&&ScopyBackground&&", StyleHelper::getColor("ScopyBackground"));
	style.replace("&&UIElementHighlight&&", StyleHelper::getColor("UIElementHighlight"));
	style.replace("&&ScopyBlue&&", StyleHelper::getColor("ScopyBlue"));
	setStyleSheet(style);
	verticalHeader()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
}

void WatchListView::connectSignalsAndSlots()
{
	QObject::connect(this, &QTableWidget::cellClicked, this, [this](int row, int column) {
		selectRow(row);
		QString path = item(row, PATH_POS)->text();
		qInfo(CAT_WATCHLISTVIEW()) << "Selected" << path;

		if(!m_entryObjects.contains(path)) {
			qWarning(CAT_WATCHLISTVIEW) << "No object with name" << path
						    << "was found in the entry objects map, skipping cell selection.";
			return;
		}
		WatchListEntry *watchListEntry = m_entryObjects[path];
		IIOStandardItem *item = watchListEntry->item();

		Q_EMIT selectedItem(item);
	});

	QObject::connect(horizontalHeader(), &QHeaderView::sectionResized, this,
			 [this](int logicalIndex, int oldSize, int newSize) {
				 int originalSize = width() / DIVISION_REGION;
				 int offset = newSize - originalSize;
				 m_offsets[logicalIndex] = offset;
			 });
}

void WatchListView::saveSettings(QSettings &s) { m_apiObject->save(s); }

void WatchListView::loadSettings(QSettings &s) { m_apiObject->load(s); }

void WatchListView::addToWatchlist(IIOStandardItem *item)
{
	auto entry = new WatchListEntry(item, this);
	m_entryObjects.insert(entry->path()->text(), entry);
	int row = rowCount();
	insertRow(row);
	setItem(row, NAME_POS, entry->name());
	setItem(row, VALUE_POS, entry->value());
	setItem(row, PATH_POS, entry->path());

	auto deleteButton = new QPushButton("X");
	setCellWidget(row, CLOSE_BTN_POS, deleteButton);
	deleteButton->setContentsMargins(0, 0, 0, 0);
	QObject::connect(deleteButton, &QPushButton::clicked, this, [this, entry]() {
		int row = -1;
		for(int i = 0; i < rowCount(); ++i) {
			if(this->item(i, PATH_POS)->text() == entry->path()->text()) {
				row = i;
				break;
			}
		}

		if(row != -1) {
			removeRow(row);
			entry->deleteLater();
		}
	});
}

void WatchListView::removeFromWatchlist(IIOStandardItem *item)
{
	int row = -1;
	for(int i = 0; i < rowCount(); ++i) {
		if(this->item(i, PATH_POS)->text() == item->path()) {
			row = i;
			break;
		}
	}

	if(row != -1) {
		removeRow(row);
		WatchListEntry *entry = m_entryObjects.value(item->path(), nullptr);
		if(entry) {
			entry->deleteLater();
		} else {
			qWarning(CAT_WATCHLISTVIEW)
				<< "Entry" << item->path() << "not present in entries, cannot remove from watchlist.";
		}
	}
}

void WatchListView::currentTreeSelectionChanged(IIOStandardItem *item)
{
	QString path = item->path();
	if(m_entryObjects.contains(path)) {
		// highlight path
		for(int i = 0; i < rowCount(); ++i) {
			if(this->item(i, PATH_POS)->text() == path) {
				selectRow(i);
				return;
			}
		}
	} else {
		// clear any highlights to avoid confusion
		clearSelection();
	}
}

void WatchListView::resizeEvent(QResizeEvent *event)
{
	int w = width();
	int sectionWidth = w / DIVISION_REGION;
	setColumnWidth(NAME_POS, sectionWidth + m_offsets[NAME_POS]);
	setColumnWidth(VALUE_POS, sectionWidth + m_offsets[VALUE_POS]);
	setColumnWidth(PATH_POS, sectionWidth + m_offsets[PATH_POS]);
	setColumnWidth(CLOSE_BTN_POS, sectionWidth + m_offsets[CLOSE_BTN_POS]);

	QTableWidget::resizeEvent(event);
}

QList<int> WatchListView_API::tableHeader() const
{
	QHeaderView *header = p->horizontalHeader();
	int columns = p->columnCount();
	QList<int> sizes;
	for(int i = 0; i < columns; ++i) {
		int size = header->sectionSize(i);
		sizes.append(size);
	}
	return sizes;
}

void WatchListView_API::setTableHeader(const QList<int> &newTableHeader)
{
	QHeaderView *header = p->horizontalHeader();
	int columns = p->columnCount();
	for(int i = 0; i < qMin(newTableHeader.size(), columns); ++i) {
		header->resizeSection(i, newTableHeader.at(i));
	}
}

QList<int> WatchListView_API::offsets() const { return p->m_offsets; }

void WatchListView_API::setOffsets(const QList<int> &newOffsets)
{
	p->m_offsets = newOffsets;
	int size = newOffsets.size();
	int sum = 0;

	for(int i = 0; i < size; ++i) {
		sum += newOffsets[i];
	}

	// abs(sum) < 1 is an error that can be accepted when computing offsets
	if(sum < -1 || sum > 1) {
		// the table is not fully visible, this is a backup measure to allow good display
		qWarning(CAT_WATCHLISTVIEW)
			<< "The table contents are not fully visible, the offset that is not seen is" << sum
			<< "and it will be subtracted from the widest column from the last 2.";

		if(newOffsets[size - 1] > newOffsets[size - 2]) {
			p->m_offsets[size - 1] -= sum;
		} else {
			p->m_offsets[size - 2] -= sum;
		}
	}
}
