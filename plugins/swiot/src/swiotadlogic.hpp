#ifndef SWIOTADLOGIC_HPP
#define SWIOTADLOGIC_HPP

#include <QObject>
#include <QMap>
#include <iio.h>
#include <errno.h>

#define MAX_BUFFER_SIZE 144
#define MIN_BUFFER_SIZE 20
#define MAX_KERNEL_BUFFER 64

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
	struct iio_buffer;
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
		int getEnabledChnls();
		int getPlotChnlsNo();

		void createIioBuffer(int sampleRate, double timestamp);
		void destroyIioBuffer();
		void enableIioChnls(bool changes);

		bool verifyEnableChanges(std::vector<bool> enabledChnls);
Q_SIGNALS:
		void bufferCreated(struct iio_buffer* iioBuff, int enabledChnlsNo, std::vector<std::pair<double, double>> offsetScaleValues);
		void bufferDestroyed();

private:
		int m_plotChnlsNo;
		int m_chnlsNumber;

		QMap<int, struct chnlInfo*> m_chnlsInfo;
		struct iio_device* m_iioDev;
		struct iio_buffer* m_iioBuff;

		void createChannels();
		std::vector<std::pair<double, double>> getOffsetScaleVector();
	};
}


#endif // SWIOTADLOGIC_HPP
