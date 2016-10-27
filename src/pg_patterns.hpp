#ifndef PG_PATTERNS_HPP
#define PG_PATTERNS_HPP

#include <QWidget>
#include <QPushButton>
#include <QJsonArray>
#include <QJsonObject>
#include <QIntValidator>
#include <QtQml/QJSEngine>
#include <QtUiTools/QUiLoader>
#include <vector>
#include <string>

// Generated UI
#include "ui_binarycounterpatternui.h"
#include "ui_uartpatternui.h"
#include "ui_lfsrpatternui.h"
#include "ui_clockpatternui.h"
#include "ui_genericjspatternui.h"
#include "ui_constantpattern.h"
#include "ui_numberpattern.h"
#include "ui_frequencypattern.h"
#include "ui_pulsepattern.h"
#include "ui_walkingpattern.h"


namespace Ui {
    class BinaryCounterPatternUI;
    class UARTPatternUI;
    class LFSRPatternUI;
    class ClockPatternUI;
    class GenericJSPatternUI;
    class ConstantPatternUI;
    class NumberPatternUI;
    class FrequencyPatternUI;
    class PulsePatternUI;
    class WalkingPatternUI;
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
    bool periodic;
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
    void set_periodic(bool periodic_);
    short* get_buffer();
    void delete_buffer();
    virtual void init();
    virtual uint8_t pre_generate();
    virtual bool is_periodic();
    virtual uint32_t get_min_sampling_freq();
    virtual uint32_t get_required_nr_of_samples();
    virtual uint8_t generate_pattern() = 0;
    virtual void deinit();
};

/*
 * How to derive Pattern
 class SomePattern : virtual public Pattern
 {
 public:
 SomePattern();
 uint8_t generate_pattern();
 };

 SomePattern::SomePattern()
 {
 set_name("SomePattern");
 set_description("SomePatternDescription")
 }
 uint8_t SomePattern::generate_pattern()
 {
 return 0;
 }
*/

class PatternUI : public QWidget, public virtual Pattern
{
    Q_OBJECT
public:
    PatternUI(QWidget *parent = 0);
    ~PatternUI();
    virtual void build_ui(QWidget *parent = 0);
    virtual void post_load_ui();
    virtual void parse_ui();
    virtual void destroy_ui();
};


/*
 * How to derive PatternUI
 class SomePatternUI : public PatternUI, public SomePattern
 {
    Q_OBJECT
    Ui::SomePatternUI *ui;
    QWidget *parent_;
 public:
    SomePatternUI(QWidget *parent = 0);
    ~SomePatternUI();
    void parse_ui();
    void build_ui(QWidget *parent = 0);
    void destroy_ui();
 };


*/


class BinaryCounterPattern : virtual public Pattern
{
protected:
    uint32_t frequency;
    uint16_t start_value;
    uint16_t end_value;
    uint16_t increment;
    uint16_t init_value;
public:
    BinaryCounterPattern();
    virtual uint8_t generate_pattern();
    uint32_t get_frequency() const;
    void set_frequency(const uint32_t &value);
    uint32_t get_min_sampling_freq();
    uint32_t get_required_nr_of_samples();
    uint16_t get_start_value() const;
    void set_start_value(const uint16_t &value);
    uint16_t get_end_value() const;
    void set_end_value(const uint16_t &value);
    uint16_t get_increment() const;
    void set_increment(const uint16_t &value);
    uint16_t get_init_value() const;
    void set_init_value(const uint16_t &value);
};

class BinaryCounterPatternUI : public PatternUI, public BinaryCounterPattern
{
    Q_OBJECT
    Ui::BinaryCounterPatternUI *ui;
    QWidget *parent_;
public:
    BinaryCounterPatternUI(QWidget *parent = 0);
    ~BinaryCounterPatternUI();
    void parse_ui();
    void build_ui(QWidget *parent = 0);
    void destroy_ui();
};


class GrayCounterPattern : virtual public BinaryCounterPattern
{
public:
    GrayCounterPattern();
    uint8_t generate_pattern();
};

