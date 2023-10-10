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

#include "tutorialbuilder.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QLoggingCategory>

#include <exception>
#include <utility>

using namespace scopy::gui;

Q_LOGGING_CATEGORY(CAT_TUTORIALBUILDER, "TutorialBuilder")

TutorialBuilder::TutorialBuilder(QWidget *topWidget, const QString &jsonFile, QString jsonEntry, QWidget *parent)
	: scopy::gui::TutorialOverlay(parent)
	, m_topWidget(topWidget)
	, m_jsonFile(jsonFile)
	, m_jsonEntry(std::move(jsonEntry))
{
	if(!m_jsonFile.exists()) {
		qWarning(CAT_TUTORIALBUILDER) << "The file" << jsonFile << "does not exist.";
	}
	qDebug(CAT_TUTORIALBUILDER) << "Tutorial Builder constructed for json entry" << m_jsonEntry;
}

TutorialBuilder::TutorialBuilder(QWidget *parent)
	: scopy::gui::TutorialOverlay(parent)
	, m_topWidget(nullptr)
	, m_jsonFile(nullptr)
{
	qDebug(CAT_TUTORIALBUILDER) << "Tutorial Builder constructed";
}

QString TutorialBuilder::getJsonFileName() const { return m_jsonFile.fileName(); }

void TutorialBuilder::setJsonFileName(const QString &jsonFile)
{
	m_jsonFile.setFileName(jsonFile);
	if(!m_jsonFile.exists()) {
		qWarning(CAT_TUTORIALBUILDER) << "The file" << jsonFile << "does not exist.";
	}
}

const QString &TutorialBuilder::getJsonEntry() const { return m_jsonEntry; }

void TutorialBuilder::setJsonEntry(const QString &jsonEntry) { m_jsonEntry = jsonEntry; }

QWidget *TutorialBuilder::getTopWidget() const { return m_topWidget; }

void TutorialBuilder::setTopWidget(QWidget *topWidget) { m_topWidget = topWidget; }

void TutorialBuilder::start()
{
	try {
		this->readTutorialRequirements();
		uint16_t chaptersCollected = this->collectChapters();
		qInfo(CAT_TUTORIALBUILDER) << "Chapters collected" << chaptersCollected;

		TutorialOverlay::start();
	} catch(std::runtime_error &error) {
		qCritical(CAT_TUTORIALBUILDER) << "Cannot start tutorial:" << error.what();
	}
}

uint16_t TutorialBuilder::collectChapters()
{
	if(!m_topWidget) {
		throw std::runtime_error("No top widget was provided.");
	}

	uint16_t chaptersCollected = 0;	     // number of objects that are part of the tutorial
	uint16_t totalChaptersCollected = 0; // total number of objects collected, used only as debug statistic

	// collect all children objects from m_topWidget recursively
	QObjectList childrenList =
		m_topWidget->findChildren<QObject *>(QRegularExpression(".*"), Qt::FindChildrenRecursively);

	for(auto child : childrenList) {
		QString currentObjectName = child->property("tutorial_name").toString();

		// if the object has no property named "tutorial_name", then it is not part of the tutorial
		if(!currentObjectName.isEmpty()) {
			try {
				// get the chapters that use the widget with the property name currentObjectName
				QList<TutorialBuilder::ChapterInstructions *> chapters =
					this->getChapterInstructionFromName(currentObjectName);
				for(auto chapter : chapters) {
					chapter->widgets[currentObjectName] = qobject_cast<QWidget *>(child);
				}
				++chaptersCollected;
			} catch(std::runtime_error &error) {
				qCritical(CAT_TUTORIALBUILDER) << error.what();
			}
		}
		++totalChaptersCollected;
	}
	qDebug(CAT_TUTORIALBUILDER) << "Collected a total of" << totalChaptersCollected << "in the" << m_jsonEntry
				    << "entry.";

	// add the now sorted chapters into the tutorial overlay
	for(const ChapterInstructions *chapter : qAsConst(m_chapters)) {
		// if there is no mainWidget specified, the first widget from the list will be set to be the mainWidget
		this->addChapter(chapter->widgets.values(), chapter->description,
				 (chapter->mainWidget.isNull()) ? chapter->widgets.first()
								: chapter->widgets[chapter->mainWidget],
				 chapter->x_offset, chapter->y_offset, chapter->anchor, chapter->content);
	}

	return chaptersCollected;
}

