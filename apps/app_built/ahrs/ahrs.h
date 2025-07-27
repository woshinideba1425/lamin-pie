#pragma once
#include "../../app/app.hpp"
#include "../../framework/framwork.h"
#define CUBE_SIZE 100

namespace LAMINATEPIE {
    namespace BUILTIN_APP {

        class Ahrs : public FreeRTOSAppBase {
            private:
                Framework* _framework = Framework::getInstance();
                FreeRTOSAppBase* _ahrs_app;
                const char* TAG = "Ahrs";
                lv_obj_t * canvas;
                lv_coord_t canvas_center_x = 150;
                lv_coord_t canvas_center_y = 150;
                float current_anglex = 0.0f;
                float current_angley = 0.0f;
                float current_anglez = 0.0f;
                HAL *update;
                MPU6050::mpu6050_handle_t mpu;

                float vertices[8][3] = {

                    {-1.0f, -1.0f,  1.0f},  // Back-left-bottom
                    { 1.0f, -1.0f,  1.0f},  // Back-right-bottom
                    { 1.0f,  1.0f,  1.0f},  // Back-right-top
                    {-1.0f,  1.0f,  1.0f},   // Back-left-top                    
                    {-1.0f, -1.0f, -1.0f},  // Front-left-bottom
                    { 1.0f, -1.0f, -1.0f},  // Front-right-bottom
                    { 1.0f,  1.0f, -1.0f},  // Front-right-top
                    {-1.0f,  1.0f, -1.0f},  // Front-left-top
                };
                
                int cube_edges[12][2] = {
                    {0, 1}, {1, 2}, {2, 3}, {3, 0}, // 后面
                    {4, 5}, {5, 6}, {6, 7}, {7, 4}, // 前面
                    {0, 4}, {1, 5}, {2, 6}, {3, 7}  // 侧面
                };

                lv_point_t my_pixel_vertices[8] = {
                    {static_cast<lv_coord_t>(canvas_center_x), static_cast<lv_coord_t>(canvas_center_y)},  // Front face left-top
                    {static_cast<lv_coord_t>(canvas_center_x + CUBE_SIZE), static_cast<lv_coord_t>(canvas_center_y)},  // Front face right-top
                    {static_cast<lv_coord_t>(canvas_center_x + CUBE_SIZE), static_cast<lv_coord_t>(canvas_center_y + CUBE_SIZE)},  // Front face right-bottom
                    {static_cast<lv_coord_t>(canvas_center_x), static_cast<lv_coord_t>(canvas_center_y + CUBE_SIZE)},  // Front face left-bottom

                    {static_cast<lv_coord_t>(canvas_center_x + CUBE_SIZE / 2), static_cast<lv_coord_t>(canvas_center_y + CUBE_SIZE / 2)},  // Back face left-top
                    {static_cast<lv_coord_t>(canvas_center_x + CUBE_SIZE + CUBE_SIZE / 2), static_cast<lv_coord_t>(canvas_center_y + CUBE_SIZE / 2)},  // Back face right-top
                    {static_cast<lv_coord_t>(canvas_center_x + CUBE_SIZE + CUBE_SIZE / 2), static_cast<lv_coord_t>(canvas_center_y + CUBE_SIZE + CUBE_SIZE / 2)},  // Back face right-bottom
                    {static_cast<lv_coord_t>(canvas_center_x + CUBE_SIZE / 2), static_cast<lv_coord_t>(canvas_center_y + CUBE_SIZE + CUBE_SIZE / 2)}   // Back face left-bottom
                };


            public:
                Ahrs() : _framework(Framework::getInstance()), _ahrs_app(nullptr) {}
                ~Ahrs() = default;

                /**
                 * @brief Lifecycle callbacks for derived to override
                 * 
                 */
                /* Setup App configs, called when App "install()" */
                void onSetup();
                void update_ui();
                
                void rotate_x(float vertices[8][3], float angle_deg);
                void rotate_z(float vertices[8][3], float angle_deg);
                void rotate_y(float vertices[8][3], float angle_deg);

                void draw_lines(lv_obj_t *canvas, lv_point_t vertices[], lv_draw_line_dsc_t draw_dsc);
                void update_rotation_angles(MPU6050::mpu6050_gyro_value_t *gyro_value);
                void convert_to_pixel_coords(float projectedVertices[8][2], float screenWidth, float screenHeight, float scaleFactor);
                void project(float vertices[8][3], float projected_vertices[8][2], float focal_length, float depth);
                
                void init_cude();
                void draw_rotating_cube(lv_obj_t *canvas, float angle_x, float angle_y, float angle_z);

                /* Life cycle */
                void onCreate();
                void onResume();
                void taskLoop();
                void onRunningBG();
                void onPause();
                void onDestroy();
            
        };

 

    }
}