class RandomPattern : virtual public Pattern
{
protected:
    uint32_t frequency;
public:
    RandomPattern();
    uint32_t get_min_sampling_freq();
    uint32_t get_required_nr_of_samples();
    uint32_t get_frequency() const;
    void set_frequency(const uint32_t &value);
    uint8_t generate_pattern();
};

class RandomPatternUI : public PatternUI, public RandomPattern
{
    Q_OBJECT
    Ui::FrequencyPatternUI *ui;
    QWidget *parent_;
public:
    RandomPatternUI(QWidget *parent = 0);
    ~RandomPatternUI();
    void parse_ui();
    void build_ui(QWidget *parent = 0);
    void destroy_ui();
};


class GrayCounterPatternUI : public PatternUI, public GrayCounterPattern
{
    Q_OBJECT
    Ui::BinaryCounterPatternUI *ui;
    QWidget *parent_;
public:
    GrayCounterPatternUI(QWidget *parent = 0);
    ~GrayCounterPatternUI();
    void parse_ui();
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
    uint32_t get_min_sampling_freq();
    uint32_t get_required_nr_of_samples();
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
    void parse_ui();
    /*
private Q_SLOTS:
    void on_setUARTParameters_clicked();*/
};

class JSPattern : public QObject, virtual public Pattern
{
    Q_OBJECT
private:
protected:
    QJSEngine *qEngine;
    JSConsole *console;
    QWidget *ui_form;
    QJsonObject obj;
public:

    JSPattern(QJsonObject obj_);
    Q_INVOKABLE quint32 get_nr_of_samples();
    Q_INVOKABLE quint32 get_nr_of_channels();
    Q_INVOKABLE quint32 get_sample_rate();
    /*Q_INVOKABLE*/ void JSErrorDialog(QString errorMessage);
    /*Q_INVOKABLE*/ void commitBuffer(QJSValue jsBufferValue, QJSValue jsBufferSize);
    bool is_periodic();
    uint32_t get_min_sampling_freq();
    uint32_t get_required_nr_of_samples();
    void init();
    uint8_t pre_generate();
    uint8_t generate_pattern();
    void deinit();
    virtual bool handle_result(QJSValue result,QString str = "");
};

class JSPatternUIStatusWindow : public QObject
{
    Q_OBJECT
    QTextEdit *con;
public:
    JSPatternUIStatusWindow(QTextEdit *textedit);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void print(QString str);
};


class JSPatternUI : public PatternUI, public JSPattern
{
    QUiLoader *loader;
    Ui::GenericJSPatternUI *ui;
    //QWidget *ui_form;
    QString form_name;
    QWidget *parent_;
    JSPatternUIStatusWindow *textedit;
public:
    JSPatternUI(QJsonObject obj_, QWidget *parent = 0);
    ~JSPatternUI();

    bool handle_result(QJSValue result,QString str = "");
    void find_all_children(QObject* parent, QJSValue property);
    void build_ui(QWidget *parent = 0);
    void post_load_ui();
    void parse_ui();
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
    LFSRPatternUI(QWidget *parent = 0);
    ~LFSRPatternUI();
    void build_ui(QWidget *parent = 0);
    void parse_ui();
    void destroy_ui();
/*private Q_SLOTS:
    void on_setLFSRParameters_clicked();*/
};

class ClockPattern : virtual public Pattern
{
    int duty_cycle_granularity = 20;
    float frequency;
    float duty_cycle;
    int phase;
public:
    ClockPattern();
    uint8_t generate_pattern();
    float get_frequency() const;
    void set_frequency(float value);
    float get_duty_cycle() const;
    void set_duty_cycle(float value);
    int get_phase() const;
    void set_phase(int value);
    uint32_t get_min_sampling_freq();
    uint32_t get_required_nr_of_samples();

};

class ClockPatternUI : public PatternUI, public ClockPattern
{
    Q_OBJECT
    Ui::ClockPatternUI *ui;
    QWidget *parent_;
public:
    ClockPatternUI(QWidget *parent = 0);
    ~ClockPatternUI();
    void parse_ui();
    void build_ui(QWidget *parent = 0);
    void destroy_ui();
};

