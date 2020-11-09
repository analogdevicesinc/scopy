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

#ifndef SINGLETONE_WRAPPER_H
#define SINGLETONE_WRAPPER_H

namespace adiscope {
template <typename T>
class SingleToneWrapper {
public:
        SingleToneWrapper() = default;

        SingleToneWrapper(const SingleToneWrapper<T>&) = delete;
        SingleToneWrapper& operator=(const SingleToneWrapper<T> &) = delete;
public:
        static SingleToneWrapper& getInstance() {
                static SingleToneWrapper<T> instance;
                return instance;
        }

        void setWrapped(T wrapped) {
                m_wrapped = wrapped;
        }

        T getWrapped() {
              return m_wrapped;
        }

private:
        T m_wrapped;
};

} // namespace adiscope

#endif // SINGLETONE_WRAPPER_H
