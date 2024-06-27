#ifndef TXNODE_H
#define TXNODE_H

#include <QObject>
#include <QMap>
#include <QString>

#include <iio.h>

namespace scopy {
class TxNode : public QObject
{
	Q_OBJECT
public:
	TxNode(QString uuid, struct iio_channel *chn = nullptr, QObject *parent = nullptr);
	virtual ~TxNode();

	TxNode* addChildNode(QString uuid, struct iio_channel *chn = nullptr);

	QMap<QString, TxNode*> getTones() const;
	QString getUuid() const;
	struct iio_channel *getChannel();

private:
	QString m_txUuid;
	QMap<QString, TxNode*> m_childNodes = {};
	struct iio_channel *m_channel;
//	TxNode *m_parentNode;
//	QList<DacTone*> m_tones;

};
} // namespace scopy

#endif // TXNODE_H
