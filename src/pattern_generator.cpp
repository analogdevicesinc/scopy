#include <iio.h>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QFile>
#include <QTimer>
#include <QtQml/QJSEngine>
#include <QDirIterator>
#include <QPushButton>
#include <QHBoxLayout>

///* pulseview and sigrok */
#include "pulseview/pv/mainwindow.hpp"
#include "pulseview/pv/devices/binarybuffer.hpp"
#include "pulseview/pv/devicemanager.hpp"
#include "pulseview/pv/toolbars/mainbar.hpp"
#include "libsigrokcxx/libsigrokcxx.hpp"
#include "libsigrokdecode/libsigrokdecode.h"

#include "boost/thread.hpp"
#include <libserialport.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <vector>
#include <string.h>

#if _UNIX
#include <unistd.h>     //check if UNIX or WIN and handle includes
#endif
#if _WIN32
#include <windows.h>
#include <io.h>
#endif

#include <boost/thread.hpp>

#include "pattern_generator.hpp"

// Generated UI
#include "ui_pattern_generator.h"
#include "ui_binarycounterpatternui.h"
#include "ui_uartpatternui.h"




using namespace std;
using namespace adiscope;

namespace pv {
class MainWindow;
class DeviceManager;
class Session;

namespace toolbars {
class MainBar;
}

namespace widgets {
class DeviceToolButton;
}

}

namespace sigrok {
class Context;
}

namespace Glibmm {

}

