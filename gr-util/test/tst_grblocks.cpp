#include "gr-util/grproxyblock.h"
#include "gr-util/grsignalpath.h"
#include "gr-util/grtopblock.h"

#include <gnuradio/blocks/head.h>
#include <gnuradio/blocks/stream_to_vector.h>
#include <gnuradio/blocks/vector_sink.h>

#include <QSignalSpy>
#include <QTest>
#include <QVector>

#include <gr-util/griiocomplexchannelsrc.h>
#include <gr-util/griiodevicesource.h>
#include <gr-util/griiofloatchannelsrc.h>
#include <gr-util/grproxyblock.h>
#include <gr-util/grscaleoffsetproc.h>
#include <gr-util/grsignalpath.h>
#include <gr-util/grsignalsrc.h>
#include <gr-util/grtopblock.h>

using namespace scopy::grutil;

class TST_GRBlocks : public QObject
{
	Q_OBJECT
private Q_SLOTS: // these are tests
	void test1();
	void test2();
	void test3();
	void test4();
	void test5();

public Q_SLOTS:			   // these are actual slots
	void connectVectorSinks(); // return vec sinks
	QVector<float> computeSigSourceExpected(gr::analog::gr_waveform_t wave, float ampl, float offset, float sr,
						float freq, float scale_1, float offset_1);

private:
	void connectVectorSinks(GRTopBlock *top); // return vec sinks
	struct test1Params
	{
		const int nr_samples = 100;
		const float sig_ampl = 2;
		const float sig_offset = 0;
		const float sig_sr = 100;  // only select an integer multiple of frequency here for testing
		const float sig_freq = 10; //
		const float offset_1 = 2;
		const float scale_1 = 1;

		const float offset_2 = 1;
		const float scale_2 = 2;

	} t1;

	std::vector<gr::blocks::vector_sink_f::sptr> testOutputs;
	std::vector<gr::blocks::vector_sink_c::sptr> testOutputs_c;
};

void TST_GRBlocks::connectVectorSinks()
{
	GRTopBlock *sender = dynamic_cast<GRTopBlock *>(QObject::sender());
	connectVectorSinks(sender);
}

void TST_GRBlocks::connectVectorSinks(GRTopBlock *top)
{

	testOutputs.clear();
	gr::blocks::head::sptr head;
	gr::blocks::stream_to_vector::sptr s2v;
	gr::blocks::vector_sink_f::sptr vec;
	gr::blocks::vector_sink_c::sptr vec_c;

	std::vector<gr::blocks::vector_sink_f::sptr> ret;

	for(GRSignalPath *path : top->signalPaths()) {
		if(!path->enabled())
			continue;
		gr::basic_block_sptr endpoint = path->getGrEndPoint();

		int size = endpoint->output_signature()->sizeof_stream_item(0);
		if(size == sizeof(float)) {
			head = gr::blocks::head::make(size, t1.nr_samples * 2 - 1);
			s2v = gr::blocks::stream_to_vector::make(size, t1.nr_samples);

			vec = gr::blocks::vector_sink_f::make(t1.nr_samples);

			top->connect(endpoint, 0, head, 0);
			top->connect(head, 0, s2v, 0);
			top->connect(s2v, 0, vec, 0);
			testOutputs.push_back(vec);
		} else if(size == sizeof(gr_complex)) {
			head = gr::blocks::head::make(size, t1.nr_samples * 2 - 1);
			s2v = gr::blocks::stream_to_vector::make(size, t1.nr_samples);
			vec_c = gr::blocks::vector_sink_c::make(t1.nr_samples);

			top->connect(endpoint, 0, head, 0);
			top->connect(head, 0, s2v, 0);
			top->connect(s2v, 0, vec_c, 0);
			testOutputs_c.push_back(vec_c);
		}
	}
}

