#ifndef SWIOTADLOGIC_HPP
#define SWIOTADLOGIC_HPP

#include <QObject>
#include <QMap>
#include <iio.h>
#include <errno.h>

#define MAX_BUFFER_SIZE 160
#define MIN_BUFFER_SIZE 20
#define MAX_KERNEL_BUFFER 64
#define SAMPLING_FREQ_ATTR_NAME "sampling_frequency"

struct chnlInfo{
	QString chnlId;
	bool isOutput;
	bool isEnabled;
	std::pair<double, double> offsetScalePair;
	struct iio_channel* iioChnl;
};

extern "C" {
	struct iio_context;
	struct iio_device;
	struct iio_channel;
}
Q_DECLARE_OPAQUE_POINTER(struct iio_buffer*)
namespace adiscope {
	class SwiotAdLogic: public QObject
	{
		Q_OBJECT
public:
		explicit SwiotAdLogic(struct iio_device* iioDev = nullptr);
		~SwiotAdLogic();

		struct iio_channel* getIioChnl(int chnlIdx, bool outputPriotity);
		bool verifyEnableChanges(std::vector<bool> enabledChnls);
		int getPlotChnlsNo();

		QStringList readChnlsFrequencyAttr(QString attrName);
public Q_SLOTS:
		void onSamplingFreqChanged(int idx);
Q_SIGNALS:
		void chnlsChanged(QMap<int, struct chnlInfo*> chnlsInfo);
private:
		void createChannels();
private:
		int m_plotChnlsNo;
		int m_chnlsNumber;
		QStringList m_samplingFreqAvailable;

		QMap<int, struct chnlInfo*> m_chnlsInfo;
		struct iio_device* m_iioDev;


	};
}


#endif // SWIOTADLOGIC_HPP
