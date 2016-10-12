#ifndef PATTERN_GENERATOR_H
#define PATTERN_GENERATOR_H

#include <QWidget>
#include <QPushButton>
#include <QJsonArray>
#include <QIntValidator>
#include <QtQml/QJSEngine>
#include <vector>
#include <string>
#include <libserialport.h>

#include "src/pulseview/pv/devices/binarybuffer.hpp"
#include "iio_manager.hpp"
#include "filter.hpp"
#include "streams_to_short.h"
#include <gnuradio/blocks/file_descriptor_sink.h>

// Generated UI
#include "ui_pattern_generator.h"
#include "ui_binarycounterpatternui.h"
#include "ui_uartpatternui.h"
#include "ui_lfsrpatternui.h"


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
    class BinaryCounterPatternUI;
    class UARTPatternUI;
    class LFSRPatternUI;
}

namespace adiscope {

// http://stackoverflow.com/questions/32040101/qjsengine-print-to-console
class JSConsole : public QObject // for logging purposes in QJSEngine
{
    Q_OBJECT
public:
    explicit JSConsole(QObject *parent = 0);
    Q_INVOKABLE void log(QString msg);
};


class Pattern
{
private:
    std::string name;
    std::string description;
protected: // temp
    short *buffer;
    uint32_t sample_rate;
    uint32_t number_of_samples;
    uint16_t number_of_channels;

public:

    Pattern(/*string name_, string description_*/);
    ~Pattern();
    std::string get_name();
    void set_name(std::string name_);
    std::string get_description();
    void set_description(std::string description_);
    void set_sample_rate(uint32_t sample_rate_);
    void set_number_of_samples(uint32_t number_of_samples_);
    void set_number_of_channels(uint16_t number_of_channels_);
    short* get_buffer();
    void delete_buffer();
    virtual uint8_t generate_pattern() = 0;
};

class PatternUI : public QWidget, public virtual Pattern
{
    Q_OBJECT
public:
    PatternUI(QWidget *parent = 0);
    ~PatternUI();
    virtual void build_ui(QWidget *parent = 0);
    virtual void destroy_ui();
};

class BinaryCounterPattern : virtual public Pattern
{
public:
    BinaryCounterPattern();
    uint8_t generate_pattern();
};

class BinaryCounterPatternUI : public PatternUI, public BinaryCounterPattern
{
    Ui::BinaryCounterPatternUI *ui;
    QWidget *parent_;
public:
    BinaryCounterPatternUI(QWidget *parent = 0);
    ~BinaryCounterPatternUI();
    void build_ui(QWidget *parent = 0);
    void destroy_ui();
};

class UARTPattern : virtual public Pattern
{
protected:
    std::string str;
    bool msb_first;
    unsigned int baud_rate;
    unsigned int data_bits;
    unsigned int stop_bits;
    enum sp_parity parity;


public:
    UARTPattern();
    void set_string(std::string str_);
    int set_params(std::string params_);
    void set_msb_first(bool msb_first_);
    uint16_t encapsulateUartFrame(char chr, uint16_t *bits_per_frame);
    uint8_t generate_pattern();
};

class UARTPatternUI : public PatternUI, public UARTPattern
{
    Q_OBJECT
    Ui::UARTPatternUI *ui;
    QWidget *parent_;
public:
    UARTPatternUI(QWidget *parent = 0);
    ~UARTPatternUI();
    void build_ui(QWidget *parent = 0);
    void destroy_ui();
private Q_SLOTS:
    void on_setUARTParameters_clicked();
};

class JSPattern : public QObject, virtual public Pattern
{
    Q_OBJECT
private:

    QJSEngine qEngine;
    QJsonObject obj;
public:

    JSPattern(QJsonObject obj_);
    Q_INVOKABLE quint32 get_nr_of_samples();
    Q_INVOKABLE quint32 get_nr_of_channels();
    Q_INVOKABLE quint32 get_sample_rate();
    Q_INVOKABLE void JSErrorDialog(QString errorMessage);
    Q_INVOKABLE void commitBuffer(QJSValue jsBufferValue, QJSValue jsBufferSize);
    uint8_t generate_pattern();
};

class JSPatternUI : public PatternUI, public JSPattern
{
public:
    JSPatternUI(QJsonObject obj_, QWidget *parent = 0);
    ~JSPatternUI();
    void build_ui(QWidget *parent = 0);
    void destroy_ui();
};

class LFSRPattern : virtual public Pattern
{
private:

    uint32_t lfsr_poly;
    uint16_t start_state;
    uint32_t lfsr_period;
public:
    LFSRPattern();
    uint8_t generate_pattern();

    uint32_t compute_period();
    uint32_t get_lfsr_poly() const;
    void set_lfsr_poly(const uint32_t &value);
    uint16_t get_start_state() const;
    void set_start_state(const uint16_t &value);
    uint32_t get_lfsr_period() const;
};

class LFSRPatternUI : public PatternUI, public LFSRPattern
{
    Q_OBJECT
    Ui::LFSRPatternUI *ui;
    QWidget *parent_;
public:
    LFSRPatternUI(QWidget *parent = 0) : PatternUI(parent)
    {
        qDebug()<<"LFSRPatternUI created";
        ui = new Ui::LFSRPatternUI();
        ui->setupUi(this);
        setVisible(false);
    }
    ~LFSRPatternUI();
    void build_ui(QWidget *parent = 0);
    void destroy_ui();
private Q_SLOTS:
    void on_setLFSRParameters_clicked();
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
    short convert_jsbuffer(uint8_t *mapping, uint32_t val);
    void commitBuffer(short *bufferPtr);


private Q_SLOTS:
    void startStop(bool start);
    void singleRun();
    void singleRunStop();
    void toggleRightMenu();

    void on_sampleRateCombo_activated(const QString &arg1);
    void on_generateScript_clicked();
    void on_clearButton_clicked();
    void on_generateUI_clicked();

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

