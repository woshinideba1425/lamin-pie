#pragma once
#include "esp_log.h"
#include "constants.h"

enum class ProgressStage {
    COLLECTION,  // 数据收集阶段
    PROCESSING   // 数据处理阶段
};
class BP_task : public DataProvider{
public:
    BP_task(LAMINATEPIE::Framework* framework) : DataProvider(framework), inited(false), dataCounter(0) {}

    ~BP_task() {}
    void BPInitial();
    void Run();
    //void showProgress();
private:
    void BPdataFill();
    
    bool inited;
    int dataCounter;

    ProgressStage current_stage = ProgressStage::COLLECTION;  // 当前阶段，初始为数据收集
    int progress_counter = 0;  // 进度计数器
    const int process_stage_percentage = 90; // 数据收集阶段占90%
    const int total_percentage = 100;  // 总的百分比为100

};