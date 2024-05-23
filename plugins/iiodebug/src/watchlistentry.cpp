#include "watchlistentry.h"
#include <iio-widgets/datastrategy/datastrategyinterface.h>
#include <iio-widgets/guistrategy/comboguistrategy.h>
#include <QHBoxLayout>
#include <QComboBox>

using namespace scopy::iiodebugplugin;

WatchListEntry::WatchListEntry(IIOStandardItem *item, QObject *parent)
	: QObject(parent)
	, m_item(item)
	, m_name(new QTableWidgetItem())
	, m_type(new QTableWidgetItem())
	, m_path(new QTableWidgetItem())
	, m_valueUi(nullptr)
	, m_combo(nullptr)
	, m_lineedit(nullptr)
{
	setupUi();
	m_name->setText(item->text());
	m_type->setText(item->typeString());
	m_path->setText(item->path());

	IIOStandardItem::Type type = item->type();
	if(type == IIOStandardItem::ContextAttribute || type == IIOStandardItem::DeviceAttribute ||
	   type == IIOStandardItem::ChannelAttribute) {
		// it is a leaf node so it has only 1 widget
		QList<IIOWidget *> widgets = item->getIIOWidgets();
		IIOWidget *widget = widgets[0];
		setupWidget(widget);
	} else {
		QLabel *label = new QLabel("N/A");
		label->setContentsMargins(3, 0, 3, 0);
		m_valueUi = label;
	}
}

WatchListEntry::~WatchListEntry()
{
	// the objects created here do not need to be deleted
	// if they are added to the QTableWidget as it will become the parent
}

QTableWidgetItem *WatchListEntry::name() { return m_name; }

void WatchListEntry::setName(QString name) { m_name->setText(name); }

QTableWidgetItem *WatchListEntry::path() { return m_path; }

void WatchListEntry::setPath(QString path) { m_path->setText(path); }

IIOStandardItem *WatchListEntry::item() { return m_item; }

void WatchListEntry::setNewData(QString data, QString optionalData)
{
	if(m_combo) {
		QSignalBlocker sg(m_combo);
		m_combo->clear();
		QStringList list = optionalData.split(" ", Qt::SkipEmptyParts);
		m_combo->addItems(list);
		m_combo->setCurrentText(data);
	} else {
		QSignalBlocker sg(m_lineedit);
		m_lineedit->setText(data);
	}
}

void WatchListEntry::setupUi()
{
	// remove the editable flag from every QTableWidgetItem
	m_name->setFlags(m_name->flags() ^ Qt::ItemIsEditable);
	m_type->setFlags(m_type->flags() ^ Qt::ItemIsEditable);
	m_path->setFlags(m_path->flags() ^ Qt::ItemIsEditable);
}

void WatchListEntry::setupWidget(IIOWidget *widget)
{
	// if you can cast the uiStrategy to combo, this value ui will be a combo, otherwise it will be a lineedit
	GuiStrategyInterface *ui = dynamic_cast<ComboAttrUi *>(widget->getUiStrategy());
	if(ui) {
		// https://forum.qt.io/topic/139728/can-t-set-qcombobox-qslider-margins
		// QFrame because QWidget does not have the paintEvent implemented
		QFrame *wrapper = new QFrame();
		wrapper->setLayout(new QVBoxLayout(wrapper));
		wrapper->layout()->setContentsMargins(3, 0, 3, 0);

		m_combo = new QComboBox();
		// TODO: maybe move these to the stylehelper as well?
		m_combo->setStyleSheet(R"css(
		QComboBox {
			font-family: Open Sans;
			border: none;
			background-color: transparent;
			font-size: 13px;
		})css");
		wrapper->layout()->addWidget(m_combo);

		QString options = widget->getDataStrategy()->optionalData();
		QStringList list = options.split(" ", Qt::SkipEmptyParts);
		m_combo->addItems(widget->getDataStrategy()->optionalData().split(" ", Qt::SkipEmptyParts));
		m_combo->setCurrentText(widget->getDataStrategy()->data());
		QObject::connect(m_combo, &QComboBox::currentTextChanged, this,
				 [this, widget, options](QString text) { widget->getDataStrategy()->save(text); });
		m_valueUi = wrapper;
	} else {
		m_lineedit = new QLineEdit();
		m_lineedit->setStyleSheet(R"css(
		QLineEdit {
			font-family: Open Sans;
			border: none;
			background-color: transparent;
			font-size: 13px;
		})css");
		m_lineedit->setText(widget->getDataStrategy()->data());
		QObject::connect(m_lineedit, &QLineEdit::editingFinished, this, [this, widget]() {
			QString text = m_lineedit->text();
			widget->getDataStrategy()->save(text);
		});
		m_valueUi = m_lineedit;
	}

	QObject::connect(dynamic_cast<QWidget *>(widget->getDataStrategy()), SIGNAL(sendData(QString, QString)), this,
			 SLOT(setNewData(QString, QString)));
}

QWidget *WatchListEntry::valueUi() const { return m_valueUi; }

void WatchListEntry::setValueUi(QWidget *newValueUi) { m_valueUi = newValueUi; }

QTableWidgetItem *WatchListEntry::type() const { return m_type; }

void WatchListEntry::setType(QString type) { m_type->setText(type); }