namespace adiscope {


/*QStringList PatternGenerator::digital_trigger_conditions = QStringList()
        << "edge-rising"
        << "edge-falling"
        << "edge-any"
        << "level-low"
        << "level-high";

*/

QStringList PatternGenerator::possibleSampleRates = QStringList()
        << "80000000"
        << "40000000"   << "20000000"  << "10000000"
        << "5000000"    << "2000000"   << "1000000"
        << "500000"     << "200000"    << "100000"
        << "50000"      << "20000"     << "10000"
        << "5000"       << "2000"      << "1000"
        << "500"        << "200"       << "100"
        << "50"         << "20"        << "10"
        << "5"          << "2"         << "1";

PatternGenerator::PatternGenerator(struct iio_context *ctx, Filter *filt, QPushButton *runBtn, QWidget *parent) :
    QWidget(parent),
    ctx(ctx),
    dev(iio_context_find_device(ctx, "m2k-logic-analyzer-tx")),
    channel_manager_dev(iio_context_find_device(ctx, "m2k-logic-analyzer")),
    menuOpened(true),
    settings_group(new QButtonGroup(this)), menuRunButton(runBtn),
    ui(new Ui::PatternGenerator),
    txbuf(0), sample_rate(100000), channel_enable_mask(0xffff),buffer(nullptr),
    buffer_created(0), currentUI(nullptr)
{
    // UI
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    this->settings_group->setExclusive(true);

    // IIO
    this->no_channels = iio_device_get_channels_count(channel_manager_dev);

    //sigrok and sigrokdecode initialisation
    context = sigrok::Context::create();
    pv::DeviceManager device_manager(context);
    pv::MainWindow* w = new pv::MainWindow(device_manager, "pattern_generator", "",parent);
    binary_format = w->get_format_from_string("binary");


    /* setup PV plot view */
    main_win = w;
    ui->horizontalLayout->removeWidget(ui->centralWidget);
    ui->horizontalLayout->insertWidget(0, static_cast<QWidget* >(main_win));

    /* setup toolbar */
    pv::toolbars::MainBar* main_bar = main_win->main_bar_;
    QPushButton *btnDecoder = new QPushButton();
    btnDecoder->setIcon(QIcon::fromTheme("add-decoder", QIcon(":/icons/add-decoder.svg")));
    btnDecoder->setMenu(main_win->menu_decoder_add());
    ui->gridLayout->addWidget(btnDecoder);
    ui->gridLayout->addWidget(static_cast<QWidget *>(main_bar));

    int i = 0;

    BinaryCounterPatternUI *bcpu = new BinaryCounterPatternUI(this);
    UARTPatternUI *upu = new UARTPatternUI(this);
    patterns.push_back(dynamic_cast<PatternUI*>(bcpu));
    patterns.push_back(dynamic_cast<PatternUI*>(upu));

    for(auto &var : patterns)
    {
        ui->scriptCombo->addItem(QString::fromStdString(var->get_name()));
    }


    QString searchPattern = "generator.json";
    QDirIterator it("patterngenerator", QStringList() << searchPattern, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QFile file;
        QString filename = it.next();

        file.setFileName(filename);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QJsonDocument d = QJsonDocument::fromJson(file.readAll());
        file.close();        
        QJsonObject obj(d.object());

        filename.chop(searchPattern.length());
        obj.insert("filepath",filename);

        if(obj["enabled"] == true)
        {
            ui->scriptCombo->addItem(obj["name"].toString());
            if(obj.contains("description"))
                ui->scriptCombo->setItemData(i,obj["description"].toString(),Qt::ToolTipRole);
            i++;
            JSPatternUI *jspu = new JSPatternUI(obj, this);
            patterns.push_back(dynamic_cast<PatternUI*>(jspu));
        }
        qDebug()<<obj;
    }

    ui->sampleRateCombo->addItems(possibleSampleRates);
    for(i=0;i<possibleSampleRates.length()-1;i++)
        if(sample_rate <= possibleSampleRates[i].toInt() && sample_rate > possibleSampleRates[i+1].toInt())
            break;
    ui->sampleRateCombo->setCurrentIndex(i);
    sampleRateValidator = new QIntValidator(1,80000000,this);
    ui->sampleRateCombo->setValidator(sampleRateValidator);

    ui->ChannelEnableMask->setText("0xffff");
    ui->ChannelsToGenerate->setText("0x0001");
    ui->numberOfSamples->setText("2048");


    connect(ui->btnRunStop, SIGNAL(toggled(bool)), this, SLOT(startStop(bool)));
    connect(runBtn, SIGNAL(toggled(bool)), ui->btnRunStop, SLOT(setChecked(bool)));
    connect(ui->btnRunStop, SIGNAL(toggled(bool)), runBtn, SLOT(setChecked(bool)));

    connect(ui->btnSingleRun, SIGNAL(pressed()),this,SLOT(singleRun()));
    connect(ui->btnSettings, SIGNAL(pressed()),
            this, SLOT(toggleRightMenu()));

}

PatternGenerator::~PatternGenerator()
{
    stopPatternGeneration();

    for(auto var : patterns)
    {
        delete var;
    }
    delete ui;
    delete sampleRateValidator;
    // Destroy libsigrokdecode
    srd_exit();
}

void PatternGenerator::createBinaryBuffer()
{
    options["numchannels"] = Glib::Variant<gint32>(g_variant_new_int32(no_channels),true);//(Glib::VariantBase)(gint32(16));
    options["samplerate"] = Glib::Variant<guint64>(g_variant_new_uint64(sample_rate),true);//(Glib::VariantBase)(gint64(1000000));
    std::shared_ptr<pv::devices::BinaryBuffer> patern_generator_ptr( new pv::devices::BinaryBuffer(context,buffer,&buffersize,binary_format,options));
    main_win->select_device(patern_generator_ptr);
}

void PatternGenerator::dataChanged()
{
    main_win->run_stop();
}

bool PatternGenerator::startPatternGeneration(bool cyclic)
{

    /* Enable Tx channels*/
    char temp_buffer[12];

    if(!channel_manager_dev || !dev)
    {
        qDebug("Devices not found");
        return false;
    }
    qDebug("Setting channel direction");
    for (int j = 0; j < no_channels; j++) {
        if(channel_enable_mask & (1<<j)) {
            auto ch = iio_device_get_channel(channel_manager_dev, j);
            iio_channel_attr_write(ch, "direction", "out");
        }
    }

    qDebug("Setting sample rate");
    /* Set sample rate   */

    iio_device_attr_write(dev, "sampling_frequency", std::to_string(sample_rate).c_str());
    qDebug("Enabling channels");
    for (int j = 0; j < no_channels; j++) {
        auto ch = iio_device_get_channel(dev, j);
        iio_channel_enable(ch);
    }

    /* Create buffer     */
    qDebug("Creating buffer");
    txbuf = iio_device_create_buffer(dev, number_of_samples, cyclic);
    if(!txbuf)
    {
        qDebug("Could not create buffer - errno: %d - %s", errno, strerror(errno));
        return false;
    }
    buffer_created = true;
    short *p_dat;
    ptrdiff_t p_inc;

    int i = 0;
    for (p_dat = (short*)iio_buffer_start(txbuf); (p_dat < iio_buffer_end(txbuf)); (uint16_t*)p_dat++,i++)
    {
        *p_dat = buffer[i];
    }

    /* Push buffer       */
    auto number_of_bytes = iio_buffer_push(txbuf);
    qDebug("\nPushed %ld bytes to devices\r\n",number_of_bytes);
    return true;
}

void PatternGenerator::stopPatternGeneration()
{
    /* Destroy buffer */
    if(buffer_created == true) {
        iio_buffer_destroy(txbuf);
        buffer_created = false;
    }
    /* Reset Tx Channls*/
    auto nb_channels = iio_device_get_channels_count(channel_manager_dev);
    for (int j = 0; j < nb_channels; j++) {
        auto ch = iio_device_get_channel(channel_manager_dev, j);
        iio_channel_attr_write(ch, "direction", "in");
    }
}

void PatternGenerator::startStop(bool start)
{
    main_win->action_view_zoom_fit()->trigger();
    if (start)
    {
        if(startPatternGeneration(true))
            ui->btnRunStop->setText("Stop");
        else
            qDebug("Pattern generation failed");
    }
    else
    {
        stopPatternGeneration();
        ui->btnRunStop->setText("Run");
    }
}

void PatternGenerator::singleRun()
{
    main_win->action_view_zoom_fit()->trigger();
    stopPatternGeneration();
    if(startPatternGeneration(false))
    {
        uint32_t time_until_buffer_destroy = 1000 + (uint32_t)((number_of_samples/((float)sample_rate))*1000.0);
        qDebug("Time until buffer destroy %d", time_until_buffer_destroy);
        QTimer::singleShot(time_until_buffer_destroy, this, SLOT(singleRunStop()));
        qDebug("Pattern generation single started");
        ui->btnSingleRun->setChecked(false);
    }
    else
    {
        qDebug("Pattern generation failed");
        ui->btnSingleRun->setChecked(true);
    }
}

void PatternGenerator::singleRunStop()
{
    qDebug("Pattern Generation stopped ");
    stopPatternGeneration();
    ui->btnSingleRun->setChecked(false);
}

void PatternGenerator::toggleRightMenu(QPushButton *btn)
{
    bool open = !menuOpened;
 //   ui->rightWidget->toggleMenu(open); TEMP
    this->menuOpened = open;
}

void PatternGenerator::toggleRightMenu()
{
    toggleRightMenu(static_cast<QPushButton *>(QObject::sender()));
}


} /* namespace adiscope */