QVector<float> TST_GRBlocks::computeSigSourceExpected(gr::analog::gr_waveform_t wave, float ampl, float offset,
						      float sr, float freq, float scale_1, float offset_1)
{
	QVector<float> expected;
	auto period = int(t1.sig_sr / t1.sig_freq);
	auto pol_change = period / 2;

	for(int i = 0; i < t1.nr_samples; i++) {
		switch(wave) {
		case gr::analog::GR_CONST_WAVE:
			expected.push_back((ampl + offset) * scale_1 + offset_1);
			break;
		case gr::analog::GR_SQR_WAVE:
			if((i % period) < pol_change)
				expected.push_back(offset * scale_1 + offset_1);
			else
				expected.push_back((ampl + offset) * scale_1 + offset_1);
			break;
		default:
			qWarning() << "Waveform NOT SUPPORTED";
			break;
		}
	}
	return expected;
}

void TST_GRBlocks::test1()
{

	qInfo() << "This tests the basic functionality of GRBlocks ";
	qInfo() << "We create a signal path with a simple source, scale and offset, run it, and verify results";
	qInfo() << "We then destroy the top block, disable the scale and offset, and rerun the flowgraph";

	GRTopBlock top("aa", this);
	GRSignalPath *ch1;
	GRSignalSrc *sin1;
	GRScaleOffsetProc *scale_offset;

	ch1 = new GRSignalPath("iio1", &top);
	top.registerSignalPath(ch1);

	sin1 = new GRSignalSrc(ch1);
	scale_offset = new GRScaleOffsetProc(ch1);
	sin1->setWaveform(gr::analog::GR_CONST_WAVE);
	sin1->setSamplingFreq(t1.sig_sr);
	sin1->setAmplitude(t1.sig_ampl);
	sin1->setOffset(t1.sig_offset);

	scale_offset->setScale(t1.scale_1);

	ch1->append(sin1);
	ch1->append(scale_offset);
	top.build();
	qInfo() << "built flowgraph";
	{
		connectVectorSinks(&top);
		sin1->setFreq(t1.sig_freq); // change parameters after building
		scale_offset->setOffset(t1.offset_1);
		qInfo() << "modified scale_offset after build";
		top.getGrBlock()->run();

		// |sig_source| --> |multiply| --> |add| --> |head| --> |stream_to_vector| --> |vector_sink|
		QCOMPARE(QString::fromStdString(top.getGrBlock()->edge_list()),
			 QString("multiply_const_ff0:0->add_const_ff0:0\n" // Build order matters for edge_list
				 "sig_source0:0->multiply_const_ff0:0\n"
				 "add_const_ff0:0->head0:0\n"
				 "head0:0->stream_to_vector0:0\n"
				 "stream_to_vector0:0->vector_sink0:0\n"));

		QVector<float> expected =
			computeSigSourceExpected(gr::analog::GR_CONST_WAVE, t1.sig_ampl, t1.sig_offset, t1.sig_sr,
						 t1.sig_freq, t1.scale_1, t1.offset_1);
		std::vector<float> data = testOutputs[0]->data();
		QVector<float> res = QVector<float>(data.begin(), data.end());
		QCOMPARE(res, expected);
	}

	top.teardown();
	scale_offset->setEnabled(false); // disabling requires rebuild - should this be handled internally (?)
	qInfo() << "disabled block in signal path";
	top.build();
	{
		connectVectorSinks(&top);
		top.getGrBlock()->run();
		// |sig_source| --> |head| --> |stream_to_vector| --> |vector_sink|
		QCOMPARE(QString::fromStdString(top.getGrBlock()->edge_list()),
			 QString("sig_source1:0->head1:0\n"
				 "head1:0->stream_to_vector1:0\n"
				 "stream_to_vector1:0->vector_sink1:0\n"));

		QVector<float> expected = computeSigSourceExpected(gr::analog::GR_CONST_WAVE, t1.sig_ampl,
								   t1.sig_offset, t1.sig_sr, t1.sig_freq, 1, 0);
		std::vector<float> data = testOutputs[0]->data();
		QVector<float> res = QVector<float>(data.begin(), data.end());
		qDebug() << res;

		qDebug() << expected;
		QCOMPARE(res, expected);
	}
}

