#ifndef SIGNAL_GENERATOR_MENU_HPP
#define SIGNAL_GENERATOR_MENU_HPP

#include <QWidget>

namespace Ui {
class SignalGeneratorMenu;
}

namespace scopy {
namespace gui {

class PhaseSpinButton;
class PositionSpinButton;
class ScaleSpinButton;

class SignalGeneratorMenu : public QWidget
{
	Q_OBJECT

public:
	explicit SignalGeneratorMenu(QWidget* parent = nullptr);
	~SignalGeneratorMenu();

	static constexpr float maxFrequency = 30000000;

private:
	Ui::SignalGeneratorMenu* m_ui;

	double m_sampleRate;

	PhaseSpinButton* m_phase;
	PositionSpinButton *m_filePhase, *m_stairPhase;
	PositionSpinButton *m_offset, *m_fileOffset;
	PositionSpinButton *m_constantValue, *m_dutycycle;
	ScaleSpinButton *m_amplitude, *m_frequency;
	ScaleSpinButton *m_riseTime, *m_fallTime;
	PositionSpinButton *m_stepsUp, *m_stepsDown;
	ScaleSpinButton *m_holdHighTime, *m_holdLowTime;
	ScaleSpinButton *m_fileSampleRate, *m_fileAmplitude;
	ScaleSpinButton *m_mathRecordLength, *m_noiseAmplitude, *m_mathSampleRate;
};
} // namespace gui
} // namespace scopy

#endif // SIGNAL_GENERATOR_MENU_HPP