void adiscope::PatternGenerator::on_sampleRateCombo_activated(const QString &arg1)
{
    sample_rate = arg1.toInt();
}

void adiscope::PatternGenerator::on_generateScript_clicked()
{

    bool ok;
    channel_group = ui->ChannelsToGenerate->text().toShort(&ok,16);
    if(!ok) {qDebug()<< "could not convert to hex";return;}
    number_of_samples = ui->numberOfSamples->text().toLong();
    last_sample = number_of_samples-100;
    start_sample = ui->startingSample->text().toLong();
    if(buffersize != number_of_samples * 2 && buffer != nullptr){
        delete buffer;
        buffer=nullptr;
    }
    if(!buffer)
    {
        buffer = new short[number_of_samples];
        buffersize = number_of_samples * sizeof(short);
        memset(buffer, 0x0000, (number_of_samples)*sizeof(short));
    }

    PatternUI *current;
    current = patterns[ui->scriptCombo->currentIndex()];
    current->set_number_of_channels(get_nr_of_channels());
    current->set_number_of_samples(get_nr_of_samples());
    current->set_sample_rate(sample_rate);
    if(current->generate_pattern() != 0) {qDebug()<<"Pattern Generation failed";return;} //ERROR TEMPORARY

    commitBuffer(current->get_buffer());
    createBinaryBuffer();
    current->delete_buffer();
    dataChanged();
    main_win->action_view_zoom_fit()->trigger();

}