void TST_GRBlocks::test2()
{
	qInfo() << "This testcase verifies if one can use a signal path as a source for a different signal path";

	GRTopBlock top("aa", this);
	GRSignalPath *ch1;
	GRSignalPath *ch2;
	GRSignalSrc *sin1;
	GRScaleOffsetProc *scale_offset;

	ch1 = new GRSignalPath("iio1", &top);
	sin1 = new GRSignalSrc(ch1); // do not register this one

	ch2 = new GRSignalPath("iio2", &top);
	top.registerSignalPath(ch2);
	scale_offset = new GRScaleOffsetProc(ch2);

	sin1->setWaveform(gr::analog::GR_CONST_WAVE);
	sin1->setSamplingFreq(t1.sig_sr);
	sin1->setAmplitude(t1.sig_ampl);
	sin1->setFreq(t1.sig_freq);
	scale_offset->setOffset(t1.offset_1);
	scale_offset->setScale(t1.scale_1);

	ch1->append(sin1);
	ch2->append(ch1);
	ch2->append(scale_offset);

	top.build();
	qInfo() << "built flowgraph";
	{
		connectVectorSinks(&top);

		top.getGrBlock()->run();

		QCOMPARE(QString::fromStdString(top.getGrBlock()->edge_list()),
			 QString("multiply_const_ff0:0->add_const_ff0:0\n"
				 "sig_source0:0->multiply_const_ff0:0\n"
				 "add_const_ff0:0->head0:0\n"
				 "head0:0->stream_to_vector0:0\n"
				 "stream_to_vector0:0->vector_sink0:0\n"));

		QVector<float> expected =
			computeSigSourceExpected(gr::analog::GR_CONST_WAVE, t1.sig_ampl, t1.sig_offset, t1.sig_sr,
						 t1.sig_freq, t1.scale_1, t1.offset_1);

		std::vector<float> data = testOutputs[0]->data();
		QVector<float> res = QVector<float>(data.begin(), data.end());

		QCOMPARE(res, expected);
	}
}

