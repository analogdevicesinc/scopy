#include "logging_categories.h"
#include "plugin/ichannelplugin.h"

#include "ref_channelplugin.cpp"

#include <QCoreApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <filemanager.h>
#include <newinstrument.hpp>
#include <gui/osc_import_settings.h>

using namespace adiscope;
using namespace gui;
class AddChannelPlugin : public IChannelPlugin
{
private:
	adiscope::ImportSettings *importSettings;
	NewInstrument *instrument;

	QVector<QVector<double>> import_data;
	QVector<QStringList> importedChannelDetails;
	QLineEdit *fileLineEdit;
	QPushButton *btnImport;
	QPushButton *btnBrowseFile;

public:
	AddChannelPlugin(QWidget *parent, ToolView* toolView, ChannelManager *chManager, bool dockable) : IChannelPlugin(parent, toolView, chManager, dockable)
	{
		init();
	}

	void init() override
	{
		qDebug(CAT_ADDCHANNEL_PLUGIN) << "init()";

		instrument = dynamic_cast<NewInstrument *>(parent);

		menu = new GenericMenu(parent);
		menu->initInteractiveMenu();
		menu->setMenuHeader("Add CH", new QColor('gray'), true);

		initMenu();
		chManager->insertAddBtn(menu, dockable);
	}

	void initMenu()
	{
		auto addReference = new QWidget();
		addReference->setObjectName(QString::fromUtf8("addReference"));
		auto verticalLayout_131 = new QVBoxLayout(addReference);
		verticalLayout_131->setObjectName(QString::fromUtf8("verticalLayout_131"));
		verticalLayout_131->setContentsMargins(18, 20, 18, 0);
		auto verticalLayout_14 = new QVBoxLayout();
		verticalLayout_14->setObjectName(QString::fromUtf8("verticalLayout_14"));
		auto label = new QLabel(addReference);
		label->setObjectName(QString::fromUtf8("label"));
		label->setProperty("general_settings_label", QVariant(true));

		verticalLayout_14->addWidget(label);

		auto verticalSpacer_13 = new QSpacerItem(20, 8, QSizePolicy::Minimum, QSizePolicy::Fixed);

		verticalLayout_14->addItem(verticalSpacer_13);

		auto line_5 = new QFrame(addReference);
		line_5->setObjectName(QString::fromUtf8("line_5"));
		line_5->setMaximumSize(QSize(16777215, 1));
		line_5->setFrameShadow(QFrame::Plain);
		line_5->setFrameShape(QFrame::HLine);
		line_5->setProperty("blue_line", QVariant(true));

		verticalLayout_14->addWidget(line_5);

		auto verticalSpacer_14 = new QSpacerItem(20, 14, QSizePolicy::Minimum, QSizePolicy::Fixed);

		verticalLayout_14->addItem(verticalSpacer_14);

		fileLineEdit = new QLineEdit(addReference);
		fileLineEdit->setObjectName(QString::fromUtf8("fileLineEdit"));
		fileLineEdit->setEnabled(false);
		fileLineEdit->setReadOnly(true);

		verticalLayout_14->addWidget(fileLineEdit);

		btnBrowseFile = new QPushButton(addReference);
		btnBrowseFile->setObjectName(QString::fromUtf8("btnBrowseFile"));
		btnBrowseFile->setStyleSheet(QString::fromUtf8("QPushButton {\n"
	"  min-height: 30px;\n"
	"  max-height: 30px;\n"
	"\n"
	"  border: 0px;\n"
	"}"));
		btnBrowseFile->setProperty("blue_button", QVariant(true));

		verticalLayout_14->addWidget(btnBrowseFile);

		importSettings = new adiscope::ImportSettings(addReference);
		importSettings->setObjectName(QString::fromUtf8("importSettings"));
		importSettings->setEnabled(false);

		verticalLayout_14->addWidget(importSettings);

		btnImport = new QPushButton(addReference);
		btnImport->setObjectName(QString::fromUtf8("btnImport"));
		btnImport->setEnabled(false);
		btnImport->setStyleSheet(QString::fromUtf8("QPushButton {\n"
	"  min-height: 30px;\n"
	"  max-height: 30px;\n"
	"\n"
	"  border: 0px;\n"
	"}"));
		btnImport->setProperty("blue_button", QVariant(true));

		verticalLayout_14->addWidget(btnImport);

		auto verticalSpacer_12 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

		verticalLayout_14->addItem(verticalSpacer_12);


		verticalLayout_131->addLayout(verticalLayout_14);

		label->setText(QCoreApplication::translate("New Instrument", "Import Reference Channels", nullptr));
		fileLineEdit->setText(QCoreApplication::translate("New Instrument", "No file selected!", nullptr));
		btnBrowseFile->setText(QCoreApplication::translate("New Instrument", "Browse", nullptr));
		btnImport->setText(QCoreApplication::translate("New Instrument", "Import", nullptr));

		menu->setMenuWidget(addReference);

		connect(btnImport, &QPushButton::clicked, this, &AddChannelPlugin::importBtnClicked);
		connect(btnBrowseFile, &QPushButton::clicked, this, &AddChannelPlugin::browseBtnClicked);
	}

