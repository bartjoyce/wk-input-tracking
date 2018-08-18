//
//  main.cpp
//  wp-input-tracking
//
//  Created by Bartholomew Joyce on 18/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include <ddui/app>
#include <cmath>

constexpr auto BAR_HEIGHT = 22.0;

bool hit(Context ctx, int x1, int y1, int width, int height) {
    if (!ctx.mouse->pressed || ctx.mouse->accepted) {
        return false;
    }
    
    float mx, my;
    float mat[6], inv_mat[6];
    nvgCurrentTransform(ctx.vg, mat);
    nvgTransformInverse(inv_mat, mat);
    nvgTransformPoint(&mx, &my, inv_mat, ctx.mouse->initial_x, ctx.mouse->initial_y);
    
    int x2 = x1 + width;
    int y2 = y1 + height;
    return (
        (ctx.x + x1 < mx) && (mx <= ctx.x + x2) &&
        (ctx.y + y1 < my) && (my <= ctx.y + y2)
    );
}

bool over(Context ctx, int x1, int y1, int width, int height) {
    if (ctx.mouse->pressed || ctx.mouse->pressed_secondary) {
        return false;
    }
    
    float mx, my;
    float mat[6], inv_mat[6];
    nvgCurrentTransform(ctx.vg, mat);
    nvgTransformInverse(inv_mat, mat);
    nvgTransformPoint(&mx, &my, inv_mat, ctx.mouse->x, ctx.mouse->y);

    int x2 = x1 + width;
    int y2 = y1 + height;
    return (
        (ctx.x + x1 < mx) && (mx <= ctx.x + x2) &&
        (ctx.y + y1 < my) && (my <= ctx.y + y2)
    );
}

constexpr auto OFFSET_X = 8.0;
constexpr auto OFFSET_Y = 5.0;
constexpr auto RADIUS = 6.0;
constexpr auto SPACE_X = 8.0;

static float orange_x = 0;
static float orange_y = 0;
static bool orange_dragging = false;
static float orange_ix, orange_iy;

void draw_window(Context ctx, float x, float y, float width, float height) {

    constexpr auto BORDER_RADIUS = 4.0;
    auto BAR_COLOR_TOP = nvgRGB(0xe8, 0xe6, 0xe8);
    auto BAR_COLOR_BOTTOM = nvgRGB(0xd3, 0xd1, 0xd3);
    
    NVGpaint paint;
    
    // Shadow
    nvgSave(ctx.vg);
    float mat[6];
    nvgCurrentTransform(ctx.vg, mat);
    nvgResetTransform(ctx.vg);
    nvgTranslate(ctx.vg, 0, 40);
    nvgTransform(ctx.vg, mat[0], mat[1], mat[2], mat[3], mat[4], mat[5]);
    nvgBeginPath(ctx.vg);
    nvgRect(ctx.vg, x - 60, y - 60, width + 120, height + 120);
    paint = nvgBoxGradient(ctx.vg, x + 5, y + 5, width - 10, height - 10, 20, 60, nvgRGBA(0, 0, 0, 0x60), nvgRGBA(0, 0, 0, 0));
    nvgFillPaint(ctx.vg, paint);
    nvgFill(ctx.vg);
    nvgRestore(ctx.vg);
    
    // Background
    nvgBeginPath(ctx.vg);
    nvgFillColor(ctx.vg, nvgRGBf(0.949f, 0.949f, 0.949f));
    nvgRoundedRect(ctx.vg, x, y, width, height, BORDER_RADIUS);
    nvgFill(ctx.vg);

    // Title bar background
    nvgBeginPath(ctx.vg);
    nvgRoundedRectVarying(ctx.vg, x, y, width, BAR_HEIGHT, BORDER_RADIUS, BORDER_RADIUS, 0, 0);
    paint = nvgLinearGradient(ctx.vg, 0, y, 0, y + BAR_HEIGHT, BAR_COLOR_TOP, BAR_COLOR_BOTTOM);
    nvgFillPaint(ctx.vg, paint);
    nvgFill(ctx.vg);

    // Window outline
    nvgBeginPath(ctx.vg);
    nvgRoundedRect(ctx.vg, x, y, width, height, BORDER_RADIUS);
    nvgMoveTo(ctx.vg, x, y + BAR_HEIGHT);
    nvgLineTo(ctx.vg, x + width, y + BAR_HEIGHT);
    nvgStrokeColor(ctx.vg, nvgRGB(0xaa, 0xaa, 0xaa));
    nvgStrokeWidth(ctx.vg, 0.5);
    nvgStroke(ctx.vg);
    
    // Buttons
    auto cx = x + OFFSET_X + RADIUS;
    auto cy = y + OFFSET_Y + RADIUS;
    
    nvgBeginPath(ctx.vg);
    nvgCircle(ctx.vg, cx, cy, RADIUS);
    nvgFillColor(ctx.vg, nvgRGB(0xed, 0x6a, 0x5e));
    nvgStrokeColor(ctx.vg, nvgRGB(0xd0, 0x52, 0x46));
    nvgFill(ctx.vg);
    nvgStroke(ctx.vg);
    cx += 2 * SPACE_X + 4 * RADIUS;
    
    nvgBeginPath(ctx.vg);
    nvgCircle(ctx.vg, cx, cy, RADIUS);
    nvgFillColor(ctx.vg, nvgRGB(0x62, 0xc5, 0x54));
    nvgStrokeColor(ctx.vg, nvgRGB(0x55, 0xa7, 0x41));
    nvgFill(ctx.vg);
    nvgStroke(ctx.vg);
    cx -= SPACE_X + 2.0 * RADIUS;
    
    nvgBeginPath(ctx.vg);
    nvgCircle(ctx.vg, cx, cy, RADIUS);
    nvgFillColor(ctx.vg, nvgRGBA(0x00, 0x00, 0x00, 0x20));
    nvgFill(ctx.vg);
    
    nvgBeginPath(ctx.vg);
    nvgCircle(ctx.vg, orange_x + cx, orange_y + cy, RADIUS);
    nvgFillColor(ctx.vg, nvgRGB(0xf5, 0xbf, 0x50));
    nvgStrokeColor(ctx.vg, nvgRGB(0xd6, 0xa1, 0x43));
    nvgFill(ctx.vg);
    nvgStroke(ctx.vg);

}

