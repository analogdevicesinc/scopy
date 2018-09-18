#include "pg_buffer_manager.hpp"
#include "pattern_generator.hpp"

namespace adiscope {

PatternGeneratorBufferManager::PatternGeneratorBufferManager(
        PatternGeneratorChannelManager *chman) : chm(chman)
{
	autoSet = true;
	bufferSize = 1;
	buffer = new short[bufferSize];
	sampleRate = 1;
	start_sample = 0;
	last_sample = 1;
	buffer_created = 0;
}

PatternGeneratorBufferManager::~PatternGeneratorBufferManager()
{
	delete[] buffer;
}

void PatternGeneratorBufferManager::update(PatternGeneratorChannelGroup *chg)
{
	bool sampleRateChanged = false;

	chm->preGenerate();
	uint32_t suggestedSampleRate = (autoSet) ? chm->computeSuggestedSampleRate() :
	                               sampleRate;
	uint32_t adjustedSampleRate = adjustSampleRate(suggestedSampleRate);

	if (sampleRate != adjustedSampleRate) {
		sampleRate = adjustedSampleRate;
		sampleRateChanged = true;
	}

	bool bufferSizeChanged = false;
	uint32_t suggestedBufferSize = (autoSet) ? chm->computeSuggestedBufferSize(
	                                       sampleRate) : bufferSize;
	uint32_t adjustedBufferSize = adjustBufferSize(suggestedBufferSize);

	if (bufferSize != adjustedBufferSize) {
		bufferSize = adjustedBufferSize;
		bufferSizeChanged = true;
	}

	if (sampleRateChanged) {
		// recreate sigrok buffer
	}

	if (bufferSizeChanged) {
		// recreate local buffer
		delete[] buffer;
		buffer = new short[bufferSize];
	}

	memset(buffer, 0x0000, (bufferSize)*sizeof(short));
	chm->generatePatterns(buffer, sampleRate, bufferSize);

	if (sampleRateChanged || bufferSizeChanged) {
		// regenerate all
	} else {
		// only generate current ?
	}
}

void PatternGeneratorBufferManager::enableAutoSet(bool val)
{
	autoSet = val;
}

uint32_t PatternGeneratorBufferManager::adjustSampleRate(
        uint32_t suggestedSampleRate)
{
	if(suggestedSampleRate == 0)
		return PGMaxSampleRate;
	if (suggestedSampleRate>PGMaxSampleRate) {
		suggestedSampleRate = PGMaxSampleRate;
	} else {
		suggestedSampleRate = PGMaxSampleRate / (PGMaxSampleRate/suggestedSampleRate);
	}

	return suggestedSampleRate;
}

uint32_t PatternGeneratorBufferManager::adjustBufferSize(
        uint32_t suggestedBufferSize)
{
	if (suggestedBufferSize>1048576) {
		suggestedBufferSize = 1048576;
	}

	return suggestedBufferSize;
}

uint32_t PatternGeneratorBufferManager::getSampleRate()
{
	return sampleRate;
}

uint32_t PatternGeneratorBufferManager::getBufferSize()
{
	return bufferSize;
}

void PatternGeneratorBufferManager::setSampleRate(uint32_t val)
{
	sampleRate = val;
}

void PatternGeneratorBufferManager::setBufferSize(uint32_t val)
{
	bufferSize = val;
}

PatternGeneratorBufferManagerUi::PatternGeneratorBufferManagerUi(
        QWidget *parent, PatternGeneratorBufferManager *bufmanager,
        QWidget *settingsWidget, PatternGenerator *pg) : QWidget(parent),
	settingsWidget(settingsWidget), bufman(bufmanager) , pg(pg)
{
	//sigrok and sigrokdecode initialisation
	context = sigrok::Context::create();
	pv::DeviceManager device_manager(context);
	pv::MainWindow *w = new pv::MainWindow(device_manager, nullptr,
	                                       "pattern_generator", "", parent);
	binary_format = w->get_format_from_string("binary");
	/* setup PV plot view */
	main_win = w;

	parent->layout()->addWidget(static_cast<QWidget * >(main_win));
	/* setup toolbar */
	main_win->main_bar_->setVisible(false);
	main_win->view_->ruler_->set_offset(0);
	main_win->view_->ruler_->update();
	createBinaryBuffer();
}

void PatternGeneratorBufferManagerUi::updateUi()
{
	bufman->update();
	reloadPVDevice();

	auto scale = (1/(double)bufman->getSampleRate()) * bufman->getBufferSize() /
	             (double)main_win->view_->divisionCount();
	main_win->view_->set_scale_offset(scale,pv::util::Timestamp(0));
	Q_EMIT uiUpdated();

}

PatternGeneratorBufferManagerUi::~PatternGeneratorBufferManagerUi()
{
}

void PatternGeneratorBufferManagerUi::createBinaryBuffer()
{
	options["numchannels"] = Glib::Variant<gint32>(g_variant_new_int32(16),
	                         true);//(Glib::VariantBase)(gint32(16));
	options["samplerate"] = Glib::Variant<guint64>(g_variant_new_uint64(
	                                bufman->getSampleRate()),true);//(Glib::VariantBase)(gint64(1000000));
	pattern_generator_ptr = std::make_shared<pv::devices::BinaryBuffer>(context,
	                        bufman->buffer,&bufman->bufferSize,binary_format,options);
	main_win->select_device(pattern_generator_ptr);
}

pv::MainWindow *PatternGeneratorBufferManagerUi::getPVWindow()
{
	return main_win;
}

void PatternGeneratorBufferManagerUi::reloadPVDevice()
{
	pattern_generator_ptr->close();
	options["samplerate"] = Glib::Variant<guint64>(g_variant_new_uint64(
	                                bufman->getSampleRate()),true);//(Glib::VariantBase)(gint64(1000000));
	pattern_generator_ptr->options_ = options;
	pattern_generator_ptr->data_ = bufman->buffer;
	pattern_generator_ptr->open();
	main_win->run_stop();
}

}
