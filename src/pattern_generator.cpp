#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <vector>
#include <string.h>

#include <iio.h>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTimer>
#include <QFile>
#include <QtQml/QJSEngine>
#include <QtQml/QQmlEngine>
#include <QDirIterator>
#include <QPushButton>
#include <QFileDialog>

///* pulseview and sigrok */
#include <boost/math/common_factor.hpp>
#include "pulseview/pv/mainwindow.hpp"
#include "pulseview/pv/devices/binarybuffer.hpp"
#include "pulseview/pv/devicemanager.hpp"
#include "pulseview/pv/toolbars/mainbar.hpp"
#include "libsigrokcxx/libsigrokcxx.hpp"
#include "libsigrokdecode/libsigrokdecode.h"

#include "pattern_generator.hpp"

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
    pv::MainWindow* w = new pv::MainWindow(device_manager, filt, "pattern_generator", "",parent);
    binary_format = w->get_format_from_string("binary");


    /* setup PV plot view */
    main_win = w;
  //  ui->horizontalLayout->removeWidget(ui->centralWidget);
    ui->centralWidgetLayout->insertWidget(0, static_cast<QWidget* >(main_win));

    /* setup toolbar */
    pv::toolbars::MainBar* main_bar = main_win->main_bar_;
    QPushButton *btnDecoder = new QPushButton();
    btnDecoder->setIcon(QIcon::fromTheme("add-decoder", QIcon(":/icons/add-decoder.svg")));
    btnDecoder->setMenu(main_win->menu_decoder_add());
    ui->gridLayout->addWidget(btnDecoder);
    ui->gridLayout->addWidget(static_cast<QWidget *>(main_bar));

    int i = 0;

    PatternFactory::init();

    for(auto var : PatternFactory::get_ui_list())
    {
        ui->scriptCombo->addItem(var);
        ui->scriptCombo->setItemData(i, (PatternFactory::get_description_list())[i],Qt::ToolTipRole);
        i++;
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

    connect(ui->btnSingleRun, SIGNAL(pressed()), this, SLOT(singleRun()));
    connect(ui->btnSettings , SIGNAL(pressed()), this, SLOT(toggleRightMenu()));

    buffer = new short[1];
    chmui = new PatternGeneratorChannelManagerUI(0,main_win,&chm,this);
    ui->leftWidgetLayout->addWidget(chmui);
    chmui->update_ui();
    chmui->setVisible(true);


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

void adiscope::PatternGenerator::on_sampleRateCombo_activated(const QString &arg1)
{
    sample_rate = arg1.toInt();
}

void adiscope::PatternGenerator::on_generatePattern_clicked()
{

    bool ok;
    channel_group = selected_channel_group->get_mask();
    ui->ChannelsToGenerate->setText("0x"+QString().number(channel_group,16));
    number_of_samples = ui->numberOfSamples->text().toLong();
    last_sample = ui->lastSample->text().toLong();// number_of_samples;//-100;
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
    current = currentUI;// patterns[ui->scriptCombo->currentIndex()];

   if(current!=nullptr){
        //current->parse_ui();
        current->get_pattern()->set_number_of_channels(get_nr_of_channels());
        current->get_pattern()->set_number_of_samples(get_nr_of_samples());
        current->get_pattern()->set_sample_rate(sample_rate);

        qDebug()<<"pregenerate status: "<<current->get_pattern()->pre_generate();
        qDebug()<<"minimum sampling frequency"<<current->get_pattern()->get_min_sampling_freq(); // least common multiplier
        current->get_pattern()->set_sample_rate(current->get_pattern()->get_min_sampling_freq()); // TEMP
        qDebug()<<"minimum number of samples"<<current->get_pattern()->get_required_nr_of_samples(); // if not periodic, verify minimum, else least common multiplier with least common freq
        current->get_pattern()->set_sample_rate(sample_rate);


        if(current->get_pattern()->generate_pattern() != 0) {qDebug()<<"Pattern Generation failed";return;} //ERROR TEMPORARY
        /*if(current->number_of_samples>(last_sample-start_sample)) {qDebug()<<"Warning! not enough buffer space to generate whole pattern";}
    else {last_sample = current->number_of_samples+start_sample;}*/
        commitBuffer(current->get_pattern()->get_buffer());
        createBinaryBuffer();
        current->get_pattern()->delete_buffer();
        dataChanged();
        main_win->action_view_zoom_fit()->trigger();
    }

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

short PatternGenerator::remap_buffer(uint8_t *mapping, uint32_t val)
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
        buffer[i] = (buffer[i] & ~channel_group) | remap_buffer(channel_mapping, val);
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
        currentUI->get_pattern()->deinit();
        currentUI->setVisible(false);
        currentUI->destroy_ui();
        delete currentUI;
        currentUI = nullptr;
    }
    /*currentUI = PatternFactory::create_ui(ui->scriptCombo->currentIndex());

    currentUI->build_ui(ui->rightWidgetPage2);
    currentUI->get_pattern()->init();
    currentUI->post_load_ui();
    cu=rrentUI->setVisible(true);*/
}

