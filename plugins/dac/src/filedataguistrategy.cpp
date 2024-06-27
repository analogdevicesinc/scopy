#include "filedataguistrategy.h"

#include <menusectionwidget.h>
#include <menucollapsesection.h>
#include <titlespinbox.h>
#include <menuonoffswitch.h>
#include <stylehelper.h>

#include "dac_logging_categories.h"

using namespace scopy;
using namespace scopy::dac;
FileDataGuiStrategy::FileDataGuiStrategy(QWidget *parent)
	: QObject(parent)
{
	m_ui = new QWidget(parent);

	m_ui->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	auto lay = new QHBoxLayout(m_ui);
	lay->setMargin(0);
	lay->setSpacing(0);
	m_ui->setLayout(lay);

	MenuSectionWidget *guiContainer = new MenuSectionWidget(m_ui);
	MenuCollapseSection *guiSection =
		new MenuCollapseSection("DATA CONFIGURATION", MenuCollapseSection::MHCW_ARROW, guiContainer);
	guiSection->contentLayout()->setSpacing(10);

	m_scaled = new MenuOnOffSwitch("Scaled", guiSection);
	m_scaleSpin = new TitleSpinBox("Scale (DBFS)", true, guiSection);
	m_scaleSpin->setMax(0.0);
	m_scaleSpin->setMin(-91.0);
	m_scaleSpin->setStep(1.0); // when value invalid display -Inf db
	StyleHelper::BackgroundWidget(m_scaleSpin);

	connect(m_scaled->onOffswitch(), &QAbstractButton::toggled, this, [this](bool b) {
		m_recipe.scaled = b;
		Q_EMIT recipeUpdated(m_recipe);
	});
	connect(m_scaleSpin->getLineEdit(), &QLineEdit::textChanged, this, [this](QString text) {
		bool ok = true;
		double val = text.toDouble(&ok);
		if(ok) {
			m_recipe.scale = val;
			Q_EMIT recipeUpdated(m_recipe);
		}
	});

	guiSection->contentLayout()->addWidget(m_scaleSpin);
	guiSection->contentLayout()->addWidget(m_scaled);
	guiContainer->contentLayout()->addWidget(guiSection);
	lay->addWidget(guiContainer);
}

QWidget *FileDataGuiStrategy::ui() { return m_ui; }

void FileDataGuiStrategy::init()
{
	m_scaleSpin->setValue(0.0);
	m_scaled->onOffswitch()->setChecked(true);
}
