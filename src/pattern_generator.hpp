#ifndef PATTERN_GENERATOR_H
#define PATTERN_GENERATOR_H

#include <QWidget>
#include <QPushButton>
#include <vector>
#include <libserialport.h>

#include "src/pulseview/pv/devices/binarybuffer.hpp"
#include "iio_manager.hpp"
#include "filter.hpp"
#include "streams_to_short.h"
#include <gnuradio/blocks/file_descriptor_sink.h>

extern "C" {
  struct iio_context;
  struct iio_device;
  struct iio_channel;
  struct iio_buffer;
}

namespace pv {
    class MainWindow;
    class DeviceManager;
    namespace toolbars {
        class MainBar;
    }
}

namespace sigrok {
    class Context;
}

namespace Glibmm {

}

namespace Ui {
    class PatternGenerator;
}

namespace adiscope {
class PatternGenerator : public QWidget
{
    Q_OBJECT

public:
    explicit PatternGenerator(struct iio_context *ctx, Filter* filt, QPushButton *runButton, QWidget *parent = 0);
    ~PatternGenerator();

private Q_SLOTS:
    void startStop(bool start);
    void singleRun();
    void singleRunStop();
    void toggleRightMenu();

    void on_BinaryCounter_clicked();
    void on_UART_clicked();

private:
//    pv::devices::PatternGenerator *pattern_generator_device;
    std::shared_ptr<sigrok::Context> context;
    std::shared_ptr<pv::devices::BinaryBuffer> pattern_generator_ptr;
    std::shared_ptr<sigrok::InputFormat> binary_format;
    Ui::PatternGenerator *ui;
    QButtonGroup *settings_group;
    QPushButton *menuRunButton;
    boost::shared_ptr<iio_manager> manager;
    iio_manager::port_id* ids;
    struct iio_context *ctx;
    struct iio_device *dev;
    struct iio_device *channel_manager_dev;
    struct iio_buffer *txbuf;
    int no_channels;
    bool buffer_created;
    uint16_t channel_enable_mask;
    uint64_t sample_rate;

    std::map<std::string, Glib::VariantBase> options;

    short *buffer;

    uint64_t number_of_samples;
    uint64_t buffersize;
    bool startPatternGeneration(bool cyclic);
    void stopPatternGeneration();
    void dataChanged();

    pv::MainWindow* main_win;


    static int parseParamsUart(const char *params,
            unsigned int *baud_rate, unsigned int *bits,
            enum sp_parity *parity,  unsigned int *stop_bits);
    void createBinaryCounter(int maxCount, uint64_t sampleRate);
    void createUart(const char *str, uint16_t str_length, const char *params, uint64_t sample_rate_, uint32_t holdoff_time, uint16_t channel = 0, bool msb_first = 0);
    void createBinaryBuffer();
    uint16_t encapsulateUartFrame(char chr, uint16_t *bits_per_frame, uint16_t data_bits_per_frame, sp_parity parity, uint16_t stop_bits, bool msb_first);
    void toggleRightMenu(QPushButton *btn);
    bool menuOpened;

    static QStringList digital_trigger_conditions;

};
} /* namespace adiscope */

#endif // LOGIC_ANALYZER_H