void TST_GRBlocks::test3()
{

	qInfo() << "This testcase verifies if multiple signalpaths work for the same topblock";
	GRTopBlock top("aa", this);
	GRSignalPath *ch1, *ch2, *ch3;
	GRSignalSrc *sin1, *sin2;
	GRScaleOffsetProc *scale_offset_1;
	GRScaleOffsetProc *scale_offset_2;

	ch1 = new GRSignalPath("iio1", &top);
	ch2 = new GRSignalPath("iio2", &top);
	ch3 = new GRSignalPath("iio3", &top);
	top.registerSignalPath(ch1);
	top.registerSignalPath(ch2);
	top.registerSignalPath(ch3);

	sin1 = new GRSignalSrc(ch1);
	sin2 = new GRSignalSrc(ch2);

	scale_offset_1 = new GRScaleOffsetProc(ch2);
	scale_offset_2 = new GRScaleOffsetProc(ch3);

	sin1->setWaveform(gr::analog::GR_CONST_WAVE);
	sin1->setSamplingFreq(t1.sig_sr);
	sin1->setAmplitude(t1.sig_ampl);
	sin1->setFreq(t1.sig_freq);

	sin2->setWaveform(gr::analog::GR_SQR_WAVE);
	sin2->setSamplingFreq(t1.sig_sr);
	sin2->setAmplitude(t1.sig_ampl);
	sin2->setFreq(t1.sig_freq);

	scale_offset_1->setOffset(t1.offset_1);
	scale_offset_1->setScale(t1.scale_1);

	scale_offset_2->setOffset(t1.offset_2);
	scale_offset_2->setScale(t1.scale_2);

	/*   |sin1| --+------------------------- - ch1
	 *            +---|scale_offset_1|------ - ch2
	 *   |sin2| ------|scale_offset_2|------ - ch3
	 */
	ch1->append(sin1);
	ch2->append(ch1);
	ch2->append(scale_offset_1);
	ch3->append(sin2);
	ch3->append(scale_offset_2);

	top.build();
	qInfo() << "built flowgraph";
	{
		connectVectorSinks(&top);
		top.getGrBlock()->run();

		//		qDebug()<<QString::fromStdString(top.getGrBlock()->edge_list());
		QCOMPARE(QString::fromStdString(top.getGrBlock()->edge_list()),
			 QString("multiply_const_ff0:0->add_const_ff0:0\n"
				 "sig_source0:0->multiply_const_ff0:0\n"
				 "multiply_const_ff1:0->add_const_ff1:0\n"
				 "sig_source1:0->multiply_const_ff1:0\n"
				 "sig_source0:0->head0:0\n"
				 "head0:0->stream_to_vector0:0\n"
				 "stream_to_vector0:0->vector_sink0:0\n"
				 "add_const_ff0:0->head1:0\n"
				 "head1:0->stream_to_vector1:0\n"
				 "stream_to_vector1:0->vector_sink1:0\n"
				 "add_const_ff1:0->head2:0\n"
				 "head2:0->stream_to_vector2:0\n"
				 "stream_to_vector2:0->vector_sink2:0\n"));

		QVector<QVector<float>> expectedAll;
		// constant no scale / offset block
		expectedAll.push_back(computeSigSourceExpected(gr::analog::GR_CONST_WAVE, t1.sig_ampl, t1.sig_offset,
							       t1.sig_sr, t1.sig_freq, 1, 0));
		// constant with scale and offset
		expectedAll.push_back(computeSigSourceExpected(gr::analog::GR_CONST_WAVE, t1.sig_ampl, t1.sig_offset,
							       t1.sig_sr, t1.sig_freq, t1.scale_1, t1.offset_1));
		// third channel square wave
		expectedAll.push_back(computeSigSourceExpected(gr::analog::GR_SQR_WAVE, t1.sig_ampl, t1.sig_offset,
							       t1.sig_sr, t1.sig_freq, t1.scale_2, t1.offset_2));

		for(int i = 0; i < testOutputs.size(); i++) {
			std::vector<float> data = testOutputs[i]->data();
			QVector<float> res = QVector<float>(data.begin(), data.end());
			qDebug() << expectedAll[i];
			qDebug() << testOutputs[i]->data();
			QCOMPARE(res, expectedAll[i]);
		}
	}
	top.teardown();
	ch2->setEnabled(false);
	scale_offset_2->setEnabled(false);
	top.build();

	{
		connectVectorSinks(&top);
		top.getGrBlock()->run();
		QCOMPARE(QString::fromStdString(top.getGrBlock()->edge_list()),
			 QString("sig_source2:0->head3:0\n"
				 "head3:0->stream_to_vector3:0\n"
				 "stream_to_vector3:0->vector_sink3:0\n"
				 "sig_source3:0->head4:0\n"
				 "head4:0->stream_to_vector4:0\n"
				 "stream_to_vector4:0->vector_sink4:0\n"));

		QVector<QVector<float>> expectedAll;

		// constant no scale / offset block
		expectedAll.push_back(computeSigSourceExpected(gr::analog::GR_CONST_WAVE, t1.sig_ampl, t1.sig_offset,
							       t1.sig_sr, t1.sig_freq, 1, 0));
		// constant with scale and offset
		expectedAll.push_back(computeSigSourceExpected(gr::analog::GR_SQR_WAVE, t1.sig_ampl, t1.sig_offset,
							       t1.sig_sr, t1.sig_freq, 1, 0));

		for(int i = 0; i < testOutputs.size(); i++) {

			std::vector<float> data = testOutputs[i]->data();
			QVector<float> res = QVector<float>(data.begin(), data.end());

			qDebug() << expectedAll[i];
			qDebug() << testOutputs[i]->data();

			QCOMPARE(res, expectedAll[i]);
		}
	}

	top.teardown();

	qInfo() << "This test verifies that a source can be accessed indirectly";

	ch1->setEnabled(false); // disable first signal path
	ch2->setEnabled(true);	// second signal path should use the first source indirectly

	scale_offset_2->setEnabled(false);
	top.build();

	{
		connectVectorSinks(&top);

		top.getGrBlock()->run();
		qDebug() << QString::fromStdString(top.getGrBlock()->edge_list());
		QCOMPARE(QString::fromStdString(top.getGrBlock()->edge_list()),
			 QString("multiply_const_ff0:0->add_const_ff0:0\n"
				 "sig_source0:0->multiply_const_ff0:0\n"
				 "add_const_ff0:0->head0:0\n"
				 "head0:0->stream_to_vector0:0\n"
				 "stream_to_vector0:0->vector_sink0:0\n"
				 "sig_source1:0->head1:0\n"
				 "head1:0->stream_to_vector1:0\n"
				 "stream_to_vector1:0->vector_sink1:0\n"));

		QVector<QVector<float>> expectedAll;

		// constant no scale / offset block
		expectedAll.push_back(computeSigSourceExpected(gr::analog::GR_CONST_WAVE, t1.sig_ampl, t1.sig_offset,
							       t1.sig_sr, t1.sig_freq, t1.scale_1, t1.offset_1));
		// constant with scale and offset
		expectedAll.push_back(computeSigSourceExpected(gr::analog::GR_SQR_WAVE, t1.sig_ampl, t1.sig_offset,
							       t1.sig_sr, t1.sig_freq, 1, 0));

		for(int i = 0; i < testOutputs.size(); i++) {

			std::vector<float> data = testOutputs[i]->data();
			QVector<float> res = QVector<float>(data.begin(), data.end());

			qDebug() << expectedAll[i];
			qDebug() << testOutputs[i]->data();

			QCOMPARE(res, expectedAll[i]);
		}
	}
}

