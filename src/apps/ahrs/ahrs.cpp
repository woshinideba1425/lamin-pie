#include "ahrs.h"
#include <cmath>

#define ROTATION_SPEED 1.0f
#define ROTATION_SPEED_X 1.0f
#define ROTATION_SPEED_Y 0.5f
#define ROTATION_SPEED_Z 0.3f
#define DT 0.05f

namespace LAMINATEPIE
{
    namespace BUILTIN_APP
    {
        void Ahrs::onSetup()
        {
            setTaskName("AhrsApp");
            setScreen(ui_ahrs_screen);
            setAllowBgRunning(false);
            setAppIcon((void *)&ICON_AHRS);
        }

        void Ahrs::onCreate()
        {            
            printf("[%s] onCreate\n", getAppName().c_str());
            canvas = lv_canvas_create(lv_scr_act());
            lv_obj_center(canvas);
            static lv_color_t *canvasBuf = static_cast<lv_color_t*>(
                heap_caps_malloc((32 * 256) / 8 * 256, MALLOC_CAP_SPIRAM)
            );
            assert(canvasBuf);
            lv_canvas_set_buffer(canvas, canvasBuf, 320, 320, LV_IMG_CF_TRUE_COLOR);
            lv_canvas_fill_bg(canvas, lv_color_hex(0x000000), LV_OPA_100);

            update = &_framework->getHAL();
            mpu = update->posture.get_mpu6050_handle();

        }

        void Ahrs::onResume()
        {
            _framework->enableLowPowerMode();
            printf("[%s] onResume\n", getAppName().c_str());
        }

        void Ahrs::taskLoop()
        {
            lv_disp_trig_activity(NULL);

            update_ui();
            //vTaskDelay(100);
        }

        void Ahrs::onRunningBG()
        {


        }
        void Ahrs::onPause()
        {
            printf("[%s] onPause\n", getAppName().c_str());

        }

        void Ahrs::onDestroy()
        {
            printf("[%s] onDestroy\n", getAppName().c_str());

        }

        void Ahrs::update_ui()
        {
            MPU6050::mpu6050_gyro_value_t gyro_value{0,0,0};
            update->posture.mpu6050_get_gyro(mpu,&gyro_value);
            update_rotation_angles(&gyro_value);
            draw_rotating_cube(canvas,current_anglex,current_angley,current_anglez);
        }

        void Ahrs::rotate_x(float vertices[8][3], float angle_deg) {
            float angle_rad = angle_deg * M_PI / 180.0f;
            float cosTheta = cos(angle_rad);
            float sinTheta = sin(angle_rad);

            // 计算立方体的中心 (cx, cy, cz)
            float cx = 0.0f, cy = 0.0f, cz = 0.0f;
            for (int i = 0; i < 8; i++) {
                cx += vertices[i][0];
                cy += vertices[i][1];
                cz += vertices[i][2];
            }
            cx /= 8.0f;  // 计算中心的 X 坐标
            cy /= 8.0f;  // 计算中心的 Y 坐标
            cz /= 8.0f;  // 计算中心的 Z 坐标

            // 1. 平移到原点
            for (int i = 0; i < 8; i++) {
                vertices[i][0] -= cx;
                vertices[i][1] -= cy;
                vertices[i][2] -= cz;
            }

            // 2. 应用旋转矩阵
            for (int i = 0; i < 8; i++) {
                float y = vertices[i][1];
                float z = vertices[i][2];
                float rotatedY = cosTheta * y - sinTheta * z;
                float rotatedZ = sinTheta * y + cosTheta * z;
                vertices[i][1] = rotatedY + cy;  // 恢复 Y 坐标
                vertices[i][2] = rotatedZ + cz;  // 恢复 Z 坐标
            }
        }

        void Ahrs::rotate_y(float vertices[8][3], float angle_deg) {
            float angle_rad = angle_deg * M_PI / 180.0f;
            float cosTheta = cos(angle_rad);
            float sinTheta = sin(angle_rad);

            // 计算立方体的中心 (cx, cy, cz)
            float cx = 0.0f, cy = 0.0f, cz = 0.0f;
            for (int i = 0; i < 8; i++) {
                cx += vertices[i][0];
                cy += vertices[i][1];
                cz += vertices[i][2];
            }
            cx /= 8.0f;  // 计算中心的 X 坐标
            cy /= 8.0f;  // 计算中心的 Y 坐标
            cz /= 8.0f;  // 计算中心的 Z 坐标
            for (int i = 0; i < 8; i++) {
                vertices[i][0] -= cx;
                vertices[i][1] -= cy;
                vertices[i][2] -= cz;
            }

            for (int i = 0; i < 8; i++) {
                float x = vertices[i][0];
                float z = vertices[i][2];
                float rotatedX = cosTheta * x + sinTheta * z;
                float rotatedZ = -sinTheta * x + cosTheta * z;
                vertices[i][0] = rotatedX + cx;  // 恢复 X 坐标
                vertices[i][2] = rotatedZ + cz;  // 恢复 Z 坐标
            }
        }

