/*
 * Copyright (c) 2026 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "codegeneratorpanel.h"

#include <QAbstractButton>
#include <QClipboard>
#include <QComboBox>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>

#include <component/codeemitterregistry.h>
#include <menucombo.h>
#include <menuonoffswitch.h>
#include <pluginbase/preferences.h>
#include <style.h>

using namespace scopy;
using namespace scopy::dcdebugger;

CodeGeneratorPanel::CodeGeneratorPanel(QWidget *parent)
	: QFrame(parent)
	, m_registry(component::CodeEmitterRegistry::GetInstance())
{
	// The lighter shade of the log pane (like its filter strip); the preview is the darker one.
	Style::setStyle(this, style::properties::widget::basicComponent);
	auto *lay = new QVBoxLayout(this);
	lay->setContentsMargins(6, 6, 6, 6);
	lay->setSpacing(6);

	auto *title = new QLabel("Generate code", this);
	Style::setStyle(title, style::properties::label::menuMedium);
	m_scope = new QLabel(this);
	Style::setStyle(m_scope, style::properties::label::subtle);
	m_scope->setWordWrap(true);
	lay->addWidget(title);
	lay->addWidget(m_scope);

	m_language = new MenuCombo("Language", this);
	m_language->setToolTip("Languages with bindings for every library the selected calls use");
	lay->addWidget(m_language);

	m_loops = new MenuOnOffSwitch("Loops for repeats", this);
	m_loops->setToolTip("Repeated command sequences (e.g. refills) become a loop");
	m_loops->onOffswitch()->setChecked(true);
	auto *switches = new QWidget(this);
	auto *swLay = new QHBoxLayout(switches);
	swLay->setContentsMargins(0, 0, 0, 0);
	swLay->setSpacing(12);
	swLay->addWidget(m_loops);
	connect(m_loops->onOffswitch(), &QAbstractButton::toggled, this, &CodeGeneratorPanel::regenerate);
	swLay->addStretch();
	lay->addWidget(switches);

	m_preview = new QPlainTextEdit(this);
	m_preview->setReadOnly(true);
	m_preview->setLineWrapMode(QPlainTextEdit::NoWrap);
	m_preview->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	m_preview->setPlaceholderText("Select rows of the log (none: every visible row) to generate a script");
	Style::setStyle(m_preview, style::properties::widget::textBrowser);
	lay->addWidget(m_preview, 1);

	m_copyBtn = new QPushButton("Copy", this);
	m_copyBtn->setToolTip("Copy the script to the clipboard");
	m_saveBtn = new QPushButton("Save...", this);
	m_saveBtn->setToolTip("Save the script to a file");
	auto *buttons = new QWidget(this);
	auto *btnLay = new QHBoxLayout(buttons);
	btnLay->setContentsMargins(0, 0, 0, 0);
	btnLay->addStretch();
	for(QPushButton *b : {m_copyBtn, m_saveBtn}) {
		Style::setStyle(b, style::properties::button::basicButton);
		b->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		btnLay->addWidget(b);
	}
	lay->addWidget(buttons);

	connect(m_language->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this,
		&CodeGeneratorPanel::regenerate);
	connect(m_copyBtn, &QPushButton::clicked, this, &CodeGeneratorPanel::copyToClipboard);
	connect(m_saveBtn, &QPushButton::clicked, this, &CodeGeneratorPanel::save);
	rebuildLanguages();
	regenerate();
}

CodeGeneratorPanel::~CodeGeneratorPanel() = default;

void CodeGeneratorPanel::setRegistry(const component::CodeEmitterRegistry *registry)
{
	m_registry = registry;
	rebuildLanguages();
	regenerate();
}

void CodeGeneratorPanel::setEntries(const QList<CommandLogEntry> &entries, const QString &scope)
{
	m_entries = entries;
	m_scope->setText(scope);
	rebuildLanguages();
	regenerate();
}

QStringList CodeGeneratorPanel::languages() const
{
	QStringList out;
	for(int i = 0; i < m_language->combo()->count(); ++i) {
		out << m_language->combo()->itemData(i).toString();
	}
	return out;
}

QString CodeGeneratorPanel::language() const { return m_language->combo()->currentData().toString(); }

void CodeGeneratorPanel::setLanguage(const QString &language)
{
	const int i = m_language->combo()->findData(language);
	if(i >= 0) {
		m_language->combo()->setCurrentIndex(i);
	}
}

EmitOptions CodeGeneratorPanel::options() const
{
	EmitOptions o;
	o.loops = m_loops->onOffswitch()->isChecked();
	return o;
}

void CodeGeneratorPanel::rebuildLanguages()
{
	const QStringList langs = m_registry
		? m_registry->languagesFor(component::CodeEmitterRegistry::librariesOf(m_entries))
		: QStringList();
	QComboBox *combo = m_language->combo();
	combo->setEnabled(!langs.isEmpty());
	if(langs == languages()) {
		return;
	}
	const QString current = language();
	{
		const QSignalBlocker block(combo);
		combo->clear();
		for(const QString &l : langs) {
			combo->addItem(codegen::languageName(l), l);
		}
		const int keep = combo->findData(current);
		combo->setCurrentIndex(keep >= 0 ? keep : (langs.isEmpty() ? -1 : 0));
	}
}

void CodeGeneratorPanel::regenerate()
{
	const QString lang = language();
	const QString script =
		(m_registry && !lang.isEmpty()) ? m_registry->exportScript(m_entries, lang, options()) : QString();
	m_copyBtn->setEnabled(!script.isEmpty());
	m_saveBtn->setEnabled(!script.isEmpty());
	if(script == m_script && m_preview->toPlainText() == script) {
		return;
	}
	m_script = script;
	// Keep the reader's place while the preview follows the log.
	const int scroll = m_preview->verticalScrollBar() ? m_preview->verticalScrollBar()->value() : 0;
	m_preview->setPlainText(script);
	if(m_preview->verticalScrollBar()) {
		m_preview->verticalScrollBar()->setValue(scroll);
	}
	Q_EMIT scriptChanged(script);
}

void CodeGeneratorPanel::copyToClipboard()
{
	if(!m_script.isEmpty()) {
		QGuiApplication::clipboard()->setText(m_script);
	}
}

bool CodeGeneratorPanel::saveTo(const QString &path) const
{
	if(m_script.isEmpty() || path.isEmpty()) {
		return false;
	}
	QFile file(path);
	if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		return false;
	}
	return file.write(m_script.toUtf8()) >= 0;
}

void CodeGeneratorPanel::save()
{
	if(m_script.isEmpty()) {
		return;
	}
	const QString lang = language();
	const QString ext = codegen::fileExtension(lang);
	const bool native = Preferences::get("general_use_native_dialogs").toBool();
	const QString name = QStringLiteral("dcdebugger-%1.%2")
				     .arg(QDateTime::currentDateTime().toString("yyyy_MM_dd-HH_mm_ss"), ext);
	const QString filter = QStringLiteral("%1 (*.%2)").arg(codegen::languageName(lang), ext);
	QString path = QFileDialog::getSaveFileName(this, "Save generated code", name, filter, nullptr,
						    native ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
	if(path.isEmpty()) {
		return;
	}
	if(!path.endsWith(QLatin1Char('.') + ext, Qt::CaseInsensitive)) {
		path += QLatin1Char('.') + ext;
	}
	if(!saveTo(path)) {
		m_scope->setText(QStringLiteral("Could not write %1").arg(path));
	}
}

#include "moc_codegeneratorpanel.cpp"
