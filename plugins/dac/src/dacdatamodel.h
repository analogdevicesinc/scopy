#ifndef DACDATAMODEL_H
#define DACDATAMODEL_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>

#include <iio.h>

namespace scopy {
class TxNode;
class DacDataModel : public QObject
{
	Q_OBJECT
public:
	DacDataModel(struct iio_device *dev, QObject *parent = nullptr);
	virtual ~DacDataModel();

	QColor getColor() const;
	void setColor(QColor newColor);

	QString getName() const;
	bool isBufferCapable() const;
	bool isDds() const;
	QMap<QString, TxNode*> getDdsTxs() const;

	void setCyclic(bool cyclic);
//	void enableChannel()

private:
	struct iio_device *m_dev;
	QString m_name;
	QList<struct iio_channel*> m_channels;

	bool m_isBufferCapable;
	bool m_isDds;
	bool m_cyclicBuffer;

	const QString toneId = "altvoltage";
	const int MAX_NB_TONES = 4;
	const QString Q_CHANNEL = "Q";
	const QString I_CHANNEL = "I";
	// should have a list of txs, each tx has 2 tone channels

	QMap<QString, TxNode*> m_ddsTxs;
	QList<TxNode*> m_bufferTxs;

	bool initBufferDac();
	bool initDdsDac();
	void deinitBufferDac();
	void deinitDdsDac();
	QString generateToneName(QString chnId);
	QStringList generateTxNodesForChannel(QString name);
};
} // namespace scopy

#endif //DACDATAMODEL_H
