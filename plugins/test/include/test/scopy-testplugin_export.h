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

#ifndef SCOPY_TESTPLUGIN_EXPORT_H
#define SCOPY_TESTPLUGIN_EXPORT_H

#ifdef SCOPY_TESTPLUGIN_STATIC_DEFINE
#define SCOPY_TESTPLUGIN_EXPORT
#define SCOPY_TESTPLUGIN_NO_EXPORT
#else
#ifndef SCOPY_TESTPLUGIN_EXPORT
#ifdef scopy_testplugin_EXPORTS
/* We are building this library */
#define SCOPY_TESTPLUGIN_EXPORT __attribute__((visibility("default")))
#else
/* We are using this library */
#define SCOPY_TESTPLUGIN_EXPORT __attribute__((visibility("default")))
#endif
#endif

#ifndef SCOPY_TESTPLUGIN_NO_EXPORT
#define SCOPY_TESTPLUGIN_NO_EXPORT __attribute__((visibility("hidden")))
#endif
#endif

#ifndef SCOPY_TESTPLUGIN_DEPRECATED
#define SCOPY_TESTPLUGIN_DEPRECATED __attribute__((__deprecated__))
#endif

#ifndef SCOPY_TESTPLUGIN_DEPRECATED_EXPORT
#define SCOPY_TESTPLUGIN_DEPRECATED_EXPORT SCOPY_TESTPLUGIN_EXPORT SCOPY_TESTPLUGIN_DEPRECATED
#endif

#ifndef SCOPY_TESTPLUGIN_DEPRECATED_NO_EXPORT
#define SCOPY_TESTPLUGIN_DEPRECATED_NO_EXPORT SCOPY_TESTPLUGIN_NO_EXPORT SCOPY_TESTPLUGIN_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#ifndef SCOPY_TESTPLUGIN_NO_DEPRECATED
#define SCOPY_TESTPLUGIN_NO_DEPRECATED
#endif
#endif

#endif /* SCOPY_TESTPLUGIN_EXPORT_H */
