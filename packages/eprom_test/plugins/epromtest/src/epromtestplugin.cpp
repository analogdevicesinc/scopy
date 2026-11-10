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

#include "epromtestplugin.h"
#include "epromconfigtool.h"
#include "epromtest_api.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>

#include <pkg-manager/pkgmanager.h>
#include <pluginbase/messagebroker.h>
#include <pluginbase/scopyjs.h>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_EPROMTESTPLUGIN, "EpromTestPlugin")
using namespace scopy;

namespace {
// Category of the device added by core's DongleDeviceManager; repeated here so this plugin doesn't link core.
const QString DONGLE_CATEGORY = "dongle";
const QString PACKAGE_DIR = "eprom_test";
// The simulated boards that can sit behind the dongle; switchDevice() replaces one with the other
struct SimulatedBoard
{
	QString name;
	QString eepromFile; // the simulated EEPROM of the board
};
const QList<SimulatedBoard> BOARDS = {{"board 1", "eeprom_board.json"}, {"board 2", "eeprom_board2.json"}};

// Keys of the regmap section of the simulated EEPROM JSON
const QString EEPROM_REGMAP = "regmap";
const QString KEY_TARGET = "target";
const QString KEY_TEMPLATE = "template";
const QString KEY_ACCESS = "access";
const QString KEY_FILE = "file";

QString findPackageFile(const QString &fileName)
{
	const QFileInfoList files = PkgManager::listFilesInfo({PACKAGE_DIR}, {fileName});
	if(files.isEmpty()) {
		qWarning(CAT_EPROMTESTPLUGIN) << "File" << fileName << "not found in" << PACKAGE_DIR;
		return "";
	}
	return files.first().absoluteFilePath();
}

// A file name from the EEPROM, as a path in the package. An empty name stays empty (no template, no file).
QString resolve(const QString &fileName) { return fileName.isEmpty() ? "" : findPackageFile(fileName); }
} // namespace

bool EpromTestPlugin::compatible(QString m_param, QString category)
{
	Q_UNUSED(m_param)
	return category == DONGLE_CATEGORY;
}

void EpromTestPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("epromConfigTool", "EEPROM configuration",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/gear_wheel.svg"));
}

QString EpromTestPlugin::displayName() { return "dongleDevice"; }

bool EpromTestPlugin::onConnect()
{
	m_tool = new EpromConfigTool();
	connect(m_tool, &EpromConfigTool::syncRequested, this, &EpromTestPlugin::sync);
	connect(m_tool, &EpromConfigTool::configureRequested, this, &EpromTestPlugin::configure);
	connect(m_tool, &EpromConfigTool::switchDeviceRequested, this, &EpromTestPlugin::switchDevice);
	m_tool->setBoard(board());
	m_toolList[0]->setTool(m_tool);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(false);

	initApi();
	return true;
}

bool EpromTestPlugin::onDisconnect()
{
	if(m_api) {
		ScopyJS::GetInstance()->unregisterApi(m_api);
		delete m_api;
		m_api = nullptr;
	}

	for(ToolMenuEntry *tme : std::as_const(m_toolList)) {
		tme->setEnabled(false);
		QWidget *w = tme->tool();
		if(w) {
			tme->setTool(nullptr);
			delete w;
		}
	}
	m_tool = nullptr;

	// every connect starts with board 1 and an empty register map
	m_board = 0;
	m_syncedBoard.clear();
	m_synced.clear();
	m_applied.clear();
	return true;
}

QString EpromTestPlugin::board() const { return BOARDS.value(m_board).name; }

void EpromTestPlugin::switchDevice()
{
	// the board behind the dongle was replaced: what was synced is stale, what was applied stays until Configure
	m_board = (m_board + 1) % BOARDS.size();
	m_syncedBoard.clear();
	m_synced.clear();
	qInfo(CAT_EPROMTESTPLUGIN) << "Board replaced, now" << board();
	if(m_tool) {
		m_tool->setBoard(board());
		m_tool->setStructure("");
		m_tool->setConfigureEnabled(false);
		m_tool->setStatus("Board replaced: " + board() + ". Press Sync to read its EEPROM.");
	}
}

