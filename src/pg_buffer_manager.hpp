#ifndef PG_BUFFER_MANAGER_H
#define PG_BUFFER_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <vector>
#include <string.h>

#include <iio.h>
#include "pg_channel_manager.hpp"

#include "pulseview/pv/mainwindow.hpp"
#include "pulseview/pv/devices/binarybuffer.hpp"
#include "pulseview/pv/devicemanager.hpp"
#include "pulseview/pv/toolbars/mainbar.hpp"
#include "pulseview/pv/view/view.hpp"
#include "pulseview/pv/util.hpp"


namespace pv {
class MainWindow;
class DeviceManager;
class View;
namespace toolbars {
class MainBar;
}
}

namespace sigrok {
class Context;
class ConfigKey;
}

namespace adiscope {

class PatternGeneratorBufferManager
{

	bool buffer_created;
	uint32_t start_sample;
	uint32_t last_sample;
	uint32_t number_of_samples;

	uint32_t sampleRate;
	PatternGeneratorChannelManager *chm;

public:
	PatternGeneratorBufferManager(PatternGeneratorChannelManager *chman);
	~PatternGeneratorBufferManager();
	void update(PatternGeneratorChannelGroup *chg = nullptr);
	uint32_t adjustSampleRate(uint32_t suggestedSampleRate);
	uint32_t adjustBufferSize(uint32_t suggestedBufferSize);
	uint32_t getSampleRate();
	uint32_t getBufferSize();

	uint32_t bufferSize;
	short *buffer;


};

class PatternGeneratorBufferManagerUi : public QWidget
{
	Q_OBJECT
	QWidget *settingsWidget;
	PatternGeneratorBufferManager *bufman;
	std::shared_ptr<sigrok::Context> context;
	std::shared_ptr<pv::devices::BinaryBuffer> pattern_generator_ptr;
	std::shared_ptr<sigrok::InputFormat> binary_format;
	std::map<std::string, Glib::VariantBase> options;
	pv::MainWindow *main_win;

public:
	PatternGeneratorBufferManagerUi(QWidget *parent,
	                                PatternGeneratorBufferManager *bufmananger, QWidget *settingsWidget,
	                                PatternGenerator *pg);

	~PatternGeneratorBufferManagerUi();
	void setSampleRate();
	void createBinaryBuffer();
	void dataChanged();
	pv::MainWindow *getPVWindow();

public Q_SLOTS:
	void updateUi();
};


}

#endif // PG_BUFFER_MANAGER_H