uint32_t adiscope::PatternGenerator::get_nr_of_samples()
{
    return last_sample - start_sample;
}

uint32_t adiscope::PatternGenerator::get_nr_of_channels()
{
    int i=0;
    int channel_mask_temp = channel_group;
    while(channel_mask_temp)
    {
        i=i+(channel_mask_temp & 0x01);
        channel_mask_temp>>=1;
    }
    return i;
}

uint32_t adiscope::PatternGenerator::get_sample_rate()
{
    return sample_rate;
}


short PatternGenerator::convert_jsbuffer(uint8_t *mapping, uint32_t val)
{
    short ret=0;
    int i=0;
    while(val)
    {
        if(val&0x01)
        {
            ret = ret | (1<<mapping[i]);
        }
        i++;
        val>>=1;
    }
    return ret;
}

void adiscope::PatternGenerator::commitBuffer(short *bufferPtr)
{
    uint8_t channel_mapping[no_channels];

    int i=0,j=0;
    auto channel_enable_mask_temp = channel_group;
    auto buffer_channel_mask = (1<<get_nr_of_channels())-1;
    while(channel_enable_mask_temp)
    {
        if(channel_enable_mask_temp & 0x01) {
            channel_mapping[j] = i;
            j++;
        }
        channel_enable_mask_temp>>=1;
        i++;
    }

    for(auto i=start_sample;i<last_sample;i++)
    {
        auto val = (bufferPtr[i-start_sample] & buffer_channel_mask);
        buffer[i] = (buffer[i] & ~channel_group) | convert_jsbuffer(channel_mapping, val);
    }
}

void adiscope::PatternGenerator::on_clearButton_clicked()
{
    memset(buffer, 0x0000, (number_of_samples)*sizeof(short));
    createBinaryBuffer();
    dataChanged();
    main_win->action_view_zoom_fit()->trigger();
}

void adiscope::PatternGenerator::on_generateUI_clicked()
{
    if(currentUI!=nullptr)
    {
        currentUI->setVisible(false);
        currentUI->destroy_ui();
   //     ui->rightWidgetPage2->layout()->removeWidget(currentUI);
    }

    currentUI = patterns[ui->scriptCombo->currentIndex()];

  //  ui->rightWidgetPage2->layout()->addWidget(current);
    currentUI->build_ui(ui->rightWidgetPage2);
    currentUI->setVisible(true);
}

/////////////////////////////////////////////////
//////// PATTERNS //////////////////////////////
////////////////////////////////////////////////

JSConsole::JSConsole(QObject *parent) :
    QObject(parent)
{
}

void JSConsole::log(QString msg)
{
    qDebug() << "jsConsole: "<< msg;
}

Pattern::Pattern()
{
    qDebug()<<"PatternCreated";
    buffer = nullptr;
}

Pattern::~Pattern()
{
    qDebug()<<"PatternDestroyed";
    delete_buffer();
}

string Pattern::get_name()
{
    return name;
}

void Pattern::set_name(string name_)
{
    name = name_;
}

string Pattern::get_description()
{
    return description;
}

void Pattern::set_description(string description_)
{
    description = description_;
}

void Pattern::set_sample_rate(uint32_t sample_rate_)
{
    sample_rate = sample_rate_;
}