bool EpromTestPlugin::sync()
{
	m_syncedBoard.clear();
	m_synced.clear();
	if(m_tool) {
		m_tool->setStructure("");
		m_tool->setConfigureEnabled(false);
	}

	auto fail = [this](const QString &error) {
		qWarning(CAT_EPROMTESTPLUGIN) << error;
		if(m_tool) {
			m_tool->setStatus(error);
		}
		return false;
	};

	const QString eepromFile = BOARDS.value(m_board).eepromFile;
	const QString path = findPackageFile(eepromFile);
	QFile file(path);
	if(path.isEmpty() || !file.open(QIODevice::ReadOnly)) {
		return fail("Can't read the board EEPROM (" + eepromFile + ")");
	}
	QJsonParseError err;
	const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
	if(!doc.isObject()) {
		return fail("Invalid board EEPROM: " + err.errorString());
	}

	for(const QJsonValue &value : doc.object().value(EEPROM_REGMAP).toArray()) {
		const QJsonObject entry = value.toObject();
		RegisterMapToolConfig config;
		config.target = entry.value(KEY_TARGET).toString();
		config.templatePath = resolve(entry.value(KEY_TEMPLATE).toString());
		bool ok = true;
		const QString access = entry.value(KEY_ACCESS).toString("iio");
		config.access = RegisterMapToolConfig::accessFromString(access, &ok);
		if(!ok) {
			qWarning(CAT_EPROMTESTPLUGIN)
				<< "Unknown access" << access << "for" << config.target << ", skipped";
			continue;
		}
		config.filePath = resolve(entry.value(KEY_FILE).toString());
		if(!config.target.isEmpty()) {
			m_synced.append(config);
		}
	}
	if(m_synced.isEmpty()) {
		return fail("The board EEPROM has no register map configuration");
	}

	m_syncedBoard = doc.object().value("board").toString();
	qInfo(CAT_EPROMTESTPLUGIN) << "Synced" << m_synced.size() << "regmap entries from" << m_syncedBoard;
	if(m_tool) {
		m_tool->setStructure(structure());
		m_tool->setStatus(
			QString("Synced %1 register map devices. Press Configure to apply them.").arg(m_synced.size()));
		m_tool->setConfigureEnabled(true);
	}
	return true;
}

bool EpromTestPlugin::configure()
{
	if(m_synced.isEmpty()) {
		qWarning(CAT_EPROMTESTPLUGIN) << "Nothing synced, press Sync first";
		return false;
	}
	m_applied = m_synced;
	// regmap pulls again and gets m_applied through toolConfiguration()
	MessageBroker::GetInstance()->publish(REGISTER_MAP_CONFIGURE_TOPIC, m_param);
	if(m_tool) {
		m_tool->setStatus(QString("Configured %1 register map devices.").arg(m_applied.size()));
	}
	return true;
}

QString EpromTestPlugin::structure() const
{
	if(m_synced.isEmpty()) {
		return "";
	}
	QJsonArray entries;
	for(const RegisterMapToolConfig &config : m_synced) {
		QJsonObject entry;
		entry.insert(KEY_TARGET, config.target);
		entry.insert(KEY_TEMPLATE, config.templatePath);
		entry.insert(KEY_ACCESS, RegisterMapToolConfig::accessToString(config.access));
		if(config.access == RegisterMapToolConfig::File) {
			entry.insert(KEY_FILE, config.filePath);
		}
		entries.append(entry);
	}
	QJsonObject root;
	root.insert("board", m_syncedBoard);
	root.insert(EEPROM_REGMAP, entries);
	return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

QList<RegisterMapToolConfig> EpromTestPlugin::registerMapConfiguration(const QVariantMap &query)
{
	Q_UNUSED(query)
	return m_applied;
}

void EpromTestPlugin::initApi()
{
	m_api = new EpromTest_API(this);
	m_api->setObjectName("eprom_test");
	ScopyJS::GetInstance()->registerApi(m_api);
}

void EpromTestPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":10,
	   "category":[
	      "dongle"
	   ]
	}
)plugin");
}

#include "moc_epromtestplugin.cpp"