void adiscope::PatternGenerator::on_save_PB_clicked()
{
    QFileDialog qfd;
    qfd.setDefaultSuffix("json");

    QString filename = qfd.getSaveFileName(this,tr("Save buffer"),".json",tr("Session buffer file (*.json)"));
    if(filename == 0)
        return;
    QFile savedsession( filename );

    savedsession.open( QIODevice::WriteOnly);
    QJsonObject obj;
    QJsonArray jsbuffer;
    for(auto i=0;i<number_of_samples;i++){
        jsbuffer.append(QJsonValue(buffer[i]));
    }
    obj.insert("buffer",jsbuffer);
    obj.insert("channel_enable_mask",QJsonValue(channel_enable_mask));
    obj.insert("sample_rate",QJsonValue((int)sample_rate));
    obj.insert("no_channels",QJsonValue(no_channels));
    obj.insert("number_of_samples",QJsonValue((int)number_of_samples));

    QJsonDocument doc(obj);

    savedsession.write(doc.toJson(QJsonDocument::Compact));
    savedsession.close();
}

void adiscope::PatternGenerator::update_ui()
{
    ui->ChannelEnableMask->setText("0x"+QString::number(channel_enable_mask,16));
    ui->sampleRateCombo->setCurrentText(QString::number(sample_rate));
    ui->numberOfSamples->setText(QString::number(number_of_samples));
}

void adiscope::PatternGenerator::on_load_PB_clicked()
{
    QFileDialog qfd;
    qfd.setDefaultSuffix("json");

    QString filename = qfd.getOpenFileName(this,tr("Load buffer"),".json",tr("Session buffer file (*.json)"));
    if(filename == 0)
        return;
    QFile file( filename );

    file.open( QIODevice::ReadOnly);
    QJsonArray jsbuffer;

    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QJsonDocument d = QJsonDocument::fromJson(file.readAll());
    file.close();
    QJsonObject obj(d.object());

    channel_enable_mask = obj["channel_enable_mask"].toInt();
    sample_rate = obj["sample_rate"].toInt();
    no_channels = obj["no_channels"].toInt();
    number_of_samples = obj["number_of_samples"].toInt();
    delete buffer;
    buffer = new short[number_of_samples];

    buffersize = 2 * number_of_samples;
    for(auto i=0;i<number_of_samples;i++){
        buffer[i] = obj["buffer"].toArray()[i].toInt();
    }
    update_ui();
    createBinaryBuffer();
    dataChanged();
    main_win->action_view_zoom_fit()->trigger();

}

void adiscope::PatternGenerator::createRightPatternWidget(PatternUI* pattern_ui)
{

    if(currentUI!=nullptr)
    {
       // currentUI->get_pattern()->deinit();
        currentUI->setVisible(false);
        currentUI->destroy_ui();
        delete currentUI;
        currentUI = nullptr;
    }
    currentUI = pattern_ui;

    currentUI->build_ui(ui->rightWidgetPage2);
    currentUI->get_pattern()->init();
    currentUI->post_load_ui();
    currentUI->setVisible(true);
    selected_channel_group = static_cast<PatternGeneratorChannelGroupUI*>(QObject::sender())->getChannelGroup();
    connect(currentUI,SIGNAL(generate_buffer()),this,SLOT(on_generatePattern_clicked()));
}