void Pattern::set_number_of_samples(uint32_t number_of_samples_)
{
    number_of_samples = number_of_samples_;
}
void Pattern::set_number_of_channels(uint16_t number_of_channels_)
{
    number_of_channels = number_of_channels_;
}

short* Pattern::get_buffer()
{
    return buffer;
}

void Pattern::delete_buffer()
{
    if(buffer)
        delete buffer;
    buffer=nullptr;
}

PatternUI::PatternUI(QWidget *parent) : QWidget(parent){
    qDebug()<<"PatternUICreated";
}
PatternUI::~PatternUI(){
    qDebug()<<"PatternUIDestroyed";
}
void PatternUI::build_ui(QWidget *parent){}
void PatternUI::destroy_ui(){}

BinaryCounterPattern::BinaryCounterPattern()
{
    qDebug()<<"BinaryCounterPatternCreated";
    set_name("BinaryCounter");
    set_description("BinaryCounterDescription");
}

uint8_t BinaryCounterPattern::generate_pattern()
{
    delete_buffer();
    buffer = new short[number_of_samples];
    for(auto i=0;i<number_of_samples;i++)
    {
        buffer[i] = i % (1<<number_of_channels);
    }
    return 0;
}

BinaryCounterPatternUI::BinaryCounterPatternUI(QWidget *parent) : PatternUI(parent){
    qDebug()<<"BinaryCounterPatternUI Created";
    ui = new Ui::BinaryCounterPatternUI();
    ui->setupUi(this);
    setVisible(false);
}
BinaryCounterPatternUI::~BinaryCounterPatternUI(){
              qDebug()<<"BinaryCounterPatternUI Destroyed";
}
void BinaryCounterPatternUI::build_ui(QWidget *parent){
    parent_ = parent;
    parent->layout()->addWidget(this);

}
void BinaryCounterPatternUI::destroy_ui(){
    parent_->layout()->removeWidget(this);
    delete ui;
}

UARTPattern::UARTPattern()
{
    qDebug()<<"UARTPattern Created";
    parity = SP_PARITY_NONE;
    stop_bits = 1;
    baud_rate = 9600;
    data_bits = 8;
    msb_first=false;
    set_name("UART");
    set_description("UARTDescription");

}

void UARTPattern::set_string(std::string str_)
{
    str = str_;
}

int UARTPattern::set_params(std::string params_)
{
    // https://github.com/analogdevicesinc/libiio/blob/master/serial.c#L426
    const char *params = params_.c_str();
    char *end;

    baud_rate = strtoul(params, &end, 10);
    if (params == end)
        return -EINVAL;

    auto uart_format = strchr(params,'/');
    if(uart_format == NULL) /* / not found, use default settings*/
    {
        data_bits = 8;
        parity = SP_PARITY_NONE;
        stop_bits = 1;
        return 0;
    }

    uart_format++;

    data_bits = strtoul(uart_format, &end, 10);
    if (params == end)
        return -EINVAL;

    char lowercase_parity = tolower(*end);
    switch (lowercase_parity) {
    case 'n':
        parity = SP_PARITY_NONE;
        break;
    case 'o':
        parity = SP_PARITY_ODD;
        break;
    case 'e':
        parity = SP_PARITY_EVEN;
        break;
    case 'm':
        parity = SP_PARITY_MARK;
        break;
    case 's':
        parity = SP_PARITY_SPACE;
        break;
    default:
        return -EINVAL;
    }
    end++;
    uart_format = end;
    stop_bits = strtoul(uart_format, &end, 10);
    if (params == end)
        return -EINVAL;

    return 0;
}

void UARTPattern::set_msb_first(bool msb_first_)
{
    msb_first = msb_first_;
}

