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

#ifndef CODEGENERATORPANEL_H
#define CODEGENERATORPANEL_H

#include "scopy-dcdebugger_export.h"

#include <QFrame>
#include <QList>
#include <QString>
#include <QStringList>

#include <core/callrecord.h>
#include <core/codeemitter.h>

class QLabel;
class QPlainTextEdit;
class QPushButton;

namespace scopy {
class MenuCombo;
class MenuOnOffSwitch;
} // namespace scopy

namespace scopy::component {
class CodeEmitterRegistry;
}

namespace scopy::dcdebugger {

// Side panel of the command log: the script that repeats the logged calls of the given entries
// (the log selection). Offers only the languages with an emitter for every library the entries
// use, the generation options, a live read-only preview, Copy and Save. Backend agnostic: it only
// talks to the CodeEmitterRegistry and CommandLogEntry.
class SCOPY_DCDEBUGGER_EXPORT CodeGeneratorPanel : public QFrame
{
	Q_OBJECT
public:
	explicit CodeGeneratorPanel(QWidget *parent = nullptr);
	~CodeGeneratorPanel() override;

	// Emitters used (not owned); default CodeEmitterRegistry::GetInstance().
	void setRegistry(const component::CodeEmitterRegistry *registry);

	// What to generate; `scope` describes it ("3 selected rows", ...). Rebuilds the language list
	// (keeps the current language while offered) and the preview.
	void setEntries(const QList<CommandLogEntry> &entries, const QString &scope = {});
	const QList<CommandLogEntry> &entries() const { return m_entries; }

	// Language ids offered (codegen::C / Python), in combo order.
	QStringList languages() const;
	QString language() const;
	void setLanguage(const QString &language);
	EmitOptions options() const;
	// The previewed script; empty when there is nothing to generate.
	QString script() const { return m_script; }

	MenuCombo *languageCombo() const { return m_language; }
	MenuOnOffSwitch *loopsSwitch() const { return m_loops; }
	QPlainTextEdit *preview() const { return m_preview; }
	QPushButton *copyButton() const { return m_copyBtn; }
	QPushButton *saveButton() const { return m_saveBtn; }
	QLabel *scopeLabel() const { return m_scope; }
	// "Generate code"; hidden when a dock title bar already shows it.
	QLabel *titleLabel() const { return m_title; }
	void setTitleVisible(bool visible);

	// The script to the clipboard (nothing when empty).
	void copyToClipboard();
	// Writes the script to `path`; false on error or nothing to save.
	bool saveTo(const QString &path) const;
	// Save dialog (.c / .py by language).
	void save();

Q_SIGNALS:
	void scriptChanged(const QString &script);

private:
	void rebuildLanguages();
	void regenerate();

	const component::CodeEmitterRegistry *m_registry;
	QList<CommandLogEntry> m_entries;
	QString m_script;
	QLabel *m_title;
	QLabel *m_scope;
	MenuCombo *m_language;
	MenuOnOffSwitch *m_loops;
	QPlainTextEdit *m_preview;
	QPushButton *m_copyBtn;
	QPushButton *m_saveBtn;
};

} // namespace scopy::dcdebugger

#endif // CODEGENERATORPANEL_H
