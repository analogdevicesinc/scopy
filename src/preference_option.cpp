#include "preference_option.h"
#include "ui_preference_option.h"

using namespace adiscope;
PreferenceOption::PreferenceOption(QWidget *parent, QString description,
				   QSettings *settings) :
	QWidget(parent),
	p_ui(new Ui::PreferenceOption),
	p_description(description),
	p_api(new PreferenceOption_API(this)),
	p_settings(settings)
{
	p_ui->setupUi(this);
	p_ui->label->setText(p_description);
	connect(p_ui->checkBox, &QCheckBox::stateChanged, [=](int state){
		Q_EMIT enabled(state == Qt::Checked);
	});
	p_api->setObjectName(p_description);
}

PreferenceOption::~PreferenceOption()
{
	if (p_settings)
		// when the options belong to a tool, we need to save in the .bak file,
		// if we save in the .ini file the file will be overwritten when scopy
		// is closed and the preferences won't be saved correctly
		p_api->save(*p_settings);
	else
		p_api->save();

	delete p_api;
	delete p_ui;
}

const QString PreferenceOption::getDescription() const
{
	return p_description;
}

const bool PreferenceOption::isChecked() const
{
	return p_ui->checkBox->isChecked();
}

void PreferenceOption::setChecked(bool checked)
{
	p_ui->checkBox->setChecked(checked);
}

void PreferenceOption::load()
{
	p_api->load();
}

const bool PreferenceOption_API::isChecked() const
{
	return po->isChecked();
}

void PreferenceOption_API::setChecked(bool checked)
{
	po->setChecked(checked);
}