uint16_t UARTPattern::encapsulateUartFrame(char chr, uint16_t *bits_per_frame)
{
    uint16_t ret = 0xffff;
    bool parity_bit_available;
    uint16_t parity_bit_value = 1;
    auto chr_to_test = chr;

    switch(parity)
    {
    case SP_PARITY_NONE:
        parity_bit_available = false;
        break;
    case SP_PARITY_ODD:
        parity_bit_value = 1;
        parity_bit_available = true;
        for(auto i=0;i<data_bits;i++)
        {
            parity_bit_value = parity_bit_value ^ (chr_to_test & 0x01);
            chr_to_test = chr_to_test>>1;
        }
        break;
    case SP_PARITY_EVEN:
        parity_bit_value = 0;
        parity_bit_available = true;
        for(auto i=0;i<data_bits;i++)
        {
            parity_bit_value = parity_bit_value ^ (chr_to_test & 0x01);
            chr_to_test = chr_to_test>>1;
        }
        break;
    case SP_PARITY_MARK:
        parity_bit_available = true;
        parity_bit_value = 1;
        break;
    case SP_PARITY_SPACE:
        parity_bit_available = true;
        parity_bit_value = 0;
        break;
    }

    if(!msb_first)
    {
        ret = chr;
        ret = ret << 1; // start bit
        uint16_t stop_bit_values;

/*      if(parity_bit_available)
        {
            stop_bit_values = ((1<<stop_bits+1)-1) & parity_bit_value; // parity bit value is cleared
        }
        else
            stop_bit_values = ((1<<stop_bits)-1); // if parity bit not availabe, stop bits will not be incremented
*/
        stop_bit_values = ((1<<(stop_bits+parity_bit_available))-1) & ((parity_bit_value) ? (0xffff) : (0xfffe)); // todo: Simplify this
        ret = ret | stop_bit_values << (data_bits+1);


    }
    else
    {
        ret = (~(1<<data_bits)); // start bit
        ret = ret & chr;
        if(parity_bit_available)
        {
            ret = (ret << 1) | parity_bit_value;
        }
        for(auto i=0;i<stop_bits;i++)
            ret = (ret << 1) | 0x01;
    }

    (*bits_per_frame) = data_bits + 1 + stop_bits + parity_bit_available;

    return ret;

}



uint8_t UARTPattern::generate_pattern()
{
    delete_buffer();


    uint16_t number_of_frames = str.length();
    uint32_t samples_per_bit = sample_rate/baud_rate;
    uint16_t bits_per_frame;
    encapsulateUartFrame(*(str.c_str()), &bits_per_frame);
    uint32_t samples_per_frame = samples_per_bit * bits_per_frame;
    number_of_samples = samples_per_frame*(number_of_frames + 1/* padding */);

    buffer = new short[number_of_samples]; // no need to recreate buffer
    auto buffersize = (number_of_samples)*sizeof(short);
    memset(buffer, 0xffff, (number_of_samples)*sizeof(short));

    short *buf_ptr = buffer;
    const char *str_ptr = str.c_str();
    int i;

    for(i=0;i<samples_per_frame/2;i++) // pad with half a frame
    {
        *buf_ptr = 1;
        buf_ptr++;
    }
    for(i=0;i<str.length();i++,str_ptr++)
    {
        auto frame_to_send = encapsulateUartFrame(*str_ptr, &bits_per_frame);
        for(auto j=0;j<bits_per_frame;j++)
        {
            short bit_to_send;
            if(!msb_first)
            {
                bit_to_send = (frame_to_send & 0x01 );
                frame_to_send = frame_to_send >> 1;
            }
            else
            {
                bit_to_send = ((frame_to_send & (1<<(bits_per_frame-1))) ? 1 : 0 ); // set bit here
                frame_to_send = frame_to_send << 1;
            }
            for(auto k=0;k<samples_per_bit;k++,buf_ptr++) {
                *buf_ptr =  bit_to_send;// set bit here
            }
        }
    }
    for(i=0;i<samples_per_frame/2;i++) // pad with half a frame
    {
        *buf_ptr = 1;
        buf_ptr++;
    }
    return 0;
}

UARTPatternUI::UARTPatternUI(QWidget *parent) : PatternUI(parent)
{
    qDebug()<<"UARTPatternUI created";
    set_params("9600/8N1");
    set_string("hello World");

    ui = new Ui::UARTPatternUI();
    ui->setupUi(this);
    setVisible(false);
}

