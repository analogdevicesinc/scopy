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



#ifndef SCOPY_SWIOT_EXPORT_H
#define SCOPY_SWIOT_EXPORT_H

#ifdef SCOPYSWIOT_STATIC_DEFINE
#  define SCOPY_SWIOT_EXPORT
#  define SCOPYSWIOT_NO_EXPORT
#else
#  ifndef SCOPY_SWIOT_EXPORT
#    ifdef scopyswiot_EXPORTS
        /* We are building this library */
#      define SCOPY_SWIOT_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPY_SWIOT_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPYSWIOT_NO_EXPORT
#    define SCOPYSWIOT_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPYSWIOT_DEPRECATED
#  define SCOPYSWIOT_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPYSWIOT_DEPRECATED_EXPORT
#  define SCOPYSWIOT_DEPRECATED_EXPORT SCOPY_SWIOT_EXPORT SCOPYSWIOT_DEPRECATED
#endif

#ifndef SCOPYSWIOT_DEPRECATED_NO_EXPORT
#  define SCOPYSWIOT_DEPRECATED_NO_EXPORT SCOPYSWIOT_NO_EXPORT SCOPYSWIOT_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPYSWIOT_NO_DEPRECATED
#    define SCOPYSWIOT_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPY_SWIOT_EXPORT_H */
