#include "infrared_tm.h"
#include "algorithm/include/Common.h"

#define INTERPOLATE 1
#define EMMISIVITY 0.95
#define TA_SHIFT 8  //Default shift for MLX90640 in open air
#define O_WIDTH 320
#define O_HEIGHT 240
#define O_RATIO O_WIDTH/32

namespace LAMINATEPIE
{
    namespace BUILTIN_APP
    {
        void InfraredApp::onSetup()
        {
            setTaskName("InfraredApp");
            setScreen(ui_infrared_screen);
            setAllowBgRunning(false);
            setStack( 4 * 1024);
            setAppIcon((void *)&ICON_INFRARED);
        }

        void InfraredApp::onCreate()
        {
            printf("[%s] onCreate\n", getAppName().c_str());
            int status;
            uint16_t *eeMLX90640;
            update = &_framework->getHAL();
            check_ex_device();
            m640 = &update->infrared;

            eeMLX90640 = static_cast<uint16_t*>(heap_caps_malloc(sizeof(uint16_t) * 832,MALLOC_CAP_SPIRAM));
            status = m640->MLX90640_DumpEE(m640->get_i2c_addr(), eeMLX90640);
            if (status != 0) ESP_LOGW(TAG,"Failed to load system parameters:%d",status);
            status = m640->MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
            if (status != 0) {
                ESP_LOGW(TAG,"Parameter extraction failed,erro:%d",status);
                isexdevcie = false;
            }
            // Set refresh rate
            m640->MLX90640_SetRefreshRate(m640->get_i2c_addr(), 0x05); // Set rate to 4Hz effective - Works at 800kHz

            tempValues = static_cast<float*>(heap_caps_malloc(sizeof(float)* 32 * 24, MALLOC_CAP_SPIRAM));
            ipp.interpolated = static_cast<float**>(heap_caps_malloc(O_HEIGHT * sizeof(float*), MALLOC_CAP_SPIRAM));
            for (int i = 0; i < O_HEIGHT; i++) {
                ipp.interpolated[i] = static_cast<float*>(heap_caps_malloc(O_WIDTH * sizeof(float), MALLOC_CAP_SPIRAM));
            }

            for (int i = 0; i < O_HEIGHT; i++) {
                for (int j = 0; j < O_WIDTH; j++) {
                    ipp.interpolated[i][j] = 0.0f; // 初始化为 0
                }
            }            

            canvas = lv_canvas_create(lv_scr_act());

            lv_obj_set_user_data(canvas, this);  // 绑定当前对象作为用户数据
            lv_obj_add_event_cb(canvas, update_ui, LV_EVENT_ALL, this);  // 绑定回调

            lv_obj_set_align( canvas, LV_ALIGN_TOP_MID);
            canvasBuf1 = static_cast<lv_color_t*>(
                heap_caps_malloc((O_WIDTH * O_HEIGHT)* sizeof(lv_color_t), MALLOC_CAP_SPIRAM)
            );
            assert(canvasBuf1);
            canvasBuf2 = static_cast<lv_color_t*>(
                heap_caps_malloc((O_WIDTH * O_HEIGHT)* sizeof(lv_color_t), MALLOC_CAP_SPIRAM)
            );
            assert(canvasBuf2);
            lv_canvas_set_buffer(canvas, canvasBuf1, O_WIDTH, O_HEIGHT, LV_IMG_CF_TRUE_COLOR);
            lv_canvas_fill_bg(canvas, lv_color_hex(0x000000), LV_OPA_100);

            setAbcd();
            drawLegend();

            // fps.millis = millis();
        }

        void InfraredApp::mbox_event_handler(lv_event_t* e) {
            Framework* framework = (Framework*)lv_event_get_user_data(e);
            if (e->code == LV_EVENT_DELETE) {
                _ui_screen_change( &ui_main_tabview, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 200, 0, &ui_main_tapview_screen_init);
                framework->getApp("InfraredApp")->closeTask();
                framework->startApp(framework->getApp("Launcher"));
            }
        }

        void InfraredApp::onResume()
        {
            printf("[%s] onResume\n", getAppName().c_str());
            check_ex_device();
        }

        void InfraredApp::taskLoop()
        {
            uint32_t start = millis();
            //update_ui();
            lv_disp_trig_activity(NULL);
            if(isexdevcie){
                readTempValues();
                //lv_disp_trig_activity(NULL);
                setTempScale();
                lv_disp_trig_activity(NULL);
                lv_event_send(canvas, LV_EVENT_DRAW_PART_BEGIN, this);
                
            }
            uint32_t duration = millis() - start;  // 计算整个 taskLoop 的耗时
            ESP_LOGD(TAG, "taskLoop() took %d ms", duration);
        }

        void InfraredApp::onRunningBG()
        {


        }
        void InfraredApp::onPause()
        {
            printf("[%s] onPause\n", getAppName().c_str());
        }

