#include "pg_buffer_manager.hpp"

namespace adiscope {

PatternGeneratorBufferManager::PatternGeneratorBufferManager(
        PatternGeneratorChannelManager *chman) : chm(chman)
{
	bufferSize = 1;
	buffer = new short[bufferSize];
	sampleRate = 1;
}

PatternGeneratorBufferManager::~PatternGeneratorBufferManager()
{
	delete buffer;
}

void PatternGeneratorBufferManager::update(PatternGeneratorChannelGroup *chg)
{
	bool sampleRateChanged = false;

	chm->preGenerate();
	uint32_t suggestedSampleRate = chm->computeSuggestedSampleRate();
	uint32_t adjustedSampleRate = adjustSampleRate(suggestedSampleRate);

	if (sampleRate != adjustedSampleRate) {
		sampleRate = adjustedSampleRate;
		sampleRateChanged = true;
	}

	bool bufferSizeChanged = false;
	uint32_t suggestedBufferSize = chm->computeSuggestedBufferSize(sampleRate);
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
		delete buffer;
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

uint32_t PatternGeneratorBufferManager::adjustSampleRate(
        uint32_t suggestedSampleRate)
{
	if (suggestedSampleRate>80000000) {
		suggestedSampleRate = 80000000;
	}

	return suggestedSampleRate;
}

uint32_t PatternGeneratorBufferManager::adjustBufferSize(
        uint32_t suggestedBufferSize)
{
	if (suggestedBufferSize>1000000) {
		suggestedBufferSize = 1000000;
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

PatternGeneratorBufferManagerUi::PatternGeneratorBufferManagerUi(
        QWidget *parent, PatternGeneratorBufferManager *bufmanager,
        QWidget *settingsWidget, PatternGenerator *pg) : QWidget(parent),
	settingsWidget(settingsWidget), bufman(bufmanager)
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
	pv::toolbars::MainBar *main_bar = main_win->main_bar_;
	QPushButton *btnDecoder = new QPushButton();
	btnDecoder->setIcon(QIcon::fromTheme("add-decoder",
	                                     QIcon(":/icons/add-decoder.svg")));
	btnDecoder->setMenu(main_win->menu_decoder_add());
	settingsWidget->layout()->addWidget(btnDecoder);
	settingsWidget->layout()->addWidget(static_cast<QWidget *>(main_bar));

	createBinaryBuffer();
	dataChanged();
}

void PatternGeneratorBufferManagerUi::updateUi()
{
	bufman->update();
	setSampleRate();
	dataChanged();
	//main_win->action_view_zoom_fit()->trigger();
}

PatternGeneratorBufferManagerUi::~PatternGeneratorBufferManagerUi()
{
}

void PatternGeneratorBufferManagerUi::setSampleRate()
{
	pattern_generator_ptr->close();
	options["samplerate"] = Glib::Variant<guint64>(g_variant_new_uint64(
	                                bufman->getSampleRate()),true);//(Glib::VariantBase)(gint64(1000000));
	pattern_generator_ptr->options_ = options;
	pattern_generator_ptr->open();
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

void PatternGeneratorBufferManagerUi::dataChanged()
{
	pattern_generator_ptr->close();
	pattern_generator_ptr->data_ = bufman->buffer;
	pattern_generator_ptr->open();
	main_win->run_stop();
}

}
