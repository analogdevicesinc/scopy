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

#include "scopy-gui_export.h"
#include "tutorialoverlay.h"

#include <QMap>

namespace scopy::gui {
class SCOPY_GUI_EXPORT TutorialBuilder : public scopy::gui::TutorialOverlay
{
	Q_OBJECT
public:
	/**
	 * @brief This acts as a wrapper to the scopy::gui::TutorialOverlay class that is meant to simplify adding
	 * chapters in order.
	 * @param topWidget The topmost widget that contains all the children widgets that will be part of the tutorial.
	 * @param jsonFile A json file that describes each chapter and attributes related to it (e.g. description,
	 * order).
	 * @param jsonEntry The entry in the jsonFile that represents the current tutorial.
	 * @param parent The widget that will receive the tinted overlay meant to highlight tutorial subjects.
	 * @inherit scopy::gui::TutorialOverlay
	 * */
	explicit TutorialBuilder(QWidget *topWidget, const QString &jsonFile, QString jsonEntry,
				 QWidget *parent = nullptr);
	explicit TutorialBuilder(QWidget *parent = nullptr);

	QString getJsonFileName() const;
	void setJsonFileName(const QString &jsonFile);

	const QString &getJsonEntry() const;
	void setJsonEntry(const QString &jsonEntry);

	QWidget *getTopWidget() const;
	void setTopWidget(QWidget *topWidget);

	/**
	 * @brief Collects and adds all qt objects that are part of the tutorial and are contained in the m_topWidget
	 * widget (given as constructor parameter) recursively in the chapter list. The widgets are considered
	 * part of the tutorial if their object name is in the m_jsonFile file under the m_jsonEntry section. After
	 * they are collected, they are sorted based on the number read from the json file and added as chapters. The
	 * chapter description will be matched and added from the same json file.
	 * @throws std::runtime_error Thrown then no topWidget was provided.
	 * @return The number of tutorial objects collected.
	 * */
	uint16_t collectChapters();

	/**
	 * @brief Opens the m_jsonFile and searches for the m_jsonEntry array object. After that it collects the
	 * specific attributes and populates m_chapters with the entries.
	 * @throws std::runtime_error Thrown when there are issues opening or parsing the json file.
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
	QWidget *m_topWidget;

	struct ChapterInstructions
	{
		// map representing the widget property string related to the tutorial and the related widget
		QMap<QString, QWidget *> widgets;

		// this is a name of the widget from the map declared above, the description of the chapter will
		// be anchored to this widget
		QString mainWidget;
		QString description;
		int x_offset;
		int y_offset;
		HoverPosition anchor;
		HoverPosition content;
	};

	QMap<uint16_t, ChapterInstructions *> m_chapters;

	/**
	 * @brief Searches for the ChapterInstructions entries that have the name field matching the name parameter
	 * @param name QString with the name of the ChapterInstructions that will be searched
	 * @throws std::runtime_error Thrown when the name is not found in the m_chapters QMap
	 * @return The list of pointers to the ChapterInstructions that were found with this name
	 * */
	QList<TutorialBuilder::ChapterInstructions *> getChapterInstructionFromName(const QString &name);

	/**
	 * @brief Converts a QString to the corresponding HoverPosition value (e.g. "HP_TOPLEFT" ->
	 * scopy::HoverPosition::HP_TOPLEFT)
	 * @param position QString that matches an entry of HoverPosition
	 * @return HoverPosition entry, if there is no match, the default value of CENTER will be returned
	 * */
	static HoverPosition convertStringToHoverPosition(const QString &position) noexcept;
};
} // namespace scopy::gui

#endif // SCOPY_TUTORIALBUILDER_H