        void Ahrs::rotate_z(float vertices[8][3], float angle_deg) {
            float angle_rad = angle_deg * M_PI / 180.0f;
            float cosTheta = cos(angle_rad);
            float sinTheta = sin(angle_rad);

            // 计算立方体的中心 (cx, cy, cz)
            float cx = 0.0f, cy = 0.0f, cz = 0.0f;
            for (int i = 0; i < 8; i++) {
                cx += vertices[i][0];
                cy += vertices[i][1];
                cz += vertices[i][2];
            }
            cx /= 8.0f;  // 计算中心的 X 坐标
            cy /= 8.0f;  // 计算中心的 Y 坐标
            cz /= 8.0f;  // 计算中心的 Z 坐标
            for (int i = 0; i < 8; i++) {
                vertices[i][0] -= cx;
                vertices[i][1] -= cy;
                vertices[i][2] -= cz;
            }

            for (int i = 0; i < 8; i++) {
                float x = vertices[i][0];  // 获取 X 坐标
                float y = vertices[i][1];  // 获取 Y 坐标
                float z = vertices[i][2];  // 获取 Z 坐标（在 Z 轴旋转时，z 不变）
                float rotatedX = cosTheta * x - sinTheta * y;
                float rotatedY = sinTheta * x + cosTheta * y;
                vertices[i][0] = rotatedX + cx;  // 恢复 X 坐标
                vertices[i][1] = rotatedY + cy;  // 恢复 Y 坐标
                vertices[i][2] = z + cz;         // Z 坐标不变，直接恢复
            }
        }

        void Ahrs::draw_rotating_cube(lv_obj_t * canvas, float angle_x, float angle_y, float angle_z) {
           

            // 旋转顶点
            float temp_vertices[8][3];  // 临时保存顶点数据
            for (int i = 0; i < 8; i++) {
                temp_vertices[i][0] = vertices[i][0];  // 保留原始 X 坐标
                temp_vertices[i][1] = vertices[i][1];  // 保留原始 Y 坐标
                temp_vertices[i][2] = vertices[i][2];  // 保留原始 Z 坐标
            }

            rotate_x(temp_vertices, angle_x);
            rotate_y(temp_vertices, angle_y);
            rotate_z(temp_vertices, angle_z);

            
            float projected_vertices[8][2];
            project(temp_vertices,projected_vertices,2,4);
            convert_to_pixel_coords(projected_vertices,240,240,1);

            // 绘制旋转后的立方体
            lv_draw_line_dsc_t draw_dsc;
            lv_draw_line_dsc_init(&draw_dsc);
            draw_dsc.color = lv_color_hex(0xFFFFFF);  // 白色线条
            if(lvgl_lock(50)){ 
                lv_canvas_fill_bg(canvas, lv_color_hex(0x000000), LV_OPA_100); 
                draw_lines(canvas, my_pixel_vertices, draw_dsc);  // 绘制每条线
                lvgl_unlock();
            }

        }

        void Ahrs::project(float vertices[8][3], float projected_vertices[8][2], float focal_length, float depth) {
                for (int i = 0; i < 8; i++) {
                    float x = vertices[i][0];
                    float y = vertices[i][1];
                    float z = vertices[i][2];

                    // 透视投影公式
                    projected_vertices[i][0] = x * focal_length/ (z + depth) ;  // 计算 x 坐标
                    projected_vertices[i][1] = y * focal_length/ (z + depth) ;  // 计算 y 坐标
                    //printf("Projected Vertex %d: (%f, %f)\n", i, projected_vertices[i][0], projected_vertices[i][1]);  
                }
        }

        void Ahrs::convert_to_pixel_coords(float projectedVertices[8][2], float screenWidth, float screenHeight, float scaleFactor){
            for (int i = 0; i < 8; i++) {
                // 假设原点在屏幕中心，映射到屏幕范围内
                float x = projectedVertices[i][0] * screenWidth / 2+ screenWidth / 2 ;
                float y = projectedVertices[i][1] * screenHeight / 2 + screenHeight / 2; // y轴反转

                my_pixel_vertices[i].x = (x + 35) * scaleFactor;
                my_pixel_vertices[i].y = (y + 50) * scaleFactor;
                // 输出屏幕坐标
                //std::cout << "Pixel (" << x << ", " << y << ")" << std::endl;
            }

            
        }
        
        void Ahrs::init_cude(){
            lv_draw_line_dsc_t draw_dsc;
            lv_draw_line_dsc_init(&draw_dsc);
            draw_dsc.color = lv_color_hex(0xFFFFFF);  // 白色线条

            draw_rotating_cube(canvas, 20, 0, 0);

        }

        // 画线函数，用于绘制立方体的各条线
        void Ahrs::draw_lines(lv_obj_t * canvas, lv_point_t vertices[], lv_draw_line_dsc_t draw_dsc) {
            for (int i = 0; i < 12; i++) {
                lv_point_t line_points[2];
                line_points[0] = my_pixel_vertices[cube_edges[i][0]];
                line_points[1] = my_pixel_vertices[cube_edges[i][1]];

                lv_canvas_draw_line(canvas, line_points, 2, &draw_dsc);
            }
        }

        void Ahrs::update_rotation_angles(MPU6050::mpu6050_gyro_value_t *gyro_value) {
            // 通过陀螺仪数据更新旋转角度
            current_anglex += gyro_value->gyro_x * DT;
            current_angley += gyro_value->gyro_y * DT;
            current_anglez += gyro_value->gyro_z * DT;

            // 限制角度范围
            if (current_anglex > 180.0f) current_anglex -= 360.0f;
            if (current_anglex < -180.0f) current_anglex += 360.0f;
            if (current_angley > 180.0f) current_angley -= 360.0f;
            if (current_angley < -180.0f) current_angley += 360.0f;
            if (current_anglez > 180.0f) current_anglez -= 360.0f;
            if (current_anglez < -180.0f) current_anglez += 360.0f;
        }

    }
}