static float letter_width = 0;
static float letter_height = 0;

void draw_letter(Context ctx, float x, float y) {

    nvgFontFace(ctx.vg, "times");
    nvgFontSize(ctx.vg, 200.0);
    
    float bounds[4];
    float ascender, descender, line_height;
    nvgTextMetrics(ctx.vg, &ascender, &descender, &line_height);
    nvgTextBounds(ctx.vg, 0, 0, "A", NULL, bounds);

    letter_width = bounds[2] - bounds[0];
    letter_height = ascender + descender + 1.0;
    
    nvgFillColor(ctx.vg, nvgRGB(0, 0, 0));
    nvgText(ctx.vg, x, y + letter_height, "A", NULL);

}

static auto window_dragging = false;
static auto letter_dragging = false;
static auto button_dragging = false;

static float window_x = 80;
static float window_y = 80;

static float letter_x, letter_y;

static float window_ix, window_iy, letter_ix, letter_iy;

static float window_width = 240;
static float window_height = 240;

static float rotation = 0;
static float irotation;
static float cx, cy;

void recenter(Context ctx) {
    float wx, wy, lx, ly;

    float mat[6];
    nvgCurrentTransform(ctx.vg, mat);
    nvgTransformPoint(&wx, &wy, mat, window_x, window_y);
    nvgTransformPoint(&lx, &ly, mat, letter_x, letter_y);
    nvgTransformPoint(&cx, &cy, mat, window_x + window_width / 2, window_y + window_height / 2);
    
    nvgResetTransform(ctx.vg);
    nvgTranslate(ctx.vg, cx, cy);
    nvgRotate(ctx.vg, rotation);
    nvgTranslate(ctx.vg, -cx, -cy);
    
    float mat_inv[6];
    nvgCurrentTransform(ctx.vg, mat);
    nvgTransformInverse(mat_inv, mat);
    nvgTransformPoint(&window_x, &window_y, mat_inv, wx, wy);
    nvgTransformPoint(&letter_x, &letter_y, mat_inv, lx, ly);
}

int image = 0;
int image_width, image_height;

