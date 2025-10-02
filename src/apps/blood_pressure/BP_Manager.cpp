#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"


#include "model.h"

#include "output_handler.h"
#include "BP_Manager.h"

// #include <my_algorithm.h>
#include "esp_heap_caps.h"

namespace
{
  tflite::ErrorReporter *error_reporter = nullptr;
  const tflite::Model *model = nullptr;
  tflite::MicroInterpreter *interpreter = nullptr;

  TfLiteTensor *Signal_input = nullptr;
  TfLiteTensor *Static_input = nullptr;
  TfLiteTensor *output_SBP = nullptr;
  TfLiteTensor *output_DSP = nullptr;
  float *signal_input_data = nullptr;
  float *static_input_data = nullptr;
  int inference_count = 0;
  const int kTensorArenaSize = 140 * 1024;

} // namespace

extern bool signalFillStage;

void BP_task::BPInitial()
{

  uint8_t *tensor_arena = (uint8_t *)heap_caps_malloc(kTensorArenaSize, MALLOC_CAP_SPIRAM);

  if (tensor_arena == nullptr)
  {
    // 处理内存分配失败的情况
    LOGI("BP_task", "Failed to allocate tensor arena in PSRAM");
    // serial.sprintf("Failed to allocate tensor arena in PSRAM");
    return;
  }

  // 初始化解释器
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;
  model = tflite::GetModel(g_model);

  if (model->version() != TFLITE_SCHEMA_VERSION)
  {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  const tflite::SubGraph* subgraph = model->subgraphs()->Get(0);
  if (!subgraph) {
      printf("Subgraph not found in model.\n");
      return;
  }

  auto operators = subgraph->operators();



  // OPS初始化
  tflite::MicroMutableOpResolver<7> micro_op_resolver;
  micro_op_resolver.AddExpandDims();
  if (micro_op_resolver.AddFullyConnected() != kTfLiteOk) {
      LOGE("TFLITE", "Failed to add FullyConnected operation");
      return;
  }

  // 检查 Conv2D 操作添加是否成功
  if (micro_op_resolver.AddConv2D() != kTfLiteOk) {
      LOGE("TFLITE", "Failed to add Conv2D operation");
      return;
  }

  // 检查 MaxPool2D 操作添加是否成功
  if (micro_op_resolver.AddMaxPool2D() != kTfLiteOk) {
      LOGE("TFLITE", "Failed to add MaxPool2D operation");
      return;
  }

  // 检查 Reshape 操作添加是否成功
  if (micro_op_resolver.AddReshape() != kTfLiteOk) {
      LOGE("TFLITE", "Failed to add Reshape operation");
      return;
  }

  // 检查 Concatenation 操作添加是否成功
  if (micro_op_resolver.AddConcatenation() != kTfLiteOk) {
      LOGE("TFLITE", "Failed to add Concatenation operation");
      return;
  }

  // 检查 Softmax 操作添加是否成功
  if (micro_op_resolver.AddSoftmax() != kTfLiteOk) {
      LOGE("TFLITE", "Failed to add Softmax operation");
      return;
  }


  LOGI("TFLITE", "All operations added successfully");        
  // for (size_t i = 0; i < operators->size(); i++) {
  //     const tflite::Operator* op = operators->Get(i);
  //     tflite::BuiltinOperator op_code = model->operator_codes()->Get(op->opcode_index())->builtin_code();

  //     // 如果是自定义操作，可以通过 custom_code 获取名称
  //     const char* custom_name = nullptr;
  //     if (op_code == tflite::BuiltinOperator_CUSTOM) {
  //         custom_name = model->operator_codes()->Get(op->opcode_index())->custom_code()->c_str();
  //     }

  //     // 打印操作信息
  //     if (custom_name) {
  //         printf("Operator %zu: CUSTOM(%s)\n", i, custom_name);
  //     } else {
  //         printf("Operator %zu: %s\n", i, tflite::EnumNameBuiltinOperator(op_code));
  //     }
  // }
  // 创建解释器
  static tflite::MicroInterpreter static_interpreter(model, 
                                    micro_op_resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;


  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk)
  {
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
    return;
  }
  LOGI("BP_task", "AllocateTensors() successful");
}
void BP_task::Run()
{
  // 执行模量输入
  BPdataFill();

  // 执行模型推理
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk)
  {
    LOGE("BP_TASK", "Invoke failed");
    return;
  }

  // 从输出张量中获取推理结果
  float highPressure = output_DSP->data.f[0];
  float lowPressure = output_SBP->data.f[0];

  LOGI("BP_TASK", "Output - High Pressure: %.2f, Low Pressure: %.2f", highPressure, lowPressure);

    HandleOutput(error_reporter,highPressure,lowPressure);
  // 更新推理计数器
  inference_count++;
  LOGI("BP_TASK", "Inference count: %d", inference_count);
}

void BP_task::BPdataFill()
{
  Signal_input = interpreter->input(1);
  if ((Signal_input->dims->size != 3) || (Signal_input->dims->data[0] != 1) || (Signal_input->dims->data[1] != INPUTSAMPLE) || (Signal_input->dims->data[2] != 1))
  {
    LOGE("BP_TASK", "Bad input tensor parameters in model");
    return;
  }
  else
  {
    LOGI("BP_TASK", "Signal tensor correct");
  }

  Static_input = interpreter->input(0);
  if ((Static_input->dims->size != 2) || (Static_input->dims->data[0] != 1) || (Static_input->dims->data[1] != 2))
  {
    LOGE("BP_TASK", "Bad Static_input tensor parameters in model");
    return;
  }
  else
  {
    LOGI("BP_TASK", "Static tensor correct");
  }

  signal_input_data = Signal_input->data.f;
  static_input_data = Static_input->data.f;

  output_SBP = interpreter->output(0);
  output_DSP = interpreter->output(1);

  // 初始化推理计数器
  inference_count = 0;
  LOGI("BP_TASK", "Inference count initialized: %d", inference_count);

  // 传递信号数据

  LOGI("BP_TASK", "Filling signal data...");
  for (int i2 = 0; i2 < INPUTSAMPLE; ++i2)
  {
    getSignalData();
  }

  for (int j = 0; j < INPUTSAMPLE; ++j)
  {
    signal_input_data[j] = normalizeBPD[j];
    LOGI("BP_TASK", "Signal data[%d]: %.2f", j, signal_input_data[j]);
  }

  // 传递静态数据

  getStaticData();

  for (int i = 0; i < 2; ++i)
  {
    static_input_data[i] = staticData[i];
    LOGI("BP_TASK", "Static data[%d]: %.2f", i, static_input_data[i]);
  }

  LOGI("BP_TASK", "BPdataFill end");
}
