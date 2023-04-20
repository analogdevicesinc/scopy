#ifndef SWIOTCONFIGCHNLVIEW_HPP
#define SWIOTCONFIGCHNLVIEW_HPP
#define AD_INDEX  0
#define MAX_INDEX 1
#define AD_FUNC_ATTR_NAME  "function_cfg"
#define MAX_FUNC_ATTR_NAME "function"
#define MAX_IEC_ATTR_NAME  "IEC_type"

#include "qwidget.h"
#include "ui_swiotconfigchnlview.h"
#include <QObject>

namespace scopy::swiot {
class ConfigChannelView: public QWidget
{
	Q_OBJECT
public:
	explicit ConfigChannelView(int chnlIdx = 0, QWidget *parent = nullptr);
	~ConfigChannelView();

	void setChnlsAttr(QVector<QMap<QString, QStringList>> values);
	void setAvailableOptions(QComboBox* list, QString attrName,
				 QMap<QString, QStringList> chnlAttr);
	void connectSignalsToSlots();
	QVector<QMap<QString, QStringList>> getChnlsAttr();
public Q_SLOTS:
	void adIndexChanged(int idx);
	void maxIndexChanged(int idx);
	void adEnabledChanged(int idx);
	void maxEnabledChanged(int idx);
	void max2IndexChanged(int idx);
Q_SIGNALS:
	void attrValueChanged(QString attrName, int deviceIdx);

private:
	int m_chnlIdx;
	int m_adHighZIdx;
	int m_maxHighZIdx;
	Ui::ConfigChannelView* m_ui;
	QVector<QMap<QString, QStringList>> m_chnlsAttr;

	int getHighZIdx(QComboBox* list);
};
}


#endif // SWIOTCONFIGCHNLVIEW_HPP
