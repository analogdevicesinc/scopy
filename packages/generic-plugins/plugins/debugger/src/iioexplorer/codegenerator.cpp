/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#include "codegenerator.h"
#include <QSet>
#include <QMap>
#include <QDebug>
#include "iiostandarditem.h"

CodeGenerator::CodeGeneratorRecipe CodeGenerator::convertToCodeGeneratorRecipe(scopy::debugger::IIOStandardItem *item,
									       QString uri)
{
	QString path = item->path();
	QStringList pathList = path.split("/", Qt::SkipEmptyParts);
	QString ctx;
	QString dev;
	QString chan;
	QString attr;
	bool isInput = false;

	switch(item->type()) {
	case scopy::debugger::IIOStandardItem::Type::Context:
		ctx = pathList[0];
		break;
	case scopy::debugger::IIOStandardItem::Type::ContextAttribute:
		ctx = pathList[0];
		attr = pathList[1];
		break;
	case scopy::debugger::IIOStandardItem::Type::Device:
		ctx = pathList[0];
		dev = pathList[1];
		break;
	case scopy::debugger::IIOStandardItem::Type::DeviceAttribute:
		ctx = pathList[0];
		dev = pathList[1];
		attr = pathList[2];
		break;
	case scopy::debugger::IIOStandardItem::Type::Trigger:
		// TODO: Gen special code for this
		ctx = pathList[0];
		dev = pathList[1];
		break;
	case scopy::debugger::IIOStandardItem::Type::Channel:
		ctx = pathList[0];
		dev = pathList[1];
		chan = pathList[2];
		isInput = !item->isOutput();
		break;
	case scopy::debugger::IIOStandardItem::Type::ChannelAttribute:
		ctx = pathList[0];
		dev = pathList[1];
		chan = pathList[2];
		attr = pathList[3];
		isInput = !item->isOutput();
		break;
	}

	CodeGenerator::CodeGeneratorRecipe recipe = {
		.uri = uri,
		.deviceName = dev,
		.channelName = chan,
		.channelIsInput = isInput,
		.attributeName = attr,
	};

	return recipe;
}
// TODO: Break this into smaller functions
QString CodeGenerator::generateCode(QList<CodeGeneratorRecipe> recipes)
{
	if(recipes.isEmpty()) {
		return QString();
	}

	QString code;

	code = QString("// Compile with: gcc main.cpp -o main.exe -liio\n");
	code += QString("#include <iio.h>\n");
	code += QString("#include <stdio.h>\n");
	code += QString("#include <errno.h>\n");
	code += QString("\n");
	code += QString("int main(int argc, char *argv[]) {\n");
	code += QString("  char buffer[1024];\n");
	code += QString("  ssize_t ret;\n");

	// FIXME: this does not handle device debug attributes
	// TODO: Add comments to the generated code
	code += QString("  struct iio_context *ctx = iio_create_context_from_uri(\"%1\");\n").arg(recipes[0].uri);
	code += QString("  if (!ctx) {\n");
	code += QString("    printf(\"Error %d: Failed to create context\\n\", errno);\n");
	code += QString("    return -1;\n");
	code += QString("  }\n");

	// Ctx only, it is guaranteed that there is at least one recipe
	// TODO: Add case when reading more than one ctx attribute
	if(recipes[0].deviceName.isEmpty()) {
		if(!recipes[0].attributeName.isEmpty()) {
			// Ctx attribute
			code += QString("  unsigned int nb_ctx_attrs = iio_context_get_attrs_count(ctx);\n");
			code += QString("  const char *key, *value;\n");
			code += QString("  for (int i = 0; i < nb_ctx_attrs; i++) {\n");
			code += QString("    ret = iio_context_get_attr(ctx, i, &key, &value);\n");
			code += QString("    if (!ret) {\n");
			code += QString("      printf(\"Error %d: Failed to get attribute\\n\", ret);\n");
			code += QString("    }\n");
			code += QString("    if (strcmp(key, \"%1\") == 0) {\n").arg(recipes[0].attributeName);
			code += QString("      printf(\"%1: %s\\n\", value);\n").arg(recipes[0].attributeName);
			code += QString("    }\n");
			code += QString("  }\n");
		}
	}

	struct ChannelItems
	{
		QList<QString> attrs;
	};

	struct DeviceItems
	{
		QMap<QPair<QString, bool>, ChannelItems> channels;
		QList<QString> attrs;
	};

	QMap<QString, DeviceItems> devices;

	for(const auto &recipe : qAsConst(recipes)) {
		// Device
		if(!recipe.deviceName.isEmpty()) {
			DeviceItems &device = devices[recipe.deviceName];

			// Channel
			if(!recipe.channelName.isEmpty()) {
				ChannelItems &channel = device.channels[{recipe.channelName, recipe.channelIsInput}];
				if(!recipe.attributeName.isEmpty()) {
					channel.attrs.append(recipe.attributeName);
				}
			} else if(!recipe.attributeName.isEmpty()) {
				device.attrs.append(recipe.attributeName);
			}
		}
	}

	for(QMapIterator<QString, DeviceItems> it(devices); it.hasNext();) {
		it.next();
		code += QString("  struct iio_device *%1 = iio_context_find_device(ctx, \"%2\");\n")
				.arg(var(it.key()), it.key());
		code += QString("  if (!%1) {\n").arg(var(it.key()));
		code += QString("    printf(\"Error: Could not find device %1\\n\");\n").arg(it.key());
		code += QString("  }\n");

		const DeviceItems &device = it.value();
		for(const QString &attr : device.attrs) {
			QString devAttrName = it.key() + "_" + attr;
			code += QString("  const char *%1 = iio_device_find_attr(%2, \"%3\");\n")
					.arg(var(devAttrName), var(it.key()), attr);
			code += QString("  if (!%1) {\n").arg(var(devAttrName));
			code += QString("    printf(\"Error: Could not find device attribute %1\\n\");\n").arg(attr);
			code += QString("  }\n");

			code += QString("  ret = iio_device_attr_read(%1, \"%2\", buffer, sizeof(buffer));\n")
					.arg(var(it.key()), attr);
			code += QString("  if (ret < 0) {\n");
			code += QString("    printf(\"Error %zd: Failed to read attribute %1\\n\", ret);\n").arg(attr);
			code += QString("  } else {\n");
			code += QString("    printf(\"%1: %s\\n\", buffer);\n").arg(attr);
			code += QString("  }\n");
		}

		for(QMapIterator<QPair<QString, bool>, ChannelItems> it2(device.channels); it2.hasNext();) {
			it2.next();
			QString varName =
				it.key() + "_" + it2.key().first + "_" + (it2.key().second ? "input" : "output");
			QString channelName = it2.key().first;
			QString isOutput = it2.key().second ? "false" : "true";
			code += QString("  struct iio_channel *%1 = iio_device_find_channel(%2, \"%3\", %4);\n")
					.arg(var(varName), var(it.key()), channelName, isOutput);
			code += QString("  if (!%1) {\n").arg(var(varName));
			code += QString("    printf(\"Error: Could not find channel %1\\n\");\n").arg(channelName);
			code += QString("  }\n");

			const ChannelItems &channel = it2.value();
			for(const auto &attr : channel.attrs) {
				code += QString("  const char *%1 = iio_channel_find_attr(%2, \"%3\");\n")
						.arg(var(varName + "_" + attr), var(varName), attr);
				code += QString("  if (!%1) {\n").arg(var(varName + "_" + attr));
				code += QString("    printf(\"Error: Could not find channel attribute %1\\n\");\n")
						.arg(attr);
				code += QString("  }\n");

				code += QString("  ret = iio_channel_attr_read(%1, %2, buffer, sizeof(buffer));\n")
						.arg(var(varName), var(varName + "_" + attr));
				code += QString("  if (ret < 0) {\n");
				code += QString("    printf(\"Error %zd: Failed to read attribute %1\\n\", ret);\n")
						.arg(attr);
				code += QString("  } else {\n");
				code += QString("    printf(\"%1: %s\\n\", buffer);\n").arg(attr);
				code += QString("  }\n");
			}
		}
	}

	code += QString("  iio_context_destroy(ctx);\n");
	code += QString("  return 0;\n");
	code += QString("}\n");
	code += QString("\n");

	return code;
}

QString CodeGenerator::var(QString name) { return name.replace("-", "_").replace("/", "_").replace(",", "_"); }