        void InfraredApp::onDestroy()
        {
            printf("[%s] onDestroy\n", getAppName().c_str());
        }

        void InfraredApp::update_ui(lv_event_t *e)
        {
            if (e->code == LV_EVENT_DRAW_PART_BEGIN)
            {
                InfraredApp* app = (InfraredApp*)lv_event_get_user_data(e);
                app->drawPicture();
                lv_event_send(app->canvas, LV_EVENT_DRAW_PART_END, app);                
            }
            //   if(millis() - fps.millis > 10000 ){
            //         fps.fps = fps.fps_cnt / 10.0f;  // 计算过去10秒的FPS
            //         ESP_LOGD(TAG, "fps : %.2f", fps.fps);  // 输出FPS
            //         fps.millis = millis();  // 更新FPS时间戳
            //         fps.fps_cnt = 0;  // 重置帧计数
            //   }
            //   fps.fps_cnt++;

            
        }

        bool InfraredApp::check_ex_device()
        {
            esp_err_t espRc;
            uint8_t dummyData = 0;
            
            uint8_t infrared_add = update->infrared.get_i2c_addr();
            ESP_LOGD(TAG, "Infrared address: 0x%02X", infrared_add);
            espRc = update->i2cManager.writeToDevice(infrared_add,&dummyData,1);
            if (espRc != ESP_OK){
                static const char* btns[] = {""};
                lv_obj_t *mbox1 = lv_msgbox_create(NULL, "Erro", "infrared device not found.", btns, true);
                lv_obj_center(mbox1);
                lv_obj_add_event_cb(mbox1, mbox_event_handler, LV_EVENT_DELETE, _framework);
                return false;
            }else{
                ESP_LOGD(TAG,"infrared device found");
                isexdevcie = true;
            }
            return true;
        }

        void InfraredApp::readTempValues() {
            uint32_t start = millis();  // 记录开始时间
            uint16_t* mlx90640Frame = static_cast<uint16_t*>(heap_caps_malloc(834 * sizeof(uint16_t), MALLOC_CAP_SPIRAM));
            for (byte x = 0 ; x < 2 ; x++) // Read both subpages
            {
                int status = m640->MLX90640_GetFrameData(m640->get_i2c_addr(), mlx90640Frame);
                if (status < 0)
                {
                    ESP_LOGW(TAG, "GetFrame Error: %d", status);
                }

                float vdd = m640->MLX90640_GetVdd(mlx90640Frame, &mlx90640);
                float Ta = m640->MLX90640_GetTa(mlx90640Frame, &mlx90640);

                float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature

                m640->MLX90640_CalculateTo(mlx90640Frame, &mlx90640, EMMISIVITY, tr, tempValues);
            }

            heap_caps_free(mlx90640Frame);

            uint32_t duration = millis() - start;  // 计算花费时间
            ESP_LOGD(TAG, "readTempValues() took %d ms", duration);
        }

        void InfraredApp::interpolate() {
            // 计算宽度和高度的比例
            float width_ratio = static_cast<float>(O_WIDTH) / 32.0f;
            float height_ratio = static_cast<float>(O_HEIGHT) / 24.0f;

            // 进行插值
            for (int row = 0; row < O_HEIGHT; row++) {
                for (int col = 0; col < O_WIDTH; col++) {
                    int src_row = static_cast<int>(row / height_ratio);  // 计算原始数据行
                    int src_col = static_cast<int>(col / width_ratio);  // 计算原始数据列

                    src_row = std::min(src_row, 23); 
                    src_col = std::min(src_col, 31); 

                    // 获取原始数据中的两个临近点，进行插值
                    float temp = tempValues[src_row * 32 + src_col];  
                    ipp.interpolated[row][col] = temp;
                }
            }
        }

        void InfraredApp::drawPicture() {
            lv_color_t *currentBuf = canvasBuf1;
            lv_color_t *nextBuf = canvasBuf2;

            #if INTERPOLATE
            interpolate();
            uint32_t start = millis();  // 记录开始时间
            // 准备下一帧的画面数据
            for (ids.y = 0; ids.y < O_HEIGHT; ids.y++) {
                for (ids.x = 0; ids.x < O_WIDTH; ids.x++) {
                    // 获取当前 interpolated 的值
                    float interpolatedValue = ipp.interpolated[ids.y][ids.x];

                    // 转换为 lv_color_t
                    lv_color_t color = getColor(interpolatedValue);

                    // 将颜色值写入到 nextBuf 中
                    nextBuf[(ids.y * O_WIDTH) + ids.x] = color;
                }
            }
            uint32_t duration = millis() - start;  // 计算花费时间
            ESP_LOGD(TAG, "prepra_fb_data() took %d ms", duration);

            #elif !INTERPOLATE
                for (ids.y=0; ids.y<24; ids.y++) {
                    for (ids.x=0; ids.x<32; ids.x++) {
                        float temp = tempValues[(31 - ids.x) + (ids.y * 32)];
                        lv_color_t color = getColor(temp);
                        currentBuf[(ids.y * 32 + ids.x)] = color;
                    }
                }
            #endif

            memcpy(currentBuf, nextBuf, O_WIDTH * O_HEIGHT * sizeof(lv_color_t));
            if(lvgl_lock(5))
            {
            // 更新 canvas 显示
                lv_canvas_set_buffer(canvas, currentBuf, O_WIDTH, O_HEIGHT, LV_IMG_CF_TRUE_COLOR);
                lvgl_unlock();  
            }
        }