void TST_GRBlocks::test4()
{

	qInfo() << "This testcase verifies signal emission on building/teardown and how it can be leveraged to build "
		   "sinks on demand";
	GRTopBlock top("aa", this);
	GRSignalPath *ch1, *ch2, *ch3;
	GRSignalSrc *sin1, *sin2;
	GRScaleOffsetProc *scale_offset_1;
	GRScaleOffsetProc *scale_offset_2;

	ch1 = new GRSignalPath("iio1", &top);
	top.registerSignalPath(ch1);
	ch2 = new GRSignalPath("iio2", &top);
	top.registerSignalPath(ch2);
	ch3 = new GRSignalPath("iio3", &top);
	top.registerSignalPath(ch3);

	sin1 = new GRSignalSrc(ch1);
	sin2 = new GRSignalSrc(ch2);

	scale_offset_1 = new GRScaleOffsetProc(ch2);
	scale_offset_2 = new GRScaleOffsetProc(ch3);

	sin1->setWaveform(gr::analog::GR_CONST_WAVE);
	sin1->setSamplingFreq(t1.sig_sr);
	sin1->setAmplitude(t1.sig_ampl);
	sin1->setFreq(t1.sig_freq);

	sin2->setWaveform(gr::analog::GR_SQR_WAVE);
	sin2->setSamplingFreq(t1.sig_sr);
	sin2->setAmplitude(t1.sig_ampl);
	sin2->setFreq(t1.sig_freq);

	scale_offset_1->setOffset(t1.offset_1);
	scale_offset_1->setScale(t1.scale_1);

	scale_offset_2->setOffset(t1.offset_2);
	scale_offset_2->setScale(t1.scale_2);

	/*   |sin1| --+------------------------- - ch1
	 *            +---|scale_offset_1|------ - ch2
	 *   |sin2| ------|scale_offset_2|------ - ch3
	 */
	ch1->append(sin1);
	ch2->append(ch1);
	ch2->append(scale_offset_1);
	ch3->append(sin2);
	ch3->append(scale_offset_2);

	connect(&top, SIGNAL(builtSignalPaths()), this, SLOT(connectVectorSinks()));
	top.build();
	top.start();

	{
		top.getGrBlock()->wait(); // for testing purposes

		qDebug() << QString::fromStdString(top.getGrBlock()->edge_list());
		QCOMPARE(QString::fromStdString(top.getGrBlock()->edge_list()),
			 QString("multiply_const_ff0:0->add_const_ff0:0\n"
				 "sig_source0:0->multiply_const_ff0:0\n"
				 "multiply_const_ff1:0->add_const_ff1:0\n"
				 "sig_source1:0->multiply_const_ff1:0\n"
				 "sig_source0:0->head0:0\n"
				 "head0:0->stream_to_vector0:0\n"
				 "stream_to_vector0:0->vector_sink0:0\n"
				 "add_const_ff0:0->head1:0\n"
				 "head1:0->stream_to_vector1:0\n"
				 "stream_to_vector1:0->vector_sink1:0\n"
				 "add_const_ff1:0->head2:0\n"
				 "head2:0->stream_to_vector2:0\n"
				 "stream_to_vector2:0->vector_sink2:0\n"));

		QVector<QVector<float>> expectedAll;
		// constant no scale / offset block
		expectedAll.push_back(computeSigSourceExpected(gr::analog::GR_CONST_WAVE, t1.sig_ampl, t1.sig_offset,
							       t1.sig_sr, t1.sig_freq, 1, 0));
		// constant with scale and offset
		expectedAll.push_back(computeSigSourceExpected(gr::analog::GR_CONST_WAVE, t1.sig_ampl, t1.sig_offset,
							       t1.sig_sr, t1.sig_freq, t1.scale_1, t1.offset_1));
		// third channel square wave
		expectedAll.push_back(computeSigSourceExpected(gr::analog::GR_SQR_WAVE, t1.sig_ampl, t1.sig_offset,
							       t1.sig_sr, t1.sig_freq, t1.scale_2, t1.offset_2));

		for(int i = 0; i < testOutputs.size(); i++) {
			std::vector<float> data = testOutputs[i]->data();
			QVector<float> res = QVector<float>(data.begin(), data.end());
			qDebug() << expectedAll[i];
			qDebug() << testOutputs[i]->data();
			QCOMPARE(res, expectedAll[i]);
		}
	}

	QSignalSpy spy(&top, SIGNAL(builtSignalPaths()));
	ch2->setEnabled(false);
	scale_offset_2->setEnabled(false);
	QCOMPARE(spy.count(), 2); // flowgraph rebuilt twice
	{
		top.getGrBlock()->wait(); // for testing purposes
		QCOMPARE(QString::fromStdString(top.getGrBlock()->edge_list()),
			 QString("sig_source0:0->head0:0\n"
				 "head0:0->stream_to_vector0:0\n"
				 "stream_to_vector0:0->vector_sink0:0\n"
				 "sig_source1:0->head1:0\n"
				 "head1:0->stream_to_vector1:0\n"
				 "stream_to_vector1:0->vector_sink1:0\n"));

		QVector<QVector<float>> expectedAll;

		// constant no scale / offset block
		expectedAll.push_back(computeSigSourceExpected(gr::analog::GR_CONST_WAVE, t1.sig_ampl, t1.sig_offset,
							       t1.sig_sr, t1.sig_freq, 1, 0));
		// constant with scale and offset
		expectedAll.push_back(computeSigSourceExpected(gr::analog::GR_SQR_WAVE, t1.sig_ampl, t1.sig_offset,
							       t1.sig_sr, t1.sig_freq, 1, 0));

		for(int i = 0; i < testOutputs.size(); i++) {

			std::vector<float> data = testOutputs[i]->data();
			QVector<float> res = QVector<float>(data.begin(), data.end());

			qDebug() << expectedAll[i];
			qDebug() << testOutputs[i]->data();

			QCOMPARE(res, expectedAll[i]);
		}
	}
	top.stop();
	top.teardown();
	QSignalSpy spy2(&top, SIGNAL(builtSignalPaths()));
	ch2->setEnabled(true);
	scale_offset_2->setEnabled(true);
	QCOMPARE(spy2.count(), 0); // flowgraph is not rebuilt because it was not built
	top.build();
	top.start();
	QCOMPARE(spy2.count(), 1); // built only once

	{
		top.getGrBlock()->wait(); // for testing purposes

		qDebug() << QString::fromStdString(top.getGrBlock()->edge_list());
		QCOMPARE(QString::fromStdString(top.getGrBlock()->edge_list()),
			 QString("multiply_const_ff0:0->add_const_ff0:0\n"
				 "sig_source0:0->multiply_const_ff0:0\n"
				 "multiply_const_ff1:0->add_const_ff1:0\n"
				 "sig_source1:0->multiply_const_ff1:0\n"
				 "sig_source0:0->head0:0\n"
				 "head0:0->stream_to_vector0:0\n"
				 "stream_to_vector0:0->vector_sink0:0\n"
				 "add_const_ff0:0->head1:0\n"
				 "head1:0->stream_to_vector1:0\n"
				 "stream_to_vector1:0->vector_sink1:0\n"
				 "add_const_ff1:0->head2:0\n"
				 "head2:0->stream_to_vector2:0\n"
				 "stream_to_vector2:0->vector_sink2:0\n"));

		QVector<QVector<float>> expectedAll;
		// constant no scale / offset block
		expectedAll.push_back(computeSigSourceExpected(gr::analog::GR_CONST_WAVE, t1.sig_ampl, t1.sig_offset,
							       t1.sig_sr, t1.sig_freq, 1, 0));
		// constant with scale and offset
		expectedAll.push_back(computeSigSourceExpected(gr::analog::GR_CONST_WAVE, t1.sig_ampl, t1.sig_offset,
							       t1.sig_sr, t1.sig_freq, t1.scale_1, t1.offset_1));
		// third channel square wave
		expectedAll.push_back(computeSigSourceExpected(gr::analog::GR_SQR_WAVE, t1.sig_ampl, t1.sig_offset,
							       t1.sig_sr, t1.sig_freq, t1.scale_2, t1.offset_2));

		for(int i = 0; i < testOutputs.size(); i++) {
			std::vector<float> data = testOutputs[i]->data();
			QVector<float> res = QVector<float>(data.begin(), data.end());
			qDebug() << expectedAll[i];
			qDebug() << testOutputs[i]->data();
			QCOMPARE(res, expectedAll[i]);
		}
	}
}

