/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "scopyjs.h"

#include "jsfileio.h"

#include <QApplication>
#include <QElapsedTimer>
#include <QMetaProperty>
#include <QThread>
#include <QtConcurrent>

#include <common/common.h>
#include <iostream>
#include <thread>
#include <unistd.h>

#ifdef Q_OS_UNIX
#include <fcntl.h>
#include <sys/stat.h>
#endif

Q_LOGGING_CATEGORY(CAT_JS, "Scopy_JS")

using std::cout;
using namespace scopy;

QLoggingCategory::CategoryFilter ScopyJS::oldCategoryFilter{nullptr};

ScopyJS *ScopyJS::pinstance_{nullptr};
ScopyJS::ScopyJS(QObject *parent)
	: QObject(parent)
{}

ScopyJS::~ScopyJS()
{
#ifdef Q_OS_UNIX
	cleanupMcpPipe();
#endif
}

ScopyJS *ScopyJS::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new ScopyJS(QApplication::instance()); // singleton has the app as parent
		pinstance_->init();
	}
	return pinstance_;
}

void ScopyJS::exit() { QApplication::closeAllWindows(); }

void ScopyJS::init()
{
	QJSValue js_obj = m_engine.newQObject(this);
	auto meta = metaObject();
	input = "";

	for(int i = meta->methodOffset(); i < meta->methodCount(); i++) {
		QString name(meta->method(i).name());

		m_engine.globalObject().setProperty(name, js_obj.property(name));
	}

	m_engine.globalObject().setProperty("inspect()",
					    m_engine.evaluate("(function(o) { for (each in o) { print(each); } })"));
	m_engine.installExtensions(QJSEngine::ConsoleExtension);
	m_engine.globalObject().setProperty("fileIO", m_engine.newQObject(new JsFileIo(this)));

	if(isatty(STDIN_FILENO)) {
		// verify if stdin is a true TTY - prevents QtCreator Application output from flooding the application
		// notifier->setEnabled(false);
		notifier = new QSocketNotifier(STDIN_FILENO, QSocketNotifier::Read);
		connect(notifier, SIGNAL(activated(int)), this, SLOT(hasText()));
	}

#ifdef Q_OS_UNIX
	initMcpPipe();
#endif
}

void ScopyJS::returnToApplication()
{
	bool done;
	connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, [&done]() { done = true; });

	while(!done) {
		QCoreApplication::processEvents();
		QThread::msleep(1);
	}
}

void ScopyJS::suppressScopyMessages(bool b)
{
	if(b) {
		QLoggingCategory::installFilter(jsCategoryFilter);
	} else {
		QLoggingCategory::installFilter(oldCategoryFilter);
	}
}

QJSEngine *ScopyJS::engine() { return &m_engine; }

void ScopyJS::registerApi(ApiObject *obj) { registerApi(obj, m_engine.globalObject()); }

void ScopyJS::unregisterApi(ApiObject *obj) { unregisterApi(obj, m_engine.globalObject()); }

void ScopyJS::registerApi(ApiObject *obj, QJSValue parentObj)
{
	auto newjsobj = m_engine.newQObject(obj);
	parentObj.setProperty(obj->objectName(), newjsobj);

	//	auto meta = obj->metaObject();

	//	for (int i = meta->propertyOffset();
	//			i < meta->propertyCount(); i++) {
	//		auto prop = meta->property(i);

	//		auto data = prop.read(obj);
	//		if (data.canConvert<ApiObject *>()) {
	//			registerApi(data.value<ApiObject*>(),newjsobj);
	//		}
	//	}

	//	QList<ApiObject*> list = obj->findChildren<ApiObject*>(QString(),Qt::FindDirectChildrenOnly);
	//	for(auto &&apiobj : list) {
	//		registerApi(apiobj,newjsobj);
	//	}
}

void ScopyJS::unregisterApi(ApiObject *obj, QJSValue parentObj) { parentObj.deleteProperty(obj->objectName()); }

void ScopyJS::sleep(unsigned long s) { msleep(s * 1000); }

void ScopyJS::msleep(unsigned long ms)
{
	QElapsedTimer timer;

	timer.start();
	while(!timer.hasExpired(ms)) {
		QCoreApplication::processEvents();
		QThread::msleep(1);
	}
}

QString ScopyJS::printToConsole(const QString &text)
{
	cout << text.toStdString() << std::endl;
	return text;
}

QString ScopyJS::readFromConsole(const QString &request)
{
	done = false;
	input = "";

	std::cout << request.toStdString() << std::endl;
	connect(&watcher, &QFutureWatcher<QString>::finished, this, [&]() {
		input = watcher.result();
		done = true;
	});
	future = QtConcurrent::run(std::bind(&ScopyJS::readInput, this));
	watcher.setFuture(future);

	do {
		QCoreApplication::processEvents();
		QThread::msleep(10);
	} while(!done && (input == ""));

	return input;
}

QString ScopyJS::readInput()
{
	std::string in;
	std::cin >> in;
	return QString::fromStdString(in);
}

void ScopyJS::hasText()
{
	QTextStream in(stdin);
	QTextStream out(stdout);

	QString str = in.readLine();
	js_cmd.append(str);

	qWarning() << str;
	unsigned int nb_open_braces = js_cmd.count(QChar('{'));
	unsigned int nb_closing_braces = js_cmd.count(QChar('}'));

	if(nb_open_braces == nb_closing_braces) {
		QJSValue val = m_engine.evaluate(js_cmd);

		if(val.isError()) {
			out << "Exception:" << val.toString() << Qt::endl;
		} else if(!val.isUndefined()) {
			out << val.toString() << Qt::endl;
		}

		js_cmd.clear();
		out << "scopy > ";
	} else {
		js_cmd.append(QChar('\n'));

		out << "> ";
	}

	out.flush();
}

