#ifndef PATTERN_GENERATOR_H
#define PATTERN_GENERATOR_H

#include <QWidget>
#include <QPushButton>
#include <QJsonArray>
#include <QIntValidator>
#include <QtQml/QJSEngine>
#include <QtUiTools/QUiLoader>
#include <vector>
#include <string>
#include <libserialport.h>

#include "src/pulseview/pv/devices/binarybuffer.hpp"
#include "filter.hpp"

#include "pg_patterns.hpp"

// Generated UI
#include "ui_pattern_generator.h"

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

namespace Ui {
class PatternGenerator;
/*    class BinaryCounterPatternUI;
    class UARTPatternUI;
    class LFSRPatternUI;
    class ClockPatternUI;
    class GenericJSPatternUI;*/
}

namespace adiscope {

class PatternUIFactory
{
    static QStringList ui_list;
    static QStringList description_list;
    static int static_ui_limit;
    static QJsonObject patterns;
public:
    static void init();
    static PatternUI *create_ui(int index,QWidget *parent = 0);
    static QStringList get_ui_list();
    static QStringList get_description_list();
private:
    PatternUIFactory() {}
};

class ChannelManager
{
    class ChannelGroup
    {
    private:
    public:
        class Channel
        {
        private:
            uint16_t mask;
        public:
            Channel(uint16_t mask);
            uint16_t get_mask();
        };

        std::vector<Channel> channels;
        ChannelGroup(Channel ch);
        void append(ChannelGroup tojoin);
        uint16_t get_mask();
    };

    std::vector<ChannelGroup> channel_group;
public:

    ChannelManager();
    void split(int index);
    void join(std::vector<int> index);
};




class PatternGenerator : public QWidget
{
    Q_OBJECT

public:
    explicit PatternGenerator(struct iio_context *ctx, Filter* filt, QPushButton *runButton, QWidget *parent = 0);
    ~PatternGenerator();

    uint32_t get_nr_of_samples();
    uint32_t get_nr_of_channels();
    uint32_t get_sample_rate();

    //std::vector channel_groups;

    short remap_buffer(uint8_t *mapping, uint32_t val);
    void commitBuffer(short *bufferPtr);


private Q_SLOTS:
    void startStop(bool start);
    void singleRun();
    void singleRunStop();
    void toggleRightMenu();
    void update_ui();
    void on_sampleRateCombo_activated(const QString &arg1);
    void on_generatePattern_clicked();
    void on_clearButton_clicked();
    void on_generateUI_clicked();

    void on_save_PB_clicked();

    void on_load_PB_clicked();

    void on_CreateGroup_clicked();

    void on_pushButton_clicked();
private:

    // UI
    Ui::PatternGenerator *ui;
    QButtonGroup *settings_group;
    QPushButton *menuRunButton;
    // QWidget *current;
    PatternUI *currentUI;
    QIntValidator *sampleRateValidator;
    uint16_t channel_group;

    // Buffer
    ChannelManager chm;
    short *buffer;
    bool buffer_created;
    uint32_t start_sample;
    uint32_t last_sample;
    uint32_t number_of_samples;
    uint32_t buffersize;

    // Device parameters

    uint16_t channel_enable_mask;
    uint32_t sample_rate;
    int no_channels;

    // IIO

    struct iio_context *ctx;
    struct iio_device *dev;
    struct iio_device *channel_manager_dev;
    struct iio_buffer *txbuf;


    // PV and Sigrok

    std::shared_ptr<sigrok::Context> context;
    std::shared_ptr<pv::devices::BinaryBuffer> pattern_generator_ptr;
    std::shared_ptr<sigrok::InputFormat> binary_format;
    std::map<std::string, Glib::VariantBase> options;
    pv::MainWindow* main_win;

    bool startPatternGeneration(bool cyclic);
    void stopPatternGeneration();
    void dataChanged();

    void createBinaryBuffer();
    void toggleRightMenu(QPushButton *btn);

    bool menuOpened;

    std::vector<PatternUI*> patterns;
    static QStringList digital_trigger_conditions;
    static QStringList possibleSampleRates;
};
} /* namespace adiscope */


#endif // LOGIC_ANALYZER_H

