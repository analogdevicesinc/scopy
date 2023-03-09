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

#ifndef PLOT_UTILS_HPP
#define PLOT_UTILS_HPP

#include <vector>
#include <qstring.h>

namespace adiscope {
	class PrefixFormatter
	{
	public:
		PrefixFormatter(const std::vector<std::pair<QString, double>>&);
		virtual ~PrefixFormatter();
		void setTwoDecimalMode(bool);
		bool getTwoDecimalMode();
		virtual QString format(double value, QString unitType, int precision) const;
		void getFormatAttributes(double value, QString& prefix, double& scale) const;

		bool getTrimZeroes() const;
		void setTrimZeroes(bool trimZeroes);

	protected:
		virtual QString buildString(double value, QString prefix, QString unitType, int precision) const;
		int findPrefixIndex(double value) const;

	private:
		std::vector<std::pair<QString, double>> m_prefixes;
		int m_defaultPrefixIndex;
		bool m_twoDecimalMode;
		bool m_trimZeroes;
	};

	class MetricPrefixFormatter: public PrefixFormatter
	{
	public:
		MetricPrefixFormatter();
	};

	class TimePrefixFormatter: public PrefixFormatter
	{
	public:
		TimePrefixFormatter();
	};

	/*
	 * CLASS NumberSeries contains a series of number that is being built
	 * upon contruction. The series is being build by the following rules:
	 * limits: [lower, upper]
	 * step: {step1, step2, ..., stepN} X stepPower
	 * E.g. powerStep = 10, steps = {1, 2, 5} -> ..., 0.1, 0.2, 0.5, 1.0,
	 *      2.0, 5.0, 10.0, 20.0, 50.0, ...
	 */
	class NumberSeries
	{
	public:
		NumberSeries(double lower = 1E-3, double upper = 1E3,
			     unsigned int powerStep = 10,
			     const std::vector<double>& steps = {1, 2, 5});
		~NumberSeries();

		const std::vector<double>& getNumbers();

		double getNumberAfter(double value);
		double getNumberBefore(double value);

		void setLower(double value);
		double lower();

		void setUpper(double value);
		double upper();

		void setPowerStep(unsigned int value);
		unsigned int stepPower();

	protected:
		void buildNumberSeries();

	private:
		double m_lowerLimit;
		double m_upperLimit;
		int m_powerStep;
		std::vector<double> m_templateSteps;
		std::vector<double>m_numbers;
	};
}

#endif //PLOT_UTILS_HPP