void ScopyJS::jsCategoryFilter(QLoggingCategory *category)
{
	if(oldCategoryFilter) {
		oldCategoryFilter(category);
	}
	category->setEnabled(QtDebugMsg, false);
	category->setEnabled(QtInfoMsg, false);
	category->setEnabled(QtWarningMsg, false);
	category->setEnabled(QtCriticalMsg, false);
	category->setEnabled(QtFatalMsg, false);

	if(qstrcmp(category->categoryName(), "Scopy_JS") == 0) {
		category->setEnabled(QtDebugMsg, true);
		category->setEnabled(QtInfoMsg, true);
		category->setEnabled(QtWarningMsg, true);
		category->setEnabled(QtCriticalMsg, true);
		category->setEnabled(QtFatalMsg, true);
	}

	if(qstrcmp(category->categoryName(), "Scopy_API") == 0) {
		category->setEnabled(QtWarningMsg, true);
	}
}

QJSValue ScopyJS::evaluateFile(const QString &path)
{
	QTextStream out(stdout);
	QFile file(path);
	if(!file.open(QFile::ReadOnly)) {
		out << "Unable to open the script file: " << path << Qt::endl;
		return "";
	}
	const QString scriptContent = getScriptContent(&file);
	QJSValue val = ScopyJS::GetInstance()->engine()->evaluate(scriptContent, path);
	if(val.isError()) {
		out << "Exception:" << val.toString() << Qt::endl;
	} else if(!val.isUndefined()) {
		out << val.toString() << Qt::endl;
	}

	out.flush();

	return val;
}

const QString ScopyJS::getScriptContent(QFile *file)
{
	QTextStream stream(file);
	QString firstLine = stream.readLine();
	if(!firstLine.startsWith("#!"))
		stream.seek(0);

	QString content = stream.readAll();
	file->close();
	return content;
}

#ifdef Q_OS_UNIX
void ScopyJS::initMcpPipe()
{
	// Remove stale FIFOs from previous runs
	unlink(MCP_CMD_PIPE);
	unlink(MCP_RSP_PIPE);

	// Create FIFOs with owner-only permissions
	if(mkfifo(MCP_CMD_PIPE, 0600) != 0) {
		qWarning(CAT_JS) << "MCP: Failed to create command pipe" << MCP_CMD_PIPE;
		return;
	}
	if(mkfifo(MCP_RSP_PIPE, 0600) != 0) {
		qWarning(CAT_JS) << "MCP: Failed to create response pipe" << MCP_RSP_PIPE;
		unlink(MCP_CMD_PIPE);
		return;
	}

	// Open command pipe for reading (non-blocking to avoid hanging init)
	m_pipeFd = open(MCP_CMD_PIPE, O_RDONLY | O_NONBLOCK);
	if(m_pipeFd < 0) {
		qWarning(CAT_JS) << "MCP: Failed to open command pipe for reading";
		unlink(MCP_CMD_PIPE);
		unlink(MCP_RSP_PIPE);
		return;
	}

	m_pipeNotifier = new QSocketNotifier(m_pipeFd, QSocketNotifier::Read, this);
	QObject::connect(m_pipeNotifier, &QSocketNotifier::activated, this, &ScopyJS::onPipeData);
	qInfo(CAT_JS) << "MCP pipe listener active on" << MCP_CMD_PIPE;
}

void ScopyJS::onPipeData()
{
	char buf[4096];
	ssize_t n = read(m_pipeFd, buf, sizeof(buf) - 1);
	if(n <= 0) {
		// Writer closed the pipe; reopen to accept next connection
		m_pipeNotifier->setEnabled(false);
		close(m_pipeFd);
		m_pipeFd = open(MCP_CMD_PIPE, O_RDONLY | O_NONBLOCK);
		if(m_pipeFd >= 0) {
			delete m_pipeNotifier;
			m_pipeNotifier = new QSocketNotifier(m_pipeFd, QSocketNotifier::Read, this);
			QObject::connect(m_pipeNotifier, &QSocketNotifier::activated, this, &ScopyJS::onPipeData);
		}
		return;
	}

	buf[n] = '\0';
	QString cmd = QString::fromUtf8(buf).trimmed();
	if(cmd.isEmpty()) {
		return;
	}

	QJSValue val = m_engine.evaluate(cmd);
	QString result;
	if(val.isError()) {
		result = "ERROR:" + val.toString();
	} else if(val.isUndefined()) {
		result = "OK:undefined";
	} else {
		result = "OK:" + val.toString();
	}

	// Write result to response pipe
	int rspFd = open(MCP_RSP_PIPE, O_WRONLY);
	if(rspFd >= 0) {
		QByteArray rspBytes = (result + "\n").toUtf8();
		write(rspFd, rspBytes.constData(), rspBytes.size());
		close(rspFd);
	}
}

void ScopyJS::cleanupMcpPipe()
{
	if(m_pipeNotifier) {
		m_pipeNotifier->setEnabled(false);
		delete m_pipeNotifier;
		m_pipeNotifier = nullptr;
	}
	if(m_pipeFd >= 0) {
		close(m_pipeFd);
		m_pipeFd = -1;
	}
	unlink(MCP_CMD_PIPE);
	unlink(MCP_RSP_PIPE);
}
#endif

#include "moc_scopyjs.cpp"