	void add_ref_waveform(unsigned int chIdx)
	{
		QVector<double> xData;
		QVector<double> yData;

		for (int i = 0; i < import_data.size(); ++i) {
			xData.push_back(import_data[i][0]);
			yData.push_back(import_data[i][chIdx + 1]);
		}

		add_ref_waveform(xData, yData);
	}

	void add_ref_waveform(QVector<double> xData, QVector<double> yData)
	{
//		if (nb_ref_channels == MAX_REF_CHANNELS) {
//			return;
//		}

//		unsigned int curve_id = m_adc_nb_channels + nb_ref_channels;
		instrument->addPlugin(new RefChannelPlugin(parent, toolView, chManager, xData, yData, dockable));


//		auto channelWidget = new ChannelWidget(curve_id, true, false, fft_plot->getLineColor(curve_id));
//		channelWidget->setShortName(qname);
//		channelWidget->nameButton()->setText(channelWidget->shortName());
//		channelWidget->setReferenceChannel(true);
//		channelWidget->deleteButton()->setProperty(
//			"curve_name", QVariant(qname));

//		settings_group->addButton(channelWidget->menuButton());
//		channels_group->addButton(channelWidget->nameButton());
//		referenceChannels.push_back(channelWidget);

//		connect(channelWidget, &ChannelWidget::menuToggled,
//			this, &SpectrumAnalyzer::onChannelSettingsToggled);
//		connect(channelWidget, &ChannelWidget::selected,
//			this, &SpectrumAnalyzer::onChannelSelected);
//		connect(channelWidget, &ChannelWidget::deleteClicked,
//			this, &SpectrumAnalyzer::onReferenceChannelDeleted);
//		connect(channelWidget, &ChannelWidget::enabled,
//			this, &SpectrumAnalyzer::onChannelEnabled);

//		ui->channelsList->addWidget(channelWidget);

		/* Increase ref channels count */
//		nb_ref_channels++;

//		if (!ui->btnMarkers->isEnabled()) {
//			QSignalBlocker(ui->btnMarkers);
//			ui->btnMarkers->setEnabled(true);
//		}

//		if (nb_ref_channels == MAX_REF_CHANNELS) {
//			ui->btnAddRef->hide();
//		}
	}

public	Q_SLOTS:
	void importBtnClicked()
	{
		QMap<int, bool> import_map = importSettings->getExportConfig();

		auto keys = import_map.keys();
		for (int key : qAsConst(keys)) {
			if (import_map[key]) {
				add_ref_waveform(key);
			}
		}
	}


	void browseBtnClicked()
	{
//		QTest::mouseClick(btnBrowseFile, Qt::LeftButton, 0, QPoint(), 5000);
//		QTest::mouseClick(btnBrowseFile, Qt::LeftButton);
//		QTest::keyClick(btnBrowseFile,Qt::Key_Shift);

//		return;
		QString fileName = QFileDialog::getOpenFileName(this->parent,
		    tr("Export"), "", tr("Comma-separated values files (*.csv);;"
					       "Tab-delimited values files (*.txt)"),
		    nullptr, QFileDialog::DontUseNativeDialog);

		FileManager fm("NewInstrument");

		importSettings->clear();
		import_data.clear();

		try {
			fm.open(fileName, FileManager::IMPORT);

			for (int i = 0; i < fm.getNrOfChannels(); ++i) {
				/* Amplitude CHX UNIT => mid(10, 3) strip CHX from column name */
				QString chn_name = fm.getColumnName(i).mid(10, 3);
				chn_name = (chn_name == "") ? "CH" + i : chn_name;
				importSettings->addChannel(i, chn_name);
			}

			QVector<QVector<double>> data = fm.read();
			for (int i = 0; i < data.size(); ++i) {
				import_data.push_back(data[i]);
			}

			QStringList channelDetails = fm.getAdditionalInformation();
			for (int i = 0; i < channelDetails.size() / 3; ++i) {
				QStringList currentChannelDetails;
				for (int j = 0; j < 3; ++j) {
					currentChannelDetails.push_back(channelDetails[i * 3 + j]);
				}
				importedChannelDetails.push_back(currentChannelDetails);
			}

			fileLineEdit->setText(fileName);
			fileLineEdit->setToolTip(fileName);

			btnImport->setEnabled(true);
			importSettings->setEnabled(true);


		} catch (FileManagerException &e) {
			fileLineEdit->setText(e.what());
			fileLineEdit->setToolTip("");
			btnImport->setDisabled(true);
			importSettings->setDisabled(true);
		}
	}
};
