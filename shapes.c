/*
 * File: shapes.c
 * ----------------------
 * Shape cooperative demos. 
 */

#include <stdarg.h>
#include "shapes.h"
#include "gl.h"
#include "threading.h"
#include "windows.h"

/*
 * Function: draw_snake
 * ----------------------
 * Renders a snake in the given window
 * Requires two arguments: a window id and a snake_t config object
 * The snake will circle the edge of the screen
 */
void draw_snake(va_list args){
    int window_id = va_arg(args, int);
    snake_t snake = va_arg(args, snake_t);


    int length = snake.length;
    int head_size = snake.head_size;
    int width = snake.width;
    int move_distance = snake.move_distance;

    color body_color = snake.body_color;
    color head_color = snake.head_color;

    int body_length = length - head_size;

    int snake_pos_top = 0;
    int snake_pos_right = 0;
    int snake_pos_bottom = 0;
    int snake_pos_left = 0;

    int top_active = snake.start_segment <= 0;
    int right_active = snake.start_segment == 1;
    int bottom_active = snake.start_segment == 2;
    int left_active = snake.start_segment >= 3;

    while(1) {
        if (top_active) {
            window_draw_rect(snake_pos_top - body_length, 0, body_length, width, body_color, window_id);
            window_draw_rect(snake_pos_top, 0, head_size, width, head_color, window_id);

            snake_pos_top += move_distance;
            if (snake_pos_top >= (int)window_get_width(window_id) - move_distance){
                snake_pos_left = 0;
                left_active = 0;
                right_active = 1;
            }
        }

        if (right_active) {
            window_draw_rect(window_get_width(window_id) - width, snake_pos_right - body_length, width, body_length,
                             body_color, window_id);
            window_draw_rect(window_get_width(window_id) - width, snake_pos_right - width, width, head_size, head_color,
                             window_id);

            snake_pos_right += move_distance;
            if (snake_pos_right >= (int)window_get_height(window_id) - move_distance){
                snake_pos_top = 0;
                top_active = 0;
                bottom_active = 1;
            }
        }

        if (bottom_active) {
            window_draw_rect(window_get_width(window_id) - snake_pos_bottom + head_size, window_get_height(window_id) - width, width, body_length,
                             body_color, window_id);
            window_draw_rect(window_get_width(window_id) - snake_pos_bottom, window_get_height(window_id) - width, width, head_size, head_color,
                             window_id);

            snake_pos_bottom += move_distance;
            if (snake_pos_bottom >= (int)window_get_width(window_id) - move_distance){
                snake_pos_right = 0;
                right_active = 0;
                left_active = 1;
            }
        }

        if (left_active) {
            window_draw_rect(0, window_get_height(window_id) - snake_pos_left + head_size, width, body_length,
                             body_color, window_id);
            window_draw_rect(0, window_get_height(window_id) - snake_pos_left, width, head_size, head_color,
                             window_id);

            snake_pos_left += move_distance;
            if (snake_pos_left >= (int)window_get_height(window_id) - move_distance){
                snake_pos_bottom = 0;
                bottom_active = 0;
                top_active = 1;
            }
        }


        task_delay(snake.speed);
    }
}

/*
 * Function: draw_orbiter
 * -----------------------
 * Draws an orbiter in the given window
 * Requires two arguments: a window_id and a orbiter_t config object
 * The orbiter will orbit the given center point
 */
void draw_orbiter(va_list args) {
    int window_id = va_arg(args, int);
    orbiter_t orbiter = va_arg(args, orbiter_t);

    while (1){
        //clear
        window_draw_rect(orbiter.x, orbiter.y, orbiter.width, orbiter.height, orbiter.trail, window_id);

        //update pos
        orbiter.x += orbiter.dx;
        orbiter.y += orbiter.dy;

        //update the speed
        orbiter.dx += (orbiter.center_x - orbiter.x)/100;// / distance_2;
        orbiter.dy += (orbiter.center_y - orbiter.y)/100;// / distance_2;

        //redraw
        window_draw_rect(orbiter.x, orbiter.y, orbiter.width, orbiter.height, orbiter.fill, window_id);

        task_delay(orbiter.speed);
    }
}

/*
 * Function: window_snakes
 * ------------------------
 * Sets up a window, then draws a number of snakes and orbiters
 */
