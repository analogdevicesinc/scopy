/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */


#ifndef SCOPY_TUTORIALBUILDER_H
#define SCOPY_TUTORIALBUILDER_H

#include <QMap>

#include "scopy-gui_export.h"
#include "tutorialoverlay.h"

namespace scopy::gui {
class SCOPY_GUI_EXPORT TutorialBuilder : public scopy::gui::TutorialOverlay {
	Q_OBJECT
public:
	explicit TutorialBuilder(QWidget* topWidget, const QString& jsonFile, QString jsonEntry, QWidget* parent = nullptr);
	explicit TutorialBuilder(QWidget* parent = nullptr);

	QString getJsonFileName() const;
	void setJsonFileName(const QString &jsonFile);

	const QString &getJsonEntry() const;
	void setJsonEntry(const QString &jsonEntry);

	QWidget *getTopWidget() const;
	void setTopWidget(QWidget *topWidget);

	/**
	 * @brief Collects and adds all qt objects that are part of the tutorial and are contained in the m_topWidget
	 * widget (given as constructor parameter) widget recursively in the chapter list. The widgets are considered
	 * part of the tutorial if their object name is in the m_jsonFile file under the m_jsonEntry section. After
	 * they are collected, they are sorted based on the number read from the json file and added as chapters. The
	 * chapter description will be matched and added from the same json file.
	 * @return The number of tutorial objects collected.
	 * */
	uint16_t collectChapters();

	/**
	 * @brief Opens the m_jsonFile and searches for the m_jsonEntry array object. After that it collects the index,
	 * name, description and populates m_chapters with the specified attributes.
	 * */
	void readTutorialRequirements();

public Q_SLOTS:
	/**
 	* @brief Calls collectChapters() and readTutorialRequirements() before calling the start() method from the
 	* base class.
 	* */
	void start() override;

private:
	QFile m_jsonFile;
	QString m_jsonEntry;
	QWidget* m_topWidget;

	struct ChapterInstructions {
		QString name;
		QString description;
		QWidget* widget;
	};

	QMap<uint16_t, ChapterInstructions> m_chapters;
};
}

#endif //SCOPY_TUTORIALBUILDER_H