class ConstantPattern : virtual public Pattern
{
private:
    bool constant;
public:
    ConstantPattern();
    uint8_t generate_pattern();
    bool get_constant() const;
    void set_constant(bool value);
};

class ConstantPatternUI : public PatternUI, public ConstantPattern
{
    Q_OBJECT
    Ui::ConstantPatternUI *ui;
    QWidget *parent_;
public:
    ConstantPatternUI(QWidget *parent = 0);
    ~ConstantPatternUI();
    void parse_ui();
    void build_ui(QWidget *parent = 0);
    void destroy_ui();
};

class NumberPattern : virtual public Pattern
{
private:
    uint16_t nr;
public:
    NumberPattern();
    uint8_t generate_pattern();

    uint16_t get_nr() const;
    void set_nr(const uint16_t &value);
};

class NumberPatternUI : public PatternUI, public NumberPattern
{
    Q_OBJECT
    Ui::NumberPatternUI *ui;
    QWidget *parent_;
public:
    NumberPatternUI(QWidget *parent = 0);
    ~NumberPatternUI();
    void parse_ui();
    void build_ui(QWidget *parent = 0);
    void destroy_ui();
};


class PulsePattern : virtual public Pattern
{
protected:
    bool start;
    uint32_t low_number_of_samples;
    uint32_t high_number_of_samples;
    uint32_t counter_init;
    uint16_t divider;
    uint16_t divider_init;

public:
    PulsePattern();
    uint8_t generate_pattern();
    uint32_t get_min_sampling_freq();
    uint32_t get_required_nr_of_samples();
    bool get_start();
    uint32_t get_low_number_of_samples();
    uint32_t get_high_number_of_samples();
    uint32_t get_counter_init();
    uint16_t get_divider();
    uint16_t get_divider_init();
    void set_start(bool val);
    void set_low_number_of_samples(uint32_t val);
    void set_high_number_of_samples(uint32_t val);
    void set_counter_init(uint32_t val);
    void set_divider(uint16_t val);
    void set_divider_init(uint16_t val);

};

class PulsePatternUI : public PatternUI, public PulsePattern
{
    Q_OBJECT
    Ui::PulsePatternUI *ui;
    QWidget *parent_;
public:
    PulsePatternUI(QWidget *parent = 0);
    ~PulsePatternUI();
    void parse_ui();
    void build_ui(QWidget *parent = 0);
    void destroy_ui();
};

class JohnsonCounterPattern : virtual public Pattern
{
    uint32_t frequency;
public:
    JohnsonCounterPattern();
    uint8_t generate_pattern();
    uint32_t get_min_sampling_freq();
    uint32_t get_required_nr_of_samples();
    uint32_t get_frequency() const;
    void set_frequency(const uint32_t &value);
};

class JohnsonCounterPatternUI: public PatternUI, public JohnsonCounterPattern
{
    Q_OBJECT
    Ui::FrequencyPatternUI *ui;
    QWidget *parent_;
public:
    JohnsonCounterPatternUI(QWidget *parent = 0);
    ~JohnsonCounterPatternUI();
    void parse_ui();
    void build_ui(QWidget *parent = 0);
    void destroy_ui();
};

class WalkingPattern : virtual public Pattern
{
    uint32_t frequency;
    uint16_t length;
    bool right;
    bool level;
public:
    WalkingPattern();
    uint8_t generate_pattern();
    uint32_t get_min_sampling_freq();
    uint32_t get_required_nr_of_samples();

    uint32_t get_frequency() const;
    void set_frequency(const uint32_t &value);
    uint16_t get_length() const;
    void set_length(const uint16_t &value);
    bool get_right() const;
    void set_right(bool value);
    bool get_level() const;
    void set_level(bool value);
};

class WalkingPatternUI: public PatternUI, public WalkingPattern
{
    Q_OBJECT
    Ui::WalkingPatternUI *ui;
    QWidget *parent_;
public:
    WalkingPatternUI(QWidget *parent = 0);
    ~WalkingPatternUI();
    void parse_ui();
    void build_ui(QWidget *parent = 0);
    void destroy_ui();
};

}
#endif
