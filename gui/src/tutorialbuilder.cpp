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
#include <utility>
#include <exception>
#include <QLoggingCategory>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>

using namespace scopy::gui;

Q_LOGGING_CATEGORY(CAT_TUTORIALBUILDER, "TutorialBuilder")

TutorialBuilder::TutorialBuilder(QWidget *topWidget, const QString& jsonFile, QString jsonEntry, QWidget *parent) :
	scopy::gui::TutorialOverlay(parent),
	m_topWidget(topWidget),
	m_jsonFile(jsonFile),
	m_jsonEntry(std::move(jsonEntry))
{
	if (!m_jsonFile.exists()) {
		qWarning(CAT_TUTORIALBUILDER) << "The file" << jsonFile << "does not exist.";
	}
	qDebug(CAT_TUTORIALBUILDER) << "Tutorial Builder constructed for json entry" << m_jsonEntry;
}

TutorialBuilder::TutorialBuilder(QWidget *parent) :
	scopy::gui::TutorialOverlay(parent),
	m_topWidget(nullptr),
	m_jsonFile(nullptr)
{
	qDebug(CAT_TUTORIALBUILDER) << "Tutorial Builder constructed";
}

QString TutorialBuilder::getJsonFileName() const {
	return m_jsonFile.fileName();
}

void TutorialBuilder::setJsonFileName(const QString &jsonFile) {
	m_jsonFile.setFileName(jsonFile);
	if (!m_jsonFile.exists()) {
		qWarning(CAT_TUTORIALBUILDER) << "The file" << jsonFile << "does not exist.";
	}
}

const QString &TutorialBuilder::getJsonEntry() const {
	return m_jsonEntry;
}

void TutorialBuilder::setJsonEntry(const QString &jsonEntry) {
	m_jsonEntry = jsonEntry;
}

QWidget *TutorialBuilder::getTopWidget() const {
	return m_topWidget;
}

void TutorialBuilder::setTopWidget(QWidget *topWidget) {
	m_topWidget = topWidget;
}

void TutorialBuilder::start() {
	this->readTutorialRequirements();
	uint16_t chaptersCollected = this->collectChapters();
	qInfo(CAT_TUTORIALBUILDER) << "Chapters collected" << chaptersCollected;

	TutorialOverlay::start();
}

uint16_t TutorialBuilder::collectChapters() {
	if (!m_topWidget) {
		qCritical(CAT_TUTORIALBUILDER) << "No top widget was provided. Aborting tutorial.";
		return 0;
	}

	uint16_t chaptersCollected = 0; // number of objects that are part of the tutorial
	uint16_t totalChaptersCollected = 0; // total number of objects collected, used only as debug statistic

	// collect all children objects from m_topWidget recursively
	QObjectList childrenList = m_topWidget->findChildren<QObject*>(QRegularExpression(".*"), Qt::FindChildrenRecursively);

	for (auto child: childrenList) {
		QString currentObjectName = child->property("tutorial_name").toString();

		// if the object has no property named "tutorial_name", then it is not part of the tutorial
		if (!currentObjectName.isEmpty()) {
			try {
				QList<std::reference_wrapper<TutorialBuilder::ChapterInstructions>> chapters = this->getChapterInstructionFromName(currentObjectName);
				for (auto chapter: chapters) {
					chapter.get().widget = qobject_cast<QWidget*>(child);
				}
				++chaptersCollected;
			} catch (std::runtime_error &error) {
				qCritical(CAT_TUTORIALBUILDER) << error.what();
			}
		}
		++totalChaptersCollected;
	}
	qDebug(CAT_TUTORIALBUILDER) << "Collected a total of" << totalChaptersCollected << "in the" << m_jsonEntry << "entry.";

	// add the now sorted chapters into the tutorial overlay
	for (const auto &chapterList: qAsConst(m_chapters)) {
		// init an empty list for the chapter widgets, all wiglets should have the same description
		QList<QWidget*> chapterWidgets;
		QString description = "";
		for (const auto &chapter: qAsConst(chapterList)) {
			if (chapter.widget) {
				chapterWidgets.push_back(chapter.widget);
				description = chapter.description;
			} else {
				// no widget with this name was found, it will not be included in the tutorial and a warning will be issued
				qWarning(CAT_TUTORIALBUILDER) << "No object with the name" << chapter.name << "was found. The tutorial will skip this chapter.";
			}
		}

		this->addChapter(chapterWidgets, description);
	}

	return chaptersCollected;
}

void TutorialBuilder::readTutorialRequirements() {
	// open and read the file
	QString contents;
	if (m_jsonFile.fileName().isEmpty()) {
		qCritical(CAT_TUTORIALBUILDER) << "No file name was provided. Aborting tutorial.";
		return;
	}

	m_jsonFile.open(QIODevice::ReadOnly | QIODevice::Text);
	if (!m_jsonFile.isOpen()) {
		qCritical(CAT_TUTORIALBUILDER) << "File could not be opened (read): " << m_jsonFile.fileName();
		return;
	} else {
		qDebug(CAT_TUTORIALBUILDER) << "File opened (read): " << m_jsonFile.fileName();
	}
	contents = m_jsonFile.readAll();
	m_jsonFile.close();

	// try to parse the contents as a json
	QJsonParseError parse_error{};
	QJsonDocument document = QJsonDocument::fromJson(contents.toUtf8(), &parse_error);
	if (document.isNull()) {
		qCritical(CAT_TUTORIALBUILDER) << "Invalid json: " << parse_error.errorString();
		return;
	}

	// extract the values related to the m_jsonEntry mode
	QJsonObject document_object = document.object();
	QJsonValue value = document_object.value(m_jsonEntry);
	if (value == QJsonValue::Undefined) {
		qCritical(CAT_TUTORIALBUILDER) << "Invalid json: Could not extract value " << m_jsonEntry;
		return;
	}

	// turn the json values into an array of ChapterInstructions and save the data
	// each element from the json array must have the following structure:
	// {
	//	"index": int
	//	"name": string
	//	"description": string
	// }
	QJsonArray chapters_array = value.toArray();
	for (QJsonValueRef item: chapters_array) {
		QJsonObject item_object = item.toObject();
		int index = item_object.value("index").toInt(-1);
		QString name = item_object.value("name").toString();
		QString description = item_object.value("description").toString();

		ChapterInstructions chapterInstructions;
		chapterInstructions.name = name;
		chapterInstructions.description = tr(description.toStdString().c_str());
		chapterInstructions.widget = nullptr;
		if (m_chapters.contains(index)) {
			m_chapters[index].append(chapterInstructions);
		} else {
			m_chapters.insert(index, {chapterInstructions});
		}
	}
}

QList<std::reference_wrapper<TutorialBuilder::ChapterInstructions>> TutorialBuilder::getChapterInstructionFromName(const QString& name) {
	QList<std::reference_wrapper<TutorialBuilder::ChapterInstructions>> result;
	for (auto& chapterList: m_chapters) {
		for (auto& item: chapterList) {
			if (item.name == name) {
				result.push_back(item);
			}
		}
	}

	if (result.isEmpty()) {
		throw std::runtime_error("Invalid name.");
	} else {
		return result;
	}
}

#include "moc_tutorialbuilder.cpp"
