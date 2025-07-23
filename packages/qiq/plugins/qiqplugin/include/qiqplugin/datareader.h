#ifndef DATAREADER_H
#define DATAREADER_H

#include <QObject>
#include <QFile>
#include <QStringList>

namespace scopy::qiqplugin {
class DataReader : public QObject
{
	Q_OBJECT

public:
	explicit DataReader(QObject *parent = nullptr);
	~DataReader();

	bool openFile(const QString &path);
	void unmap();

	void setChannelCount(int count);
	int channelCount() const;

	void setChannelFormat(const QStringList &format);
	QStringList channelFormat() const;

	int getBytesPerSample() const;
	int getBytesPerChannel(int channelIndex) const;

	void readData(int64_t startSample, int64_t sampleCount);

Q_SIGNALS:
	void dataReady(QMap<int, QVector<double>> &processedData);

private:
	int getFormatSize(const QString &format) const;
	double convertToDouble(const QByteArray &data, const QString &format) const;
	bool remapFile();
	bool checkForRemapping();
	void createFile(const QString &path);

	QFile m_file;
	uchar *m_data;
	int64_t m_dataSize;
	int m_channelCount;
	QStringList m_channelFormat;
	QString m_filePath;
};

} // namespace scopy::qiqplugin

#endif // DATAREADER_H