UARTPatternUI::~UARTPatternUI(){
    qDebug()<<"UARTPatternUI destroyed";
}
void UARTPatternUI::build_ui(QWidget *parent){
    parent_ = parent;
    parent->layout()->addWidget(this);
}
void UARTPatternUI::destroy_ui(){
    parent_->layout()->removeWidget(this);
    delete ui;
}

JSPattern::JSPattern(QJsonObject obj_) : obj(obj_){
    qDebug()<<"JSPattern created";
}
uint8_t JSPattern::generate_pattern()
{
    JSPattern *pg = this;

    QString fileName(obj["filepath"].toString() + obj["generate_script"].toString());
    qDebug()<<fileName;
    QFile scriptFile(fileName);
    scriptFile.open(QIODevice::ReadOnly);
    QTextStream stream(&scriptFile);
    QString contents = stream.readAll();
    scriptFile.close();

    JSConsole console;
    QJSValue consoleObj =  qEngine.newQObject(&console);

    /*QJsonObject deviceJson;
    deviceJson.insert("sample_rate",(gint32)sample_rate);
    deviceJson.insert("number_of_samples",(gint32)number_of_samples);
    QJSValue jsonObj = qEngine.toScriptValue(deviceJson);*/

    bool ok;

    /*buffer = new short[number_of_samples];
    buffersize = number_of_samples * sizeof(short);
    memset(buffer, 0x0000, (number_of_samples)*sizeof(short));*/


    QJSValue pgObj =  qEngine.newQObject(pg);
    qEngine.globalObject().setProperty("pg",pgObj);
    qEngine.globalObject().setProperty("console", consoleObj);
    //qEngine.globalObject().setProperty("dev",jsonObj);
    QJSValue result = qEngine.evaluate(contents, fileName);

    if(result.isError())
    {
        qDebug()<<"errorCode: " <<result.toString();
        return -2;
    }
    else
        qDebug("Success");
    return 0;
}

quint32 JSPattern::get_nr_of_samples()
{
    return number_of_samples;
}

quint32 JSPattern::get_nr_of_channels()
{
    return number_of_channels;
}

quint32 JSPattern::get_sample_rate()
{
    return sample_rate;
}

void JSPattern::JSErrorDialog(QString errorMessage)
{
    qDebug()<<"JSErrorDialog: "<<errorMessage;
}

void JSPattern::commitBuffer(QJSValue jsBufferValue, QJSValue jsBufferSize)
{
    if(!jsBufferValue.isArray()) {qDebug()<<"Not an array";return;}
    if(!jsBufferSize.isNumber()) {qDebug()<<"Not a valid size";return;}

    delete_buffer();
    buffer = new short[jsBufferSize.toInt()];

    for(auto i=0;i<jsBufferSize.toInt();i++)
    {
        if(!jsBufferValue.property(i).isError())
        {
            auto val = jsBufferValue.property(i).toInt();
            buffer[i] = val;
        }
        else
        {
            buffer[i] = 0;
        }
    }
}

JSPatternUI::JSPatternUI(QJsonObject obj_, QWidget *parent) : JSPattern(obj_), PatternUI(parent){
    qDebug()<<"JSPatternUI created";
}
JSPatternUI::~JSPatternUI(){
    qDebug()<<"JSPatternUI destroyed";
}
void JSPatternUI::build_ui(QWidget *parent){}
void JSPatternUI::destroy_ui(){}



void adiscope::UARTPatternUI::on_setUARTParameters_clicked()
{
    ui->paramsOut->setText(ui->baudCombo->currentText() + "/8" +ui->parityCombo->currentText()[0] + ui->stopCombo->currentText());
    qDebug()<<ui->paramsOut->text();
    set_params(ui->paramsOut->text().toStdString());
    qDebug()<<ui->dataEdit->text();
    set_string(ui->dataEdit->text().toStdString());
}
