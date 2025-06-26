#include <QThread>
#include <cmath>
#include <include/data-sink/customSourceBlocks.h>
#include <qfile.h>
using namespace scopy::datasink;

FileSourceBlock::FileSourceBlock(QString filename, QString name)
	: SourceBlock(name)
	, m_filename(filename)
{}

FileSourceBlock::~FileSourceBlock() {}

BlockData *FileSourceBlock::createData()
{
	QFile file = QFile(m_filename);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qWarning() << "Error: Could not open file";
		return {};
	}

	QTextStream m_stream(&file);

	BlockData *map = new BlockData();
	int numRows = 0;

	while(!m_stream.atEnd()) {
		QString line = m_stream.readLine();
		QStringList values = line.split(',');

		for(int col = 0; col < values.size(); ++col) {
			if(!m_channels.value(col) && !map->contains(col))
				continue;

			if(m_size <= numRows) {
				break;
			}

			// REMOVE THIS
			// + rand() % 20
			float num = values[col].toFloat();

			if(numRows == 0) {
				ChannelDataVector *data = new ChannelDataVector(m_size);
				map->insert(col, data);
				data->data.push_back(num);
			} else {
				map->value(col)->data.push_back(num);
			}
		}
		numRows++;
	}

	return map;
}

TestSourceBlock::TestSourceBlock(QString name)
	: SourceBlock(name)
{}

TestSourceBlock::~TestSourceBlock() {}

BlockData *TestSourceBlock::createData()
{
	BlockData *map = new BlockData();

	for(int ch = 0; ch < m_channels.size(); ++ch) {
		if(m_channels.value(ch)) {
			ChannelDataVector *data = new ChannelDataVector(m_size);

			for(int i=0; i<m_size; i++) {
				data->data.push_back(i * pow(10, ch));
			}
			map->insert(ch, data);
		}
	}

	return map;
}

IIOSourceBlock::IIOSourceBlock(iio_device *dev, QString name)
	: SourceBlock(name)
{
	m_dev = dev;
}

IIOSourceBlock::~IIOSourceBlock() {}

void IIOSourceBlock::enChannel(bool en, uint id)
{
	SourceBlock::enChannel(en, id);

	if(en)
		iio_channel_enable(iio_device_get_channel(m_dev, id));
	else
		iio_channel_disable(iio_device_get_channel(m_dev, id));
}

BlockData *IIOSourceBlock::createData()
{
	// QThread::msleep(100);

	iio_channel *rx0_i = iio_device_find_channel(m_dev, m_channels.value(0) ? "voltage0" : "voltage1", false);
	iio_buffer *buf = iio_device_create_buffer(m_dev, m_size, false);
	BlockData *map = new BlockData();
	ssize_t nbytes_rx;
	char *p_dat, *p_end;
	ptrdiff_t p_inc;

	if(!buf) {
		return {};
	}

	// Refill RX buffer
	nbytes_rx = iio_buffer_refill(buf);
	if(nbytes_rx < 0) {
		printf("Error refilling buf %d\n", (int)nbytes_rx);
		return {};
	}

	// READ: Get pointers to RX buf and read IQ from RX buf port 0
	p_inc = iio_buffer_step(buf);
	p_end = (char *)iio_buffer_end(buf);

	int i;
	for(auto it = m_channels.begin(); it != m_channels.end(); ++it) {
		if(it.value()) {
			ChannelDataVector *data = new ChannelDataVector(m_size);

			i = 0;
			for(p_dat = (char *)iio_buffer_first(buf, rx0_i); p_dat < p_end; p_dat += p_inc) {
				// // Example: swap I and Q
				// const int16_t i = ((int16_t*)p_dat)[0]; // Real (I)
				// const int16_t q = ((int16_t*)p_dat)[1]; // Imag (Q)
				// ((int16_t*)p_dat)[0] = q;
				// ((int16_t*)p_dat)[1] = i;
				data->data.push_back((float)((int16_t *)p_dat)[it.key()]);
				i++;
			}

			map->insert(it.key(), data);
		}
	}

	iio_buffer_destroy(buf);
	count++;
	// std::cout << "iio source:" << count <<  std::endl;

	return map;
}
