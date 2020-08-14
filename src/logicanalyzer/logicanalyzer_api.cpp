/*
 * Copyright (c) 2020 Analog Devices Inc.
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


#include "logicanalyzer_api.h"
#include "ui_logic_analyzer.h"

#include "annotationcurve.h"
#include "annotationdecoder.h"

#include <QCheckBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <glib.h>

#include "binding/decoder.hpp"

using namespace adiscope::logic;

using Glib::ustring;

double LogicAnalyzer_API::getSampleRate() const
{
	return m_logic->m_sampleRateButton->value();
}

void LogicAnalyzer_API::setSampleRate(double sampleRate)
{
	m_logic->m_sampleRateButton->setValue(sampleRate);
}

int LogicAnalyzer_API::getBufferSize() const
{
	return m_logic->m_bufferSizeButton->value();
}

void LogicAnalyzer_API::setBufferSize(int bufferSize)
{
	m_logic->m_bufferSizeButton->setValue(bufferSize);
}

QList<int> LogicAnalyzer_API::getEnabledChannels() const
{
	QList<int> enabledChannels;
	for (int i = 0; i < 16; ++i) {
		QLayout *widgetInLayout = m_logic->ui->channelEnumeratorLayout->itemAtPosition(i % 8,
							    i / 8)->layout();
		auto channelBox = dynamic_cast<QCheckBox *>(widgetInLayout->itemAt(0)->widget());
		if (channelBox->isChecked()) {
			enabledChannels.push_back(i);
		}
	}
	return enabledChannels;
}

void LogicAnalyzer_API::setEnabledChannels(const QList<int> &enabledChannels)
{
	for (const auto &channel : enabledChannels) {
		QLayout *widgetInLayout = m_logic->ui->channelEnumeratorLayout->itemAtPosition(channel % 8,
							    channel / 8)->layout();
		auto channelBox = dynamic_cast<QCheckBox *>(widgetInLayout->itemAt(0)->widget());
		channelBox->setChecked(true);
	}
}

QStringList LogicAnalyzer_API::getEnabledDecoders() const
{
	QStringList decoders;
	for (int i = 16; i < m_logic->m_plotCurves.size(); ++i) {
		decoders.push_back(m_logic->m_plotCurves[i]->getId());
	}
	return decoders;
}

void LogicAnalyzer_API::setEnabledDecoders(const QStringList &decoders)
{
	for (const QString &decoder : decoders) {
		m_logic->ui->addDecoderComboBox->setCurrentText(decoder);
	}
}

bool LogicAnalyzer_API::getStreamOrOneShot() const
{
	return m_logic->ui->btnStreamOneShot->isChecked();
}

void LogicAnalyzer_API::setStreamOrOneShot(bool streamOrOneShot)
{
	m_logic->ui->btnStreamOneShot->setChecked(streamOrOneShot);
}

int LogicAnalyzer_API::getDelay() const
{
	return m_logic->m_timePositionButton->value();
}

void LogicAnalyzer_API::setDelay(int delay)
{
	m_logic->m_timePositionButton->setValue(delay);
}

QStringList LogicAnalyzer_API::getChannelNames() const
{
	QStringList names;
	for (int i = 0; i < m_logic->m_plotCurves.size(); ++i) {
		names.push_back(m_logic->m_plotCurves[i]->getName());
	}
	return names;
}

void LogicAnalyzer_API::setChannelNames(const QStringList &channelNames)
{
	for (int i = 0; i < channelNames.size(); ++i) {
		m_logic->m_plotCurves[i]->setName(channelNames[i]);
	}
}

QList<double> LogicAnalyzer_API::getChannelHeights() const
{
	QList<double> heights;
	for (int i = 0; i < m_logic->m_plotCurves.size(); ++i) {
		heights.push_back(m_logic->m_plotCurves[i]->getTraceHeight());
	}
	return heights;
}

void LogicAnalyzer_API::setChannelHeights(const QList<double> &channelHeights)
{
	for (int i = 0; i < channelHeights.size(); ++i) {
		m_logic->m_plotCurves[i]->setTraceHeight(channelHeights[i]);
	}
}

QList<double> LogicAnalyzer_API::getChannelPosition() const
{
	QList<double> channelPosition;
	for (int i = 0; i < m_logic->m_plotCurves.size(); ++i) {
		channelPosition.push_back(m_logic->m_plotCurves[i]->getPixelOffset());
	}
	return channelPosition;
}

void LogicAnalyzer_API::setChannelPosition(const QList<double> &channelPosition)
{
	for (int i = 0; i < channelPosition.size(); ++i) {
		m_logic->m_plotCurves[i]->setPixelOffset(channelPosition[i]);
	}
}

QList<QList<QPair<int, int>>> LogicAnalyzer_API::getAssignedDecoderChannels() const
{
	QList<QList<QPair<int, int> > > assignedCh;
	for (int i = 16; i < m_logic->m_plotCurves.size(); ++i) {
		auto annCurve = dynamic_cast<adiscope::AnnotationCurve *>(m_logic->m_plotCurves[i]);
		if (!annCurve) {
			continue;
		}
		auto stack = annCurve->getDecoderStack();
		QList<QPair<int, int>> assignedChannels;
		for (std::shared_ptr<Decoder> decoder : stack) {
			for (const auto &ch : decoder->channels()) {
				if (ch->assigned_signal) {
					assignedChannels.push_back({ch->id, ch->bit_id});
				}
			}
		}
		assignedCh.push_back(assignedChannels);
	}
	return assignedCh;
}

void LogicAnalyzer_API::setAssignedDecoderChannels(const QList<QList<QPair<int, int>>> &assignedDecoderChannels)
{
	int currentDecoder = 16;
	for (const auto &chls : assignedDecoderChannels) {
		auto annCurve = dynamic_cast<adiscope::AnnotationCurve *>(m_logic->m_plotCurves[currentDecoder]);
		auto dec = annCurve->getAnnotationDecoder();
		for (const auto &chbitid : chls) {
			dec->assignChannel(chbitid.first, chbitid.second);
		}
		currentDecoder++;
	}
}

QList<QStringList> LogicAnalyzer_API::getDecoderStack() const
{
	QList<QStringList> decoderStack;
	for (int i = 16; i < m_logic->m_plotCurves.size(); ++i) {
		auto annCurve = dynamic_cast<adiscope::AnnotationCurve *>(m_logic->m_plotCurves[i]);
		if (!annCurve) {
			continue;
		}
		QStringList decStack;
		auto stack = annCurve->getDecoderStack();
		for (int i = 1; i < stack.size(); ++i) {
			decStack.push_back(stack[i]->decoder()->id);
		}

		decoderStack.push_back(decStack);
	}

	return decoderStack;
}

void LogicAnalyzer_API::setDecoderStack(const QList<QStringList> &decoderStack)
{
	int currentDecoder = 16;
	for (const auto &stack : decoderStack) {
		auto annCurve = dynamic_cast<adiscope::AnnotationCurve *>(m_logic->m_plotCurves[currentDecoder]);
		for (const auto &decoder : stack) {

			std::shared_ptr<logic::Decoder> dec = nullptr;

			GSList *decoderList = g_slist_copy((GSList *)srd_decoder_list());
			for (const GSList *sl = decoderList; sl; sl = sl->next) {
			    srd_decoder *srd_dec = (struct srd_decoder *)sl->data;
			    if (QString::fromUtf8(srd_dec->id) == decoder) {
				dec = std::make_shared<logic::Decoder>(srd_dec);
			    }
			}

			g_slist_free(decoderList);

			annCurve->stackDecoder(dec);
		}
		currentDecoder++;
	}
}

QList<QStringList> LogicAnalyzer_API::getDecoderSettings() const
{
	QList<QStringList> decoderSettings;
	for (int i = 16; i < m_logic->m_plotCurves.size(); ++i) {
		auto annCurve = dynamic_cast<adiscope::AnnotationCurve *>(m_logic->m_plotCurves[i]);
		if (!annCurve) {
			continue;
		}
		QStringList decSetting;
		auto bindings = annCurve->getDecoderBindings();
		for (int i = 0; i < bindings.size(); ++i) {
			QString settings;

			QJsonObject obj;
			QJsonArray propArray;
			for(auto p : bindings[i]->properties()) {

				QJsonObject propObj;
				QString prop_name(p->name());
				QString prop_type;
				QString prop_val;
				int64_t val;
				double d_val;

				std::string typ=p->get().get_type_string();
				const void *cptr = p->get().get_data();
				prop_type=QString::fromStdString(typ);
				switch(typ[0])
				{
				case 's':
					prop_val=QString::fromUtf8((const char*)cptr);
					qDebug() << "Property s: " << prop_name;
					break;
				case 'x':
					qDebug() << "Property x: " << prop_name;
					val = *((int64_t*)cptr);
					prop_val=QString::number(val);
					break;

				case 'd':
					qDebug() << "Property d: " << prop_name;
					d_val = *((double*)cptr);
					prop_val=QString::number(d_val);
					break;

				// case bool & enum /string
				// decode all
				default:
					qDebug()<<"error";
					break;

				}
				propObj["name"] = prop_name;
				propObj["type"] = prop_type;
				propObj["val"] = prop_val;
				propArray.append(propObj);
				qDebug()<<propObj;
			}
			obj["properties"] = propArray;
			QJsonValue val(obj);
			QJsonDocument doc(obj);
			QString ret(doc.toJson(QJsonDocument::Compact));
			settings = ret;

			decSetting.push_back(settings);
		}

		decoderSettings.push_back(decSetting);
	}

	return decoderSettings;
}

void LogicAnalyzer_API::setDecoderSettings(const QList<QStringList> &decoderSettings)
{
	int currentDecoder = 16;
	for (const auto &setting : decoderSettings) {
		auto annCurve = dynamic_cast<adiscope::AnnotationCurve *>(m_logic->m_plotCurves[currentDecoder]);
		auto bindings = annCurve->getDecoderBindings();
		int currentBinding = 0;
		for (const auto &sett : setting) {
			auto binding = bindings[currentBinding++];


			QJsonObject obj;
			QJsonDocument doc = QJsonDocument::fromJson(sett.toUtf8());

			if (!doc.isNull()) {
				if (doc.isObject()) {
					obj = doc.object();
				} else {
//					qDebug(CAT_LOGIC_ANALYZER) << "Document is not an object" << endl;
				}
			} else {
//				qDebug(CAT_LOGIC_ANALYZER) << "Invalid JSON...\n";
			}

			QJsonArray propArray = obj["properties"].toArray();
			for (auto propRef : propArray) {
				auto prop = propRef.toObject();
				for(auto p : binding->properties())
				{
					qDebug()<<p->name();
					if(p->name() == prop["name"].toString())
					{
						QByteArray ba;
						GVariant *new_value = nullptr;
						Glib::VariantBase value_;

						switch(prop["type"].toString()[0].toLatin1())
						{
						case 's':
							ba =prop["val"].toString().toLocal8Bit();
							p->set(Glib::Variant<ustring>::create(ba.data()));
							break;

						case 'd':
							new_value = g_variant_new_double(prop["val"].toString().toDouble());
							value_ = Glib::VariantBase(new_value);
							p->set(value_);
							break;

						case 'x':

							new_value = g_variant_new_int64(prop["val"].toString().toLongLong());
							value_ = Glib::VariantBase(new_value);
							p->set(value_);
							break;
						default:
//							qDebug(CAT_LOGIC_ANALYZER)<<"ERROR";
							break;
						}

					}

				}

			}
		}
		currentDecoder++;
	}
}

QVector<QVector<int> > LogicAnalyzer_API::getCurrentGroups() const
{
	return m_logic->m_plot.getAllGroups();
}

void LogicAnalyzer_API::setCurrentGroups(const QVector<QVector<int> > &groups)
{
	m_logic->m_plot.setGroups(groups);
}

QString LogicAnalyzer_API::getNotes()
{
	return m_logic->ui->instrumentNotes->getNotes();
}
void LogicAnalyzer_API::setNotes(QString str)
{
	m_logic->ui->instrumentNotes->setNotes(str);
}