void update(Context ctx) {

    if (!image) {
        image = nvgCreateImage(ctx.vg, "assets/background.jpg", 0);
        if (image == 0) {
            exit(1);
        }
        nvgImageSize(ctx.vg, image, &image_width, &image_height);
        image_width /= 2;
        image_height /= 2;
    }

    // Measure letter
    if (!letter_width) {
        draw_letter(ctx, 0, 0);
        letter_x = window_x + (window_width - letter_width) / 2.0;
        letter_y = window_y + (window_height - letter_height) / 2.0;
        cx = window_x + window_width / 2;
        cy = window_y + window_height / 2;
    }

    // Background
    nvgBeginPath(ctx.vg);
    nvgRect(ctx.vg, 0, 0, ctx.width, ctx.height);
    auto paint = nvgImagePattern(ctx.vg, 0, 0, image_width, image_height, 0, image, 1);
    nvgFillPaint(ctx.vg, paint);
    nvgFill(ctx.vg);
    
    nvgSave(ctx.vg);

    nvgTranslate(ctx.vg, cx, cy);
    nvgRotate(ctx.vg, rotation);
    nvgTranslate(ctx.vg, -cx, -cy);
    
    // Get proper dx
    float dx = 0;
    float dy = 0;
    if (ctx.mouse->accepted) {
        float mat[6], inv_mat[6];
        nvgCurrentTransform(ctx.vg, mat);
        nvgTransformInverse(inv_mat, mat);
        float x, y, ix, iy;
        nvgTransformPoint(&x, &y, inv_mat, ctx.mouse->x, ctx.mouse->y);
        nvgTransformPoint(&ix, &iy, inv_mat, ctx.mouse->initial_x, ctx.mouse->initial_y);
        dx = x - ix;
        dy = y - iy;
    }
    
    // Button dragging
    if (button_dragging && !ctx.mouse->pressed) {
        button_dragging = false;
    }
    if (hit(ctx, window_x + OFFSET_X, window_y + OFFSET_Y, 2 * RADIUS, 2 * RADIUS)) {
        ctx.mouse->accepted = true;
        button_dragging = true;
        irotation = rotation;
    }
    if (button_dragging) {
    
        float x1 = ctx.mouse->initial_x - cx;
        float y1 = ctx.mouse->initial_y - cy;
        float x2 = ctx.mouse->x - cx;
        float y2 = ctx.mouse->y - cy;

        auto a = std::atan(y2 / x2) - std::atan(y1 / x1);
        if (x1 >= 0) a += NVG_PI;
        if (x2 >= 0) a += NVG_PI;
        
        rotation = irotation + a;
    }
    
    // Button dragging
    if (orange_dragging && !ctx.mouse->pressed) {
        orange_dragging = false;
    }
    if (hit(ctx, window_x + OFFSET_X + 2 * RADIUS + SPACE_X + orange_x, window_y + OFFSET_Y + orange_y, 2 * RADIUS, 2 * RADIUS)) {
        ctx.mouse->accepted = true;
        orange_dragging = true;
        orange_ix = orange_x;
        orange_iy = orange_y;
    }
    if (orange_dragging) {
        orange_x = orange_ix + dx;
        orange_y = orange_iy + dy;
    }

    // Window dragging
    if (window_dragging && !ctx.mouse->pressed) {
        window_dragging = false;
        recenter(ctx);
    }
    if (hit(ctx, window_x, window_y, window_width, BAR_HEIGHT)) {
        ctx.mouse->accepted = true;
        window_dragging = true;
        window_ix = window_x;
        window_iy = window_y;
        letter_ix = letter_x;
        letter_iy = letter_y;
    }
    if (window_dragging) {
        window_x = window_ix + dx;
        window_y = window_iy + dy;
        letter_x = letter_ix + dx;
        letter_y = letter_iy + dy;
    }
    
    // Letter dragging
    if (letter_dragging && !ctx.mouse->pressed) {
        letter_dragging = false;
        recenter(ctx);
    }
    if (hit(ctx, letter_x, letter_y, letter_width, letter_height)) {
        ctx.mouse->accepted = true;
        letter_dragging = true;
        letter_ix = letter_x;
        letter_iy = letter_y;
    }
    if (letter_dragging) {
        *ctx.cursor = CURSOR_CLOSED_HAND;
        letter_x = letter_ix + dx;
        letter_y = letter_iy + dy;
        
        // Make sure window fits around letter
        if (window_x > letter_x) {
            window_x = letter_x;
        }
        if (window_y > letter_y - BAR_HEIGHT) {
            window_y = letter_y - BAR_HEIGHT;
        }
        if (window_x < letter_x + letter_width - window_width) {
            window_x = letter_x + letter_width - window_width;
        }
        if (window_y < letter_y + letter_height - window_height) {
            window_y = letter_y + letter_height - window_height;
        }
    }
    if (over(ctx, letter_x, letter_y, letter_width, letter_height)) {
        *ctx.cursor = CURSOR_OPEN_HAND;
    }
    
    // Window
    draw_window(ctx, window_x, window_y, window_width, window_height);
    
    // Letter
    draw_letter(ctx, letter_x, letter_y);
    
    nvgRestore(ctx.vg);

}

int main(int argc, const char** argv) {

    app::init("Weekend Project: Input Tracking");

    app::load_font_face("regular",        "assets/SFRegular.ttf");
    app::load_font_face("medium",         "assets/SFMedium.ttf");
    app::load_font_face("bold",           "assets/SFBold.ttf");
    app::load_font_face("regular-italic", "assets/SFRegularItalic.ttf");
    app::load_font_face("medium-italic",  "assets/SFMediumItalic.ttf");
    app::load_font_face("bold-italic",    "assets/SFBoldItalic.ttf");
    app::load_font_face("times",          "assets/TNRBold.ttf");

    app::run(&update);

    return 0;
}