void TST_GRBlocks::test5()
{

	qInfo() << "This testcase verifies iio-source";
	GRTopBlock top("aa", this);
	GRSignalPath *ch1, *ch2;
	GRSignalPath *ch3;
	GRIIOFloatChannelSrc *fch1;
	GRIIOFloatChannelSrc *fch2;
	GRIIOComplexChannelSrc *cch1;
	GRIIODeviceSource *dev;
	GRScaleOffsetProc *scale_offset_1;
	GRScaleOffsetProc *scale_offset_2;

	ch1 = new GRSignalPath("iio1", &top);
	top.registerSignalPath(ch1);
	ch2 = new GRSignalPath("iio2", &top);
	top.registerSignalPath(ch2);
	ch3 = new GRSignalPath("complex", &top);

	iio_context *ctx = iio_create_context_from_uri("ip:192.168.2.1");
	if(!ctx) {
		QSKIP("No context. Skipping");
	}

	dev = new GRIIODeviceSource(ctx, "cf-ad9361-lpc", "ad9361-phy", 0x400, &top);
	if(!dev) {
		QSKIP("No pluto. Skipping");
	}

	top.registerIIODeviceSource(dev);

	fch1 = new GRIIOFloatChannelSrc(dev, "voltage0", ch1);
	fch2 = new GRIIOFloatChannelSrc(dev, "voltage1", ch2);
	cch1 = new GRIIOComplexChannelSrc("complex_voltage0/1", dev, "voltage0", "voltage1", ch3);

	ch1->append(fch1);
	ch2->append(fch2);
	ch3->append(cch1);

	connect(&top, SIGNAL(builtSignalPaths()), this, SLOT(connectVectorSinks()));
	top.build();
	top.start();

	{ // create iio-source (two channel)

		top.getGrBlock()->wait(); // for testing purposes
		qDebug() << QString::fromStdString(top.getGrBlock()->edge_list());
		QCOMPARE(QString::fromStdString(top.getGrBlock()->edge_list()),
			 QString("device_source0:0->short_to_float0:0\n"
				 "device_source0:1->short_to_float1:0\n"
				 "short_to_float0:0->head0:0\n"
				 "head0:0->stream_to_vector0:0\n"
				 "stream_to_vector0:0->vector_sink0:0\n"
				 "short_to_float1:0->head1:0\n"
				 "head1:0->stream_to_vector1:0\n"
				 "stream_to_vector1:0->vector_sink1:0\n"));

		std::vector<std::string> expectedChannel;
		expectedChannel.push_back("voltage0");
		expectedChannel.push_back("voltage1");
		QCOMPARE(expectedChannel, dev->channelNames());
		qDebug() << testOutputs[0]->data();
	}
	ch1->setEnabled(false);

	{ // create iio-source (one channel)

		top.getGrBlock()->wait(); // for testing purposes
		qDebug() << QString::fromStdString(top.getGrBlock()->edge_list());
		QCOMPARE(QString::fromStdString(top.getGrBlock()->edge_list()),
			 QString("device_source0:0->short_to_float0:0\n"
				 "short_to_float0:0->head0:0\n"
				 "head0:0->stream_to_vector0:0\n"
				 "stream_to_vector0:0->vector_sink0:0\n"));
		std::vector<std::string> expectedChannel;
		expectedChannel.push_back("voltage1");
		QCOMPARE(expectedChannel, dev->channelNames());

		qDebug() << testOutputs[0]->data();
	}
	top.stop();
	top.teardown();
	ch1->setEnabled(true);
	ch3->setEnabled(false);

	top.unregisterSignalPath(ch1);
	top.unregisterSignalPath(ch2);
	top.registerSignalPath(ch2);
	top.registerSignalPath(ch1);
	top.registerSignalPath(ch3);

	top.build();
	top.start();

	{ // create iio-source (two channel - inverted)

		top.getGrBlock()->wait(); // for testing purposes
		qDebug() << QString::fromStdString(top.getGrBlock()->edge_list());
		QCOMPARE(QString::fromStdString(top.getGrBlock()->edge_list()),
			 QString("device_source0:1->short_to_float0:0\n"
				 "device_source0:0->short_to_float1:0\n" /* inversion occurs here */
				 "short_to_float0:0->head0:0\n"
				 "head0:0->stream_to_vector0:0\n"
				 "stream_to_vector0:0->vector_sink0:0\n"
				 "short_to_float1:0->head1:0\n"
				 "head1:0->stream_to_vector1:0\n"
				 "stream_to_vector1:0->vector_sink1:0\n"));

		std::vector<std::string> expectedChannel;
		expectedChannel.push_back("voltage0");
		expectedChannel.push_back("voltage1");
		QCOMPARE(expectedChannel, dev->channelNames());
		qDebug() << testOutputs[0]->data();
	}

	top.stop();
	top.teardown();
	ch1->setEnabled(false);
	ch3->setEnabled(true);

	top.build();
	top.start();

	{ // create iio-source (one channel + complex)

		top.getGrBlock()->wait(); // for testing purposes
		qDebug() << QString::fromStdString(top.getGrBlock()->edge_list());
		QCOMPARE(QString::fromStdString(top.getGrBlock()->edge_list()),
			 QString("short_to_float1:0->float_to_complex0:0\n"
				 "short_to_float2:0->float_to_complex0:1\n"
				 "device_source0:1->short_to_float0:0\n"
				 "device_source0:0->short_to_float1:0\n"
				 "device_source0:1->short_to_float2:0\n"
				 "short_to_float0:0->head0:0\n"
				 "head0:0->stream_to_vector0:0\n"
				 "stream_to_vector0:0->vector_sink0:0\n"
				 "float_to_complex0:0->head1:0\n"
				 "head1:0->stream_to_vector1:0\n"
				 "stream_to_vector1:0->vector_sink1:0\n"));

		std::vector<std::string> expectedChannel;
		expectedChannel.push_back("voltage0");
		expectedChannel.push_back("voltage1");
		QCOMPARE(expectedChannel, dev->channelNames());
		qDebug() << testOutputs[0]->data();
	}
}

// tests:
// figure out lifecycle for build/connect/disconnect/teardown - just getEndPoint - and build if required - all goes
// recursively (?) - QoL change - not necessary rn

// add more blocks (?)
// - dc blocker
// - soft trigger
// - head

// add math-test
// add audio-test
// add file-test

QTEST_MAIN(TST_GRBlocks)
// int main() {

//}

#include "tst_grblocks.moc"
