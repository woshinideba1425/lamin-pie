#ifndef TENSORFLOW_LITE_MICRO_EXAMPLES_HELLO_WORLD_CONSTANTS_H_
#define TENSORFLOW_LITE_MICRO_EXAMPLES_HELLO_WORLD_CONSTANTS_H_
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "model.h" 
#include "framwork.h"
#include <ArduinoJson.h>

#define INPUTSAMPLE 700

class DataProvider {
 public:
      explicit DataProvider(LAMINATEPIE::Framework* framework)
       : _framework(framework), ir_avg_reg1(0), avgdc(0), minD(0), maxD(0), bpindex(0), _height(0), _weight(0) {
         drive = &_framework->getHAL();
       }
   
   
   float normalizeBPD[700];
   float staticData[2];

   void BpData(float *n, int *m, HAL *drive);

   void getSignalData();
   void getStaticData();
   void putStaticData(JsonDocument userdata);

 private:
   LAMINATEPIE::Framework* _framework;
   std::string Dstatic;
   int32_t ir_avg_reg1;
   int16_t avgdc;
   HAL *drive;
   int minD;
   int maxD;
   int bpindex;
   int BPD[700];
   int _height;
   int _weight;
};

#endif  // TENSORFLOW_LITE_MICRO_EXAMPLES_HELLO_WORLD_CONSTANTS_H_
