#include "dmm_api.hpp"

#include "ui_dmm.h"

namespace adiscope {

void DMM_API::show() { Q_EMIT dmm->showTool(); }

bool DMM_API::get_mode_ac_high_ch1() const {
	return dmm->ui->btn_ch1_ac2->isChecked();
}

bool DMM_API::get_mode_ac_low_ch1() const {
	return dmm->ui->btn_ch1_ac->isChecked();
}

bool DMM_API::get_mode_ac_high_ch2() const {
	return dmm->ui->btn_ch2_ac2->isChecked();
}

bool DMM_API::get_mode_ac_low_ch2() const {
	return dmm->ui->btn_ch2_ac->isChecked();
}

void DMM_API::set_mode_ac_high_ch1(bool en) {
	/* Set DC mode as default */
	if (!get_mode_ac_low_ch1() && !en) {
		dmm->ui->btn_ch1_dc->setChecked(!en);
	}
	dmm->ui->btn_ch1_ac2->setChecked(en);
}

void DMM_API::set_mode_ac_low_ch1(bool en) {
	/* Set DC mode as default */
	if (!get_mode_ac_high_ch1() && !en) {
		dmm->ui->btn_ch1_dc->setChecked(!en);
	}
	dmm->ui->btn_ch1_ac->setChecked(en);
}

void DMM_API::set_mode_ac_high_ch2(bool en) {
	/* Set DC mode as default */
	if (!get_mode_ac_low_ch2() && !en) {
		dmm->ui->btn_ch2_dc->setChecked(!en);
	}
	dmm->ui->btn_ch2_ac2->setChecked(en);
}

void DMM_API::set_mode_ac_low_ch2(bool en) {
	/* Set DC mode as default */
	if (!get_mode_ac_high_ch2() && !en) {
		dmm->ui->btn_ch2_dc->setChecked(!en);
	}
	dmm->ui->btn_ch2_ac->setChecked(en);
}

bool DMM_API::running() const { return dmm->ui->run_button->isChecked(); }

void DMM_API::run(bool en) { dmm->ui->run_button->setChecked(en); }

double DMM_API::read_ch1() const { return dmm->ui->lcdCh1->value(); }

double DMM_API::read_ch2() const { return dmm->ui->lcdCh2->value(); }

bool DMM_API::get_histogram_ch1() const {
	return dmm->ui->histogramCh1->isChecked();
}

void DMM_API::set_histogram_ch1(bool en) {
	dmm->ui->histogramCh1->setChecked(en);
}

void DMM_API::set_histogram_ch2(bool en) {
	dmm->ui->histogramCh2->setChecked(en);
}

bool DMM_API::get_histogram_ch2() const {
	return dmm->ui->histogramCh2->isChecked();
}

int DMM_API::get_history_ch1_size_idx() const {
	return dmm->ui->historySizeCh1->currentIndex();
}

int DMM_API::get_history_ch2_size_idx() const {
	return dmm->ui->historySizeCh2->currentIndex();
}

void DMM_API::set_history_ch1_size_idx(int index) {
	dmm->ui->historySizeCh1->setCurrentIndex(index);
}

void DMM_API::set_history_ch2_size_idx(int index) {
	dmm->ui->historySizeCh2->setCurrentIndex(index);
}

bool DMM_API::getDataLoggingEn() const {
	return dmm->ui->btnDataLogging->isChecked();
}

void DMM_API::setDataLoggingEn(bool en) {
	dmm->ui->btnDataLogging->setChecked(en);
}

double DMM_API::getDataLoggingTimer() const {
	return dmm->data_logging_timer->value();
}

void DMM_API::setDataLoggingTimer(double val) {
	dmm->data_logging_timer->setValue(val);
}

bool DMM_API::getPeakHoldEn() const {
	return dmm->ui->btnDisplayPeakHold->isChecked();
}

void DMM_API::setPeakHoldEn(bool val) {
	dmm->ui->btnDisplayPeakHold->setChecked(val);
}

bool DMM_API::getDataLoggingAppend() const {
	return dmm->ui->btn_append->isChecked();
}

void DMM_API::setDataLoggingAppend(bool val) {
	dmm->ui->btn_append->setChecked(val);
	dmm->ui->btn_overwrite->setChecked(!val);
}

} // namespace adiscope
