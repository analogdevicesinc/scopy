#pragma once

#include "decoder/IAnnotationExtractor.h"
#include "decoder/IProtocolDataEncoder.h"

#include <memory>

namespace scopy {
namespace decoder {
namespace codecs {

std::unique_ptr<IAnnotationExtractor> makeUartExtractor();
std::unique_ptr<IAnnotationExtractor> makeSpiExtractor();
std::unique_ptr<IAnnotationExtractor> makeI2cExtractor();

std::unique_ptr<IProtocolDataEncoder> makeUartEncoder();
std::unique_ptr<IProtocolDataEncoder> makeSpiEncoder();
std::unique_ptr<IProtocolDataEncoder> makeI2cEncoder();

} // namespace codecs
} // namespace decoder
} // namespace scopy
