#include "configchannelview.h"

using namespace scopy::swiot;

ConfigChannelView::ConfigChannelView(int chnlIdx, QWidget *parent) :
	QWidget(parent),
	m_chnlIdx(chnlIdx),
	m_ui(new Ui::ConfigChannelView)
{
	m_ui->setupUi(this);
	m_ui->chnlLabel->setText(QString::fromStdString("Channel ") + QString::number(m_chnlIdx));
	connectSignalsToSlots();
}

ConfigChannelView::~ConfigChannelView()
{
	delete m_ui;
}

void ConfigChannelView::setChnlsAttr(QVector<QMap<QString, QStringList>> values)
{
	for (int i = 0; i < values.size(); i++) {
		m_chnlsAttr.push_back(values[i]);
	}
	if (m_chnlsAttr.size() == 2) {
		setAvailableOptions(m_ui->adOptions,QString(AD_FUNC_ATTR_NAME) + QString("_available"), m_chnlsAttr[AD_INDEX]);
		m_adHighZIdx = getHighZIdx(m_ui->adOptions);
		m_ui->adOptions->setCurrentIndex(m_adHighZIdx);

		setAvailableOptions(m_ui->maxOptions1, QString(MAX_FUNC_ATTR_NAME) + QString("_available"), m_chnlsAttr[MAX_INDEX]);
		m_maxHighZIdx = getHighZIdx(m_ui->maxOptions1);
		m_ui->maxOptions1->setCurrentIndex(m_maxHighZIdx);

		setAvailableOptions(m_ui->maxOptions2, QString(MAX_IEC_ATTR_NAME) + QString("_available"), m_chnlsAttr[MAX_INDEX]);
	}

}

void ConfigChannelView::setAvailableOptions(QComboBox* list, QString attrName,
					    QMap<QString, QStringList> chnlAttr)
{
	QStringList availableValues = chnlAttr[attrName];
	for (const auto& value : availableValues) {
		list->addItem(value);
	}
}

void ConfigChannelView::connectSignalsToSlots()
{
	connect(m_ui->adOptions, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &ConfigChannelView::adIndexChanged);
	connect(m_ui->adOptions, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &ConfigChannelView::adEnabledChanged);

	connect(m_ui->maxOptions1, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &ConfigChannelView::maxIndexChanged);
	connect(m_ui->maxOptions1, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &ConfigChannelView::maxEnabledChanged);

	connect(m_ui->maxOptions2, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &ConfigChannelView::max2IndexChanged);
}

void ConfigChannelView::adIndexChanged(int idx)
{
	QString attrName = AD_FUNC_ATTR_NAME;
	QString funcAvailable = QString(AD_FUNC_ATTR_NAME) + QString("_available");
	const auto& function = m_chnlsAttr[AD_INDEX][funcAvailable][idx];
	m_chnlsAttr[AD_INDEX][attrName].clear();
	m_chnlsAttr[AD_INDEX][attrName].push_back(function);
	if (idx != m_adHighZIdx) {
		m_ui->maxOptions1->setCurrentIndex(m_maxHighZIdx);
	}
	Q_EMIT attrValueChanged(attrName, AD_INDEX);
}

void ConfigChannelView::adEnabledChanged(int idx)
{
	QString attrName = "enabled";
	if (idx != m_adHighZIdx) {
		if (m_chnlsAttr[AD_INDEX][attrName].front().compare("0")==0) {
			m_chnlsAttr[AD_INDEX][attrName].clear();
			m_chnlsAttr[AD_INDEX][attrName].push_back("1");
			Q_EMIT attrValueChanged(attrName, AD_INDEX);
		}
	}
	else {
		if (m_chnlsAttr[AD_INDEX][attrName].front().compare("1")==0) {
			m_chnlsAttr[AD_INDEX][attrName].clear();
			m_chnlsAttr[AD_INDEX][attrName].push_back("0");
			Q_EMIT attrValueChanged(attrName, AD_INDEX);
		}
	}

}

void ConfigChannelView::maxIndexChanged(int idx)
{
	QString attrName = MAX_FUNC_ATTR_NAME;
	QString funcAvailable = QString(MAX_FUNC_ATTR_NAME) + QString("_available");
	const auto& function = m_chnlsAttr[MAX_INDEX][funcAvailable][idx];
	m_chnlsAttr[MAX_INDEX][attrName].clear();
	m_chnlsAttr[MAX_INDEX][attrName].push_back(function);
	if (function.compare("direction_in") == 0) {
		m_ui->maxOptions2->setVisible(true);
	}
	else {
		m_ui->maxOptions2->setVisible(false);
	}
	if (idx != m_maxHighZIdx) {
		m_ui->adOptions->setCurrentIndex(m_adHighZIdx);
	}
	Q_EMIT attrValueChanged(attrName, MAX_INDEX);
}

void ConfigChannelView::maxEnabledChanged(int idx)
{
	QString attrName = "enabled";
	if (idx != m_maxHighZIdx) {
		if (m_chnlsAttr[MAX_INDEX][attrName].front().compare("0")==0) {
			m_chnlsAttr[MAX_INDEX][attrName].clear();
			m_chnlsAttr[MAX_INDEX][attrName].push_back("1");
			Q_EMIT attrValueChanged(attrName, MAX_INDEX);
		}
	}
	else {
		if (m_chnlsAttr[MAX_INDEX][attrName].front().compare("1")==0) {
			m_chnlsAttr[MAX_INDEX][attrName].clear();
			m_chnlsAttr[MAX_INDEX][attrName].push_back("0");
			Q_EMIT attrValueChanged(attrName, MAX_INDEX);
		}
	}
}

void ConfigChannelView::max2IndexChanged(int idx)
{
	QString attrName = MAX_IEC_ATTR_NAME;
	QString funcAvailable = QString(MAX_IEC_ATTR_NAME) + QString("_available");
	const auto& function = m_chnlsAttr[MAX_INDEX][funcAvailable][idx];
	m_chnlsAttr[MAX_INDEX][attrName].clear();
	m_chnlsAttr[MAX_INDEX][attrName].push_back(function);
	Q_EMIT attrValueChanged(attrName, MAX_INDEX);
}

QVector<QMap<QString, QStringList>> ConfigChannelView::getChnlsAttr()
{
	return m_chnlsAttr;
}

int ConfigChannelView::getHighZIdx(QComboBox* list)
{
	int idx = 0;
	for (int i = 0; i < list->count(); i++) {
		if (list->itemText(i).compare("high_z") == 0) {
			idx = i;
		}
		if (list->itemText(i).compare("direction_high_z") == 0) {
			idx = i;
		}
	}
	return idx;
}
