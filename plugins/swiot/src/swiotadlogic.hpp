#ifndef SWIOTADLOGIC_HPP
#define SWIOTADLOGIC_HPP

#include <QObject>
#include <QMap>
#include <iio.h>

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
		void createIioBuffer(int bufferSize);
		void enableIioChnls(bool changes);
		void destroyIioBuffer();
		bool verifyEnableChanges(std::vector<bool> enabledChnls);
		int getEnabledChnls();
		int getPlotChnlsNo();

Q_SIGNALS:
		void bufferCreated(struct iio_buffer* iioBuff, int enabledChnlsNo, double scale, double offset);
		void bufferDestroyed();

private:
		int m_plotChnlsNo;
		double m_offset;
		double m_scale;

		struct iio_device* m_iioDev;
		struct iio_buffer* m_iioBuff;

		QMap<QString, struct iio_channel*> m_channels;
		std::vector<bool> m_enabledChannels;

		void createChannels();
		std::vector<struct iio_channel*> getChnlsByIndex(int chnlIdx);

	};
}


#endif // SWIOTADLOGIC_HPP