void adiscope::PatternGenerator::onChannelEnabledChanged()
{
    uint16_t mask = chm.get_enabled_mask();
    ui->ChannelEnableMask->setText("0x"+QString().number(mask,16));

}


void adiscope::PatternGenerator::onChannelSelectedChanged()
{
    uint16_t mask = chm.get_selected_mask();
    ui->ChannelsToGenerate->setText("0x"+QString().number(mask,16));

}


int PatternFactory::static_ui_limit = 0;
QStringList PatternFactory::ui_list = {};
QStringList PatternFactory::description_list = {};
QJsonObject PatternFactory::patterns = {};


void PatternFactory::init()
{
    QJsonObject pattern_object;

    ui_list.clear();

    ui_list.append(ClockPatternName);
    description_list.append(ClockPatternDescription);
    ui_list.append(RandomPatternName);
    description_list.append(RandomPatternDescription);

    /*
    ui_list.append(ConstantPatternName);
    description_list.append(ConstantPatternDescription);
    ui_list.append(NumberPatternName);
    description_list.append(NumberPatternDescription);

    ui_list.append(PulsePatternName);
    description_list.append(PulsePatternDescription);
    ui_list.append(BinaryCounterPatternName);
    description_list.append(BinaryCounterPatternDescription);
    ui_list.append(GrayCounterPatternName);
    description_list.append(GrayCounterPatternDescription);
    ui_list.append(JohnsonCounterPatternName);
    description_list.append(JohnsonCounterPatternDescription);
    ui_list.append(WalkingCounterPatternName);
    description_list.append(WalkingCounterPatternDescription);
*/
    static_ui_limit = ui_list.count();

    /*QString searchPattern = "generator.json";
    QDirIterator it("patterngenerator", QStringList() << searchPattern, QDir::Files, QDirIterator::Subdirectories);
    int i = 0;
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
            ui_list.append(obj["name"].toString());
            description_list.append(obj["description"].toString());
            pattern_object.insert(QString::number(i),QJsonValue(obj));
            i++;
        }

    }
    patterns = pattern_object;*/
    qDebug()<<patterns;
}


Pattern* PatternFactory::create(int index)
{
    switch(index){
    case 0: return new ClockPattern();
    case 1: return new RandomPattern();
   /* case 0: return new ConstantPattern();
    case 1: return new NumberPattern();

    case 3: return new PulsePattern();

    case 5: return new BinaryCounterPattern();
    case 6: return new GrayCounterPattern();
    case 7: return new JohnsonCounterPattern();
    case 8: return new WalkingPattern();
    default:
        if(index>=static_ui_limit)
        {
            return new JSPattern(patterns[QString::number(static_ui_limit-index)].toObject());
        }
        else
        {
            return nullptr;
        }
        */
    }
}

PatternUI* PatternFactory::create_ui(Pattern* pattern, int index, QWidget *parent)
{
    switch(index){
    case 0: return new ClockPatternUI(dynamic_cast<ClockPattern*>(pattern),parent);
    case 1: return new RandomPatternUI(dynamic_cast<RandomPattern*>(pattern),parent);
        /*
    case 1: return new NumberPatternUI(parent);
    case 0: return new ConstantPatternUI(parent);
    case 3: return new PulsePatternUI(parent);

    case 5: return new BinaryCounterPatternUI(parent);
    case 6: return new GrayCounterPatternUI(parent);
    case 7: return new JohnsonCounterPatternUI(parent);
    case 8: return new WalkingPatternUI(parent);
    default:
        if(index>=static_ui_limit)
        {
            return new JSPatternUI(patterns[QString::number(static_ui_limit-index)].toObject(), parent);
        }
        else
        {
            return nullptr;
        }
        */
    }
}



QStringList PatternFactory::get_ui_list()
{
    return ui_list;
}

QStringList PatternFactory::get_description_list()
{
    return description_list;
}



} /* namespace adiscope */

