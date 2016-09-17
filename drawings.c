/* 
 * File: drawings.c
 * -----------------
 * Various timer-interrupted threading demos. 
 */ 

#include <stdarg.h>

#include "gl.h"
#include "pic.c"
#include "timer.h"
#include "printf.h"
#include "drawings.h"
#include "threading.h"

extern int width; 
extern int height; 

// Number of context switches, counted by interrupt handler 
extern unsigned context_switch_count; 

/* 
 * Function: rect_thread
 * ----------------------
 * Thread to draw rectangles. 
 */
void rect_thread(va_list args) {
    unsigned x = va_arg(args, int); 
    unsigned y = va_arg(args, int); 
    unsigned w = va_arg(args, int); 
    unsigned h = va_arg(args, int); 
    unsigned col = va_arg(args, unsigned); 

    unsigned counter = 1; 

    while (1) {
        gl_draw_rect(x, y, w, h, col); 
        col = pastel(); 
        if (counter % 10 == 0) {
            w = rand() % MAX_RECT_WIDTH;
            h = rand() % MAX_RECT_HEIGHT;
            x = (rand() % (width + w)) - w;
            y = rand() % ((height + h)) - h;
        }
        counter ++; 
    }
}

/* 
 * Function: char_thread
 * ----------------------
 * Thread to draw characters. 
 */
void char_thread(va_list args) {
    unsigned x, y, col; 

    while (1) {
        char ch = (rand() % 95) + 32; 
        col = pastel(); 
        x = rand() % width; 
        y = rand() % height; 
        gl_draw_char(x, y, ch, col); 
        for (unsigned ii = 0; ii < 0x40000; ii ++);    
    }
}

/* 
 * Function: triangle_thread
 * --------------------------
 * Thread to draw triangles. 
 */ 
void triangle_thread(va_list args) {
    int x1 = va_arg(args, int); 
    int y1 = va_arg(args, int); 
    int x2 = va_arg(args, int); 
    int y2 = va_arg(args, int); 
    int x3 = va_arg(args, int); 
    int y3 = va_arg(args, int); 
    unsigned col = va_arg(args, unsigned);

    unsigned counter = 1;

    while (1) {
        gl_draw_triangle(x1, y1, x2, y2, x3, y3, col);
        col = pastel();
        
        if (counter % 10 == 0) {
            x1 = rand() % width;
            x2 = rand() % width;
            x3 = rand() % width;
            y1 = rand() % height;
            y2 = rand() % height;
            y3 = rand() % height;
        }
        counter ++;
    }
}

/* 
 * Function: image_col
 * ---------------------
 * Get color for pixel in image assuming 1 byte per pixel. 
 */ 
unsigned image_col(int image_num, int x, int y, unsigned blend) {
    char a = 0; 
    if (image_num == 0) {
        // a = image_0.pixel_data[(y + x * image_0.width) * image_0.bytes_per_pixel]; 
    } else if (image_num == 1) {
        // a = image_1.pixel_data[(y + x * image_1.width) * image_1.bytes_per_pixel]; 
    } else if (image_num == 2) {
        // a = image_2.pixel_data[(y + x * image_2.width) * image_2.bytes_per_pixel]; 
    } else {
        return 0; // Invalid image
    }

    return (blend & 0xFFFFFF) | (a << 24); 
}

/* 
 * Function: non_timer_delay
 * --------------------------
 * Simple fake delay used for debugging and gdb. 
 */ 
void non_timer_delay() {
    for (unsigned ii = 0; ii < 0x10000; ii ++);
}

/* 
 * Function: image_thread
 * -----------------------
 * Thread to draw an image (assuming 300x300 image). 
 */ 
void image_thread(va_list args) {
    unsigned blend = pastel();

    int image_num = va_arg(args, int);  
    int sector = va_arg(args, int);  
    
    // Change for non 300x300 images 
    int start_x = (sector % 3) * 100; 
    int start_y = (sector / 3) * 100; 
    int end_x = start_x + 100; 
    int end_y = start_y + 100; 

    while (1) {
        // Draw an image in different ways to make the demo cooler
        if (sector % 2 == 0) {
            for (int ii = end_x - 1; ii >= start_x; ii --) {
                for (int jj = end_y - 1; jj >= start_y; jj --) {
                    unsigned color = image_col(image_num, ii, jj, blend); 
                    gl_draw_pixel(ii, jj, color); 
                    delay_us(0x40); 
                }
            } 
        } else {
            for (int ii = start_x; ii < end_x; ii ++) {
                for (int jj = start_y; jj < end_y; jj ++) {
                    unsigned color = image_col(image_num, ii, jj, blend); 
                    gl_draw_pixel(ii, jj, color); 
                    delay_us(0x40); 
                }
            } 
        }
        
        non_timer_delay(); 

        unsigned clear_col = pastel(); 
        for (int ii = start_x; ii < end_x; ii ++) {
            for (int jj = start_y; jj < end_y; jj ++) {
                gl_draw_pixel(ii, jj, clear_col); 
            }
        } 

        gl_draw_char(start_x + 20, start_y + 20, sector + '0', GL_BLUE); 
        printf("%u context switches have occurred\n", context_switch_count); 
    }
}