        void InfraredApp::drawLegend() {
            float step = 17;
            float inc = (maxTemp - minTemp) / step;
            float bar_size = 340;
            tmpa.ii = minTemp;    
            for (int i = 0; i < step; i++)
            {   
                lv_obj_t *_legend = lv_obj_create(ui_infrared_screen);
                lv_obj_set_size(_legend, static_cast<lv_coord_t>(bar_size/step), 40); 
                lv_obj_align(_legend, LV_ALIGN_BOTTOM_MID, -(bar_size/2) + (bar_size/step) * i + 10, -15);   //  布局
                
                lv_color_t start_color = getColor(tmpa.ii);  
                tmpa.ii = tmpa.ii + inc;  // 递增温度值
                lv_color_t end_color = getColor(tmpa.ii);  

                lv_obj_set_style_bg_color(_legend, start_color, LV_PART_MAIN);
                lv_obj_set_style_bg_grad_color(_legend, end_color, LV_PART_MAIN);
                lv_obj_set_style_bg_grad_dir(_legend, LV_GRAD_DIR_HOR, LV_PART_MAIN);
                lv_obj_set_style_radius(_legend, 0, LV_PART_MAIN);
                lv_obj_set_style_border_width(_legend, 0, LV_PART_MAIN| LV_STATE_DEFAULT);  
            }   
        }

        void InfraredApp::setAbcd() {
            tmpa.a = minTemp + (maxTemp - minTemp) * 0.2121;
            tmpa.b = minTemp + (maxTemp - minTemp) * 0.3182;
            tmpa.c = minTemp + (maxTemp - minTemp) * 0.4242;
            tmpa.d = minTemp + (maxTemp - minTemp) * 0.8182;
        }

        lv_color_t InfraredApp::getColor(float val)
        {
            // 用来计算每个颜色通道的常用部分
            float tempRed, tempGreen, tempBlue;

            // 计算红色
            tempRed = 255.0f / (tmpa.c - tmpa.b) * val - ((tmpa.b * 255.0f) / (tmpa.c - tmpa.b));
            rgb.red = constrain(tempRed, 0.0f, 255.0f);

            // 计算绿色
            if (val > minTemp && val < tmpa.a) {
                tempGreen = 255.0f / (tmpa.a - minTemp) * val - (255.0f * minTemp) / (tmpa.a - minTemp);
                rgb.green = constrain(tempGreen, 0.0f, 255.0f);
            }
            else if (val >= tmpa.a && val <= tmpa.c) {
                rgb.green = 255;
            }
            else if (val > tmpa.c) {
                tempGreen = 255.0f / (tmpa.c - tmpa.d) * val - (tmpa.d * 255.0f) / (tmpa.c - tmpa.d);
                rgb.green = constrain(tempGreen, 0.0f, 255.0f);
            }
            else {
                rgb.green = 0;
            }

            // 计算蓝色
            if (val <= tmpa.b) {
                tempBlue = 255.0f / (tmpa.a - tmpa.b) * val - (255.0f * tmpa.b) / (tmpa.a - tmpa.b);
                rgb.blue = constrain(tempBlue, 0.0f, 255.0f);
            }
            else if (val > tmpa.b && val <= tmpa.d) {
                rgb.blue = 0;
            }
            else {
                tempBlue = 240.0f / (maxTemp - tmpa.d) * val - (tmpa.d * 240.0f) / (maxTemp - tmpa.d);
                rgb.blue = constrain(tempBlue, 0.0f, 240.0f);
            }

            // 返回 5-6-5 显示所需颜色
            return lv_color_make(rgb.red, rgb.green, rgb.blue);
        }

        float InfraredApp::constrain(float x, float a, float b) {
            if (x < a) return a;
            if (x > b) return b;
            return x;
        }

        void InfraredApp::setTempScale() {
            minTemp = 255;
            maxTemp = 0;

            for (int i = 0; i < 768; i++) {
                minTemp = std::min(minTemp, tempValues[i]);
                maxTemp = std::max(maxTemp, tempValues[i]);
            }

            // setAbcd();
            // drawLegend();
        }
    }
}