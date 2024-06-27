#ifndef TXNODE_H
#define TXNODE_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QColor>

#include <iio.h>

namespace scopy {
namespace dac {
class TxNode : public QObject
{
	Q_OBJECT
public:
	TxNode(QString uuid, struct iio_channel *chn = nullptr, QObject *parent = nullptr);
	virtual ~TxNode();

	TxNode *addChildNode(QString uuid, struct iio_channel *chn = nullptr);

	QMap<QString, TxNode *> getTones() const;
	QString getUuid() const;
	struct iio_channel *getChannel();
	unsigned int getFormatShift() const;
	unsigned int getFormatBits() const;

	bool enableDds(bool enable);

	const QColor &getColor() const;
	void setColor(const QColor &newColor);

private:
	QString m_txUuid;
	QMap<QString, TxNode *> m_childNodes = {};
	struct iio_channel *m_channel;
	unsigned int m_fmtShift;
	unsigned int m_fmtBits;
	QColor m_color;
};
} // namespace dac
} // namespace scopy

#endif // TXNODE_H