void TutorialBuilder::readTutorialRequirements()
{
	// open and read the file
	if(m_jsonFile.fileName().isEmpty()) {
		throw std::runtime_error("No file name was provided.");
	}

	m_jsonFile.open(QIODevice::ReadOnly | QIODevice::Text);
	if(!m_jsonFile.isOpen()) {
		throw std::runtime_error("File could not be opened (read): " + m_jsonFile.fileName().toStdString());
	} else {
		qDebug(CAT_TUTORIALBUILDER) << "File opened (read): " << m_jsonFile.fileName();
	}
	QString contents = m_jsonFile.readAll();
	m_jsonFile.close();

	// try to parse the contents as a json
	QJsonParseError parse_error{};
	QJsonDocument document = QJsonDocument::fromJson(contents.toUtf8(), &parse_error);
	if(document.isNull()) {
		throw std::runtime_error("Invalid json: " + parse_error.errorString().toStdString());
	}

	// extract the values related to the m_jsonEntry mode
	QJsonObject document_object = document.object();
	QJsonValue value = document_object.value(m_jsonEntry);
	if(value == QJsonValue::Undefined) {
		throw std::runtime_error("Invalid json: Could not extract value " + m_jsonEntry.toStdString());
	}

	// turn the json values into an array of ChapterInstructions and save the data
	// each element from the json array must have the following structure:
	// {
	//	"index": int
	//	"name": [string]
	//	"description": string
	//	"main_widget": string
	//	"x_offset": int, optional
	//	"y_offset": int, optional
	//	"anchor": string, HoverPosition, optional
	//	"content": string, HoverPosition, optional
	// }
	QJsonArray chapters_array = value.toArray();
	for(QJsonValueRef item : chapters_array) {
		QJsonObject item_object = item.toObject();
		int index = item_object.value("index").toInt(-1);
		QStringList names;
		QJsonArray jsonNameArray = item_object.value("names").toArray();
		for(auto name : jsonNameArray) {
			names.push_back(name.toString());
		}
		QString description = item_object.value("description").toString();
		QString mainWidget = item_object.value("main_widget").toString();
		int x_offset = item_object.value("x_offset").toInt(0);
		int y_offset = item_object.value("y_offset").toInt(0);
		QString anchorString = item_object.value("anchor").toString();
		QString contentString = item_object.value("content").toString();
		HoverPosition anchor = convertStringToHoverPosition(anchorString);
		HoverPosition content = convertStringToHoverPosition(contentString);

		auto *chapterInstructions = new ChapterInstructions;
		for(const auto &name : qAsConst(names)) {
			chapterInstructions->widgets.insert(name, nullptr); // the widgets will be set later
		}
		chapterInstructions->mainWidget = mainWidget;
		chapterInstructions->description = description;
		chapterInstructions->x_offset = x_offset;
		chapterInstructions->y_offset = y_offset;
		chapterInstructions->anchor = anchor;
		chapterInstructions->content = content;
		if(m_chapters.contains(index)) {
			qWarning(CAT_TUTORIALBUILDER) << "There is already a chapter with the key" << index;
		} else {
			m_chapters.insert(index, chapterInstructions);
		}
	}
}

QList<TutorialBuilder::ChapterInstructions *> TutorialBuilder::getChapterInstructionFromName(const QString &name)
{
	QList<TutorialBuilder::ChapterInstructions *> result;
	for(auto &chapter : m_chapters) {
		const QList<QString> keys = chapter->widgets.keys();
		for(const auto &itemName : keys) {
			if(itemName == name) {
				result.push_back(chapter);
			}
		}
	}

	if(result.isEmpty()) {
		throw std::runtime_error("No ChapterInstructions that need this name were found.");
	} else {
		return result;
	}
}

scopy::HoverPosition TutorialBuilder::convertStringToHoverPosition(const QString &position) noexcept
{
	if(position == "HP_LEFT") {
		return scopy::HoverPosition::HP_LEFT;
	} else if(position == "HP_TOPLEFT") {
		return scopy::HoverPosition::HP_TOPLEFT;
	} else if(position == "HP_TOP") {
		return scopy::HoverPosition::HP_TOP;
	} else if(position == "HP_TOPRIGHT") {
		return scopy::HoverPosition::HP_TOPRIGHT;
	} else if(position == "HP_RIGHT") {
		return scopy::HoverPosition::HP_RIGHT;
	} else if(position == "HP_BOTTOMRIGHT") {
		return scopy::HoverPosition::HP_BOTTOMRIGHT;
	} else if(position == "HP_BOTTOM") {
		return scopy::HoverPosition::HP_BOTTOM;
	} else if(position == "HP_BOTTOMLEFT") {
		return scopy::HoverPosition::HP_BOTTOMLEFT;
	} else { // position == "HP_CENTER"
		return scopy::HoverPosition::HP_CENTER;
	}
}

#include "moc_tutorialbuilder.cpp"
