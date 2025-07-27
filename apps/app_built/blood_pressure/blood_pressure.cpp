#include "blood_pressure.h"
#include <cstring>

#define TF_LITE_REPORT_ERROR(reporter, ...) \
    fprintf(stderr, __VA_ARGS__)

namespace LAMINATEPIE
{
    namespace BUILTIN_APP
    {
        void BloodPressureApp::_lvgl_bp_event_cb(lv_event_t *e)
        {
                lv_event_code_t event_code = lv_event_get_code(e);
            if ( event_code == LV_EVENT_CLICKED) {
                BloodPressureApp* bpa = (BloodPressureApp*)lv_event_get_user_data(e);
                // if(bpa->update->max30105.getIR() < 50000){
                //     static const char* btns[] = {""};
                //     lv_obj_t* mbox1 = lv_msgbox_create(NULL, "Erro", "Handset mistake. Please check the watch wear.", btns, true);
                //     lv_obj_center(mbox1);   
                //     return;}
                bpa->update->motor.shake(1,1000);
                bpa->bp_handle.Run();
                bpa->update->motor.shake(1,1000);
            }
        }

        void BloodPressureApp::_lvgl_kb_event_cb(lv_event_t *e)
        {
            lv_event_code_t event_code = lv_event_get_code(e);
            BloodPressureApp* bpa = (BloodPressureApp*)lv_event_get_user_data(e);

            if( event_code == LV_EVENT_READY)
            {
                const char* txt = lv_textarea_get_text(ui_TextArea3);

                if (!txt || txt[0] == '\0') {
                    lv_msgbox_create(NULL, "Error", "Input cannot be empty.", NULL, true);
                    return;
                }

                int value = atoi(txt); 
                if(bpa->first_check){
                    bpa->userdata["height"] = value;
                    lv_textarea_set_placeholder_text(ui_TextArea3, "体重(kg)");

                    lv_textarea_set_text(ui_TextArea3, "");
                    bpa->first_check = false;
                }else{
                    bpa->userdata["weight"] = value;
                    lv_textarea_set_placeholder_text(ui_TextArea3, "身高(cm)");

                    _ui_flag_modify(ui_Keyboard3, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_ADD);
                    _ui_flag_modify(ui_TextArea3, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_ADD);
                    
                    bpa->bp_handle.putStaticData(bpa->userdata);
                    lv_textarea_set_text(ui_TextArea3, "");
                    bpa->first_check = true;
                    static const char* btns[] = {""};
                    lv_obj_t* mbox1 = lv_msgbox_create(NULL, "Success", "User data set successfully.", btns, true);
                    lv_obj_center(mbox1);                
                }
            }
        }

        void BloodPressureApp::onSetup()
        {
            setTaskName("BPApp");
            setScreen(ui_Screen1);
            setAllowBgRunning(false);
            setAppIcon((void *)&ICON_BLOODPRESSURE);
            setStack(5000);
        }

        void BloodPressureApp::onCreate()
        {
            printf("[%s] onCreate\n", getAppName().c_str());

            _framework->enableHignPowerMode();
            update = &_framework->getHAL();
            update->max30105.setPulseAmplitudeIR(0x7F);
            update->init_red_led(0.5);
            update->motor.init();
            update->motor.shake(1,500);
            bp_handle.BPInitial();
            lv_obj_add_event_cb(ui_bp_Button1, _lvgl_bp_event_cb, LV_EVENT_ALL, (void*)this);
            lv_obj_add_event_cb(ui_TextArea3, _lvgl_kb_event_cb, LV_EVENT_READY, this);
        }

        void BloodPressureApp::onResume()
        {
            update->init_ir_led(0.5);
            printf("[%s] onResume\n", getAppName().c_str());
        }

        void BloodPressureApp::taskLoop()
        {
            lv_disp_trig_activity(NULL); // keep screen activity           
        }

        void BloodPressureApp::onRunningBG()
        {

        }
        void BloodPressureApp::onPause()
        {
            printf("[%s] onPause\n", getAppName().c_str());
            update->max30105.setPulseAmplitudeIR(0);
            update->deinit_ir_led();
        }

        void BloodPressureApp::onDestroy()
        {
            update->max30105.setPulseAmplitudeIR(0);
            update->deinit_ir_led();
            printf("[%s] onDestroy\n", getAppName().c_str());
        }
    }
}