void window_snakes(){
    int window_id = initialize_window(0, 0, windows_get_width(), 2*windows_get_height()/3);
    safe_printf("Initialized snakes window \n");
    yield();

    snake_t snake1;

    snake1.length = window_get_width(window_id);
    snake1.head_size = 10;
    snake1.width = 10;
    snake1.move_distance = 5;
    snake1.body_color = gl_color(255, 150, 150);
    snake1.head_color = gl_color(255, 0, 0);
    snake1.speed = 0x800;
    snake1.start_segment = 0;


    snake_t snake2;

    snake2.length = window_get_width(window_id);
    snake2.head_size = 10;
    snake2.width = 10;
    snake2.move_distance = 5;
    snake2.body_color = gl_color(150, 150, 255);
    snake2.head_color = gl_color(0, 0, 255);
    snake2.speed = 0x800;
    snake2.start_segment = 2;

    orbiter_t orbit1;
    orbit1.width = 10;
    orbit1.height = 10;
    orbit1.fill = gl_color(255, 0, 255);
    orbit1.trail = gl_color(255, 230, 255);

    orbit1.center_x = window_get_width(0)/2;
    orbit1.center_y = window_get_height(0)/2;
    orbit1.x = 3*window_get_width(0)/4;
    orbit1.y = window_get_height(0)/2;
    orbit1.dx = 0;
    orbit1.dy = 10;
    orbit1.speed = 0x400;


    orbiter_t orbit2;
    orbit2.width = 10;
    orbit2.height = 10;
    orbit2.fill = gl_color(255, 255, 0);
    orbit2.trail = gl_color(255, 255, 230);

    orbit2.center_x = window_get_width(0)/2;
    orbit2.center_y = window_get_height(0)/2;
    orbit2.x = 1*window_get_width(0)/4;
    orbit2.y = window_get_height(0)/2;
    orbit2.dx = 0;
    orbit2.dy = -10;
    orbit2.speed = 0x400;


    orbiter_t orbit3;
    orbit3.width = 10;
    orbit3.height = 10;
    orbit3.fill = gl_color(255, 0, 0);
    orbit3.trail = gl_color(255, 230, 230);

    orbit3.center_x = window_get_width(0)/2;
    orbit3.center_y = window_get_height(0)/2;
    orbit3.x = window_get_width(0)/2;
    orbit3.y = 3*window_get_height(0)/4;
    orbit3.dx = 15;
    orbit3.dy = -10;
    orbit3.speed = 0x400;


    orbiter_t orbit4;
    orbit4.width = 10;
    orbit4.height = 10;
    orbit4.fill = gl_color(0, 0, 255);
    orbit4.trail = gl_color(230, 230, 255);

    orbit4.center_x = window_get_width(0)/2;
    orbit4.center_y = window_get_height(0)/2;
    orbit4.x = window_get_width(0)/2;
    orbit4.y = window_get_height(0)/4;
    orbit4.dx = -15;
    orbit4.dy = 10;
    orbit4.speed = 0x400;

    run_thread_coop(draw_snake, 1, window_id, snake1);
    run_thread_coop(draw_snake, 1, window_id, snake2);

    int planet_size = 20;
    window_draw_rect(window_get_width(0)/2 - planet_size/2, window_get_height(0)/2 - planet_size/2, planet_size, planet_size, 0, 0);

    run_thread_coop(draw_orbiter, 1, window_id, orbit1);
    run_thread_coop(draw_orbiter, 1, window_id, orbit2);
    run_thread_coop(draw_orbiter, 1, window_id, orbit3);
    run_thread_coop(draw_orbiter, 1, window_id, orbit4);
}

/*
 * Function: draw_bouncer
 * ------------------------
 * Draws a bouncer in the given window
 * Requires two arguments: a window id and a bouncer_t config object
 * The bouncer will bounce off the walls of the window indefinitely
 */
void draw_bouncer(va_list args) {
    int window_id = va_arg(args, int);
    bouncer_t bouncer = va_arg(args, bouncer_t);

    while (1){
        //clear
        window_draw_rect(bouncer.x, bouncer.y, bouncer.width, bouncer.height, bouncer.trail, window_id);

        //update pos
        bouncer.x += bouncer.dx;
        bouncer.y += bouncer.dy;

        if ((bouncer.x < 0) || (bouncer.x + bouncer.width >= window_get_width(window_id))){
            bouncer.dx *= -1;
            bouncer.x += bouncer.dx;
        }
        if ((bouncer.y < 0) || (bouncer.y + bouncer.height >= window_get_height(window_id))){
            bouncer.dy *= -1;
            bouncer.y += bouncer.dy;
        }
        bouncer.y += bouncer.dy;

        //redraw
        window_draw_rect(bouncer.x, bouncer.y, bouncer.width, bouncer.height, bouncer.fill, window_id);

        task_delay(bouncer.speed);
    }
}

/*
 * Function: window_bounce
 * ------------------------
 * Sets up a window and renders a number of bouncers
 */
void window_bounce(){
    int window_id = initialize_window(0, 2*windows_get_height()/3, windows_get_width(), windows_get_height()/3);
    safe_printf("Initialized bounce window\n");
    yield();

    bouncer_t bounce1;
    bounce1.width = 10;
    bounce1.height = 10;
    bounce1.fill = gl_color(0, 255, 0);
    bounce1.trail = gl_color(230, 255, 230);

    bounce1.x = 0;
    bounce1.y = 15;
    bounce1.dx = 3;
    bounce1.dy = 5;

    bounce1.speed = 0x400;

    bouncer_t bounce2;
    bounce2.width = 10;
    bounce2.height = 10;
    bounce2.fill = gl_color(255, 255, 0);
    bounce2.trail = gl_color(255, 255, 230);

    bounce2.x = window_get_width(0) - 60;
    bounce2.y = 65;
    bounce2.dx = -5;
    bounce2.dy = 3;

    bounce2.speed = 0x400;

    bouncer_t bounce3;
    bounce3.width = 10;
    bounce3.height = 10;
    bounce3.fill = gl_color(0, 255, 255);
    bounce3.trail = gl_color(230, 255, 255);

    bounce3.x = window_get_width(0)/2;
    bounce3.y = window_get_height(0)/2;
    bounce3.dx = 9;
    bounce3.dy = 4;

    bounce3.speed = 0x400;

    run_thread_coop(draw_bouncer, 1, window_id, bounce1);
    run_thread_coop(draw_bouncer, 1, window_id, bounce3);
    run_thread_coop(draw_bouncer, 1, window_id, bounce2);

    while (1) {
        yield();
    }
}
