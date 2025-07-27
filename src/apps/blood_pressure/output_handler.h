#pragma once

#include "ui.h"
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"

void HandleOutput(tflite::ErrorReporter* error_reporter, float highPressure, float lowPressure);