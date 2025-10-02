/*本文档用来进行数据的预处理*/
// #include <my_algorithm.h>
#include "Heartrate.h"
#include "constants.h"
#include "algorithm/include/Common.h"


/// @brief  为模型的输入提供基本的信号数据
/// @param n  用于保存预处理完之后的数组
/// @param m  用于保存采集完后的整数数据
void DataProvider::BpData(float* n, int* m, HAL *drive)
{
  int16_t Current = 0;
  int16_t y = 0;
  
  uint32_t irValue = drive->max30105.getIR();  //  获取红外信号值

  avgdc = averageDCEstimator(&ir_avg_reg1, irValue); //  去直流
  Current = lowPassFIRFilter(irValue - avgdc);   // 滤波

  y = trendDelet(Current);  //  去趋势化

  m[bpindex++] = y;
  //Serial.print("trendDelet value");
  //Serial.println(y);
  printf("irValue raw value:%d\n",irValue);
  if(bpindex >= INPUTSAMPLE){
    bpindex = 0;
    findMinMax(m,INPUTSAMPLE,minD,maxD);
    for(int i3 =0; i3<INPUTSAMPLE; i3++)
    {
      float t = normalize_signal(m[i3],minD,maxD); //  归一化
      //Serial.print("normalized point:");
      //Serial.println(t);
      n[i3] = t;
    }
  }
}

void DataProvider::getSignalData()
{  
  BpData(normalizeBPD,BPD,drive);
}

void DataProvider::getStaticData()
{   
    staticData[0] = _height;staticData[1] = _weight;
}

void DataProvider::putStaticData(JsonDocument userdata)
{
  _height = userdata["height"];
  _weight = userdata["weight"];
  LOGI("BP_TASK", "height:%d, weight:%d", _height, _weight);
}
