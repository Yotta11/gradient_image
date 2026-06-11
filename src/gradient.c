/*
 * gradient.c — Opérateurs de gradient : Sobel, Prewitt, Roberts
 */

#define _POSIX_C_SOURCE 200809L


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "gradient.h"

/* Accès sûr avec miroir aux bords */
static inline uint8_t pixel(const ImageGray *img, int r, int c)
{
    if (r < 0) r = -r;
    if (c < 0) c = -c;
    if (r >= img->height) r = 2 * img->height - 2 - r;
    if (c >= img->width)  c = 2 * img->width  - 2 - c;
    return img->data[r * img->width + c];
}

/* Allocation d'un GradientResult */
static GradientResult *gradient_alloc(int w, int h)
{
    GradientResult *gr = (GradientResult *)malloc(sizeof(GradientResult));
    if (!gr) return NULL;
    gr->width  = w;
    gr->height = h;
    int n = w * h;
    gr->Gx        = (float *)malloc((size_t)n * sizeof(float));
    gr->Gy        = (float *)malloc((size_t)n * sizeof(float));
    gr->magnitude = (float *)malloc((size_t)n * sizeof(float));
    gr->direction = (float *)malloc((size_t)n * sizeof(float));
    if (!gr->Gx || !gr->Gy || !gr->magnitude || !gr->direction) {
        gradient_free(gr);
        return NULL;
    }
    gr->max_mag  = 0.0f;
    gr->mean_mag = 0.0f;
    return gr;
}

void gradient_free(GradientResult *gr)
{
    if (gr) {
        free(gr->Gx);
        free(gr->Gy);
        free(gr->magnitude);
        free(gr->direction);
        free(gr);
    }
}

/*
 * Opérateur de Sobel (noyaux 3x3)
 *   Kx = [-1  0 +1]     Ky = [-1 -2 -1]
 *        [-2  0 +2]          [ 0  0  0]
 *        [-1  0 +1]          [+1 +2 +1]
 */
static void apply_sobel(const ImageGray *img, GradientResult *gr)
{
    int W = img->width, H = img->height;
    for (int r = 0; r < H; r++) {
        for (int c = 0; c < W; c++) {
            float p00 = pixel(img, r-1, c-1);
            float p01 = pixel(img, r-1, c  );
            float p02 = pixel(img, r-1, c+1);
            float p10 = pixel(img, r  , c-1);
            float p12 = pixel(img, r  , c+1);
            float p20 = pixel(img, r+1, c-1);
            float p21 = pixel(img, r+1, c  );
            float p22 = pixel(img, r+1, c+1);

            float gx = (-p00 + p02) + (-2.0f*p10 + 2.0f*p12) + (-p20 + p22);
            float gy = (-p00 - 2.0f*p01 - p02) + (p20 + 2.0f*p21 + p22);

            int idx = r * W + c;
            gr->Gx[idx] = gx;
            gr->Gy[idx] = gy;
        }
    }
}

/*
 * Opérateur de Prewitt (noyaux 3x3)
 *   Kx = [-1  0 +1]     Ky = [-1 -1 -1]
 *        [-1  0 +1]          [ 0  0  0]
 *        [-1  0 +1]          [+1 +1 +1]
 */
static void apply_prewitt(const ImageGray *img, GradientResult *gr)
{
    int W = img->width, H = img->height;
    for (int r = 0; r < H; r++) {
        for (int c = 0; c < W; c++) {
            float p00 = pixel(img, r-1, c-1);
            float p01 = pixel(img, r-1, c  );
            float p02 = pixel(img, r-1, c+1);
            float p10 = pixel(img, r  , c-1);
            float p12 = pixel(img, r  , c+1);
            float p20 = pixel(img, r+1, c-1);
            float p21 = pixel(img, r+1, c  );
            float p22 = pixel(img, r+1, c+1);

            float gx = (-p00 + p02) + (-p10 + p12) + (-p20 + p22);
            float gy = (-p00 - p01 - p02) + (p20 + p21 + p22);

            int idx = r * W + c;
            gr->Gx[idx] = gx;
            gr->Gy[idx] = gy;
        }
    }
}

/*
 * Opérateur de Roberts (noyaux 2x2 diagonaux)
 *   Kx = [+1  0]     Ky = [ 0 +1]
 *        [ 0 -1]          [-1  0]
 */
static void apply_roberts(const ImageGray *img, GradientResult *gr)
{
    int W = img->width, H = img->height;
    for (int r = 0; r < H; r++) {
        for (int c = 0; c < W; c++) {
            float p00 = pixel(img, r  , c  );
            float p01 = pixel(img, r  , c+1);
            float p10 = pixel(img, r+1, c  );
            float p11 = pixel(img, r+1, c+1);

            float gx = p00 - p11;
            float gy = p01 - p10;

            int idx = r * W + c;
            gr->Gx[idx] = gx;
            gr->Gy[idx] = gy;
        }
    }
}

/* Calcule magnitude + direction et normalise */
static void compute_magnitude_direction(GradientResult *gr)
{
    int n       = gr->width * gr->height;
    float max_m = 0.0f;
    double sum  = 0.0;

    for (int i = 0; i < n; i++) {
        float m = sqrtf(gr->Gx[i] * gr->Gx[i] + gr->Gy[i] * gr->Gy[i]);
        gr->magnitude[i] = m;
        gr->direction[i] = atan2f(gr->Gy[i], gr->Gx[i]) * 180.0f / (float)M_PI;
        if (m > max_m) max_m = m;
        sum += m;
    }

    gr->max_mag  = max_m;
    gr->mean_mag = (float)(sum / n);

    if (max_m > 0.0f) {
        float scale = 255.0f / max_m;
        for (int i = 0; i < n; i++)
            gr->magnitude[i] *= scale;
    }
}

/* Point d'entrée public */
GradientResult *gradient_compute(const ImageGray *img, GradientOperator op)
{
    if (!img || !img->data) {
        fprintf(stderr, "[gradient_compute] Image source invalide\n");
        return NULL;
    }

    GradientResult *gr = gradient_alloc(img->width, img->height);
    if (!gr) return NULL;

    switch (op) {
        case GRAD_SOBEL:   apply_sobel(img, gr);   break;
        case GRAD_PREWITT: apply_prewitt(img, gr); break;
        case GRAD_ROBERTS: apply_roberts(img, gr); break;
        default:
            fprintf(stderr, "[gradient_compute] Operateur inconnu\n");
            gradient_free(gr);
            return NULL;
    }

    compute_magnitude_direction(gr);
    return gr;
}

/* Conversions vers ImageGray */
ImageGray *gradient_magnitude_to_image(const GradientResult *gr)
{
    if (!gr) return NULL;
    ImageGray *img = (ImageGray *)malloc(sizeof(ImageGray));
    if (!img) return NULL;
    img->width  = gr->width;
    img->height = gr->height;
    img->maxval = 255;
    int n = gr->width * gr->height;
    img->data = (uint8_t *)malloc((size_t)n);
    if (!img->data) { free(img); return NULL; }
    for (int i = 0; i < n; i++)
        img->data[i] = (uint8_t)(gr->magnitude[i] + 0.5f);
    return img;
}

ImageGray *gradient_direction_to_image(const GradientResult *gr)
{
    if (!gr) return NULL;
    ImageGray *img = (ImageGray *)malloc(sizeof(ImageGray));
    if (!img) return NULL;
    img->width  = gr->width;
    img->height = gr->height;
    img->maxval = 255;
    int n = gr->width * gr->height;
    img->data = (uint8_t *)malloc((size_t)n);
    if (!img->data) { free(img); return NULL; }
    for (int i = 0; i < n; i++) {
        float d = (gr->direction[i] + 180.0f) / 360.0f * 255.0f;
        img->data[i] = (uint8_t)(d + 0.5f);
    }
    return img;
}

ImageGray *gradient_gx_to_image(const GradientResult *gr)
{
    if (!gr) return NULL;
    ImageGray *img = (ImageGray *)malloc(sizeof(ImageGray));
    if (!img) return NULL;
    img->width  = gr->width;
    img->height = gr->height;
    img->maxval = 255;
    int n = gr->width * gr->height;
    img->data = (uint8_t *)malloc((size_t)n);
    if (!img->data) { free(img); return NULL; }

    float mn = FLT_MAX, mx = -FLT_MAX;
    for (int i = 0; i < n; i++) {
        if (gr->Gx[i] < mn) mn = gr->Gx[i];
        if (gr->Gx[i] > mx) mx = gr->Gx[i];
    }
    float rng = mx - mn;
    for (int i = 0; i < n; i++) {
        float v = (rng > 0.0f) ? (gr->Gx[i] - mn) / rng * 255.0f : 128.0f;
        img->data[i] = (uint8_t)(v + 0.5f);
    }
    return img;
}

ImageGray *gradient_gy_to_image(const GradientResult *gr)
{
    if (!gr) return NULL;
    ImageGray *img = (ImageGray *)malloc(sizeof(ImageGray));
    if (!img) return NULL;
    img->width  = gr->width;
    img->height = gr->height;
    img->maxval = 255;
    int n = gr->width * gr->height;
    img->data = (uint8_t *)malloc((size_t)n);
    if (!img->data) { free(img); return NULL; }

    float mn = FLT_MAX, mx = -FLT_MAX;
    for (int i = 0; i < n; i++) {
        if (gr->Gy[i] < mn) mn = gr->Gy[i];
        if (gr->Gy[i] > mx) mx = gr->Gy[i];
    }
    float rng = mx - mn;
    for (int i = 0; i < n; i++) {
        float v = (rng > 0.0f) ? (gr->Gy[i] - mn) / rng * 255.0f : 128.0f;
        img->data[i] = (uint8_t)(v + 0.5f);
    }
    return img;
}

const char *gradient_operator_name(GradientOperator op)
{
    switch (op) {
        case GRAD_SOBEL:   return "Sobel";
        case GRAD_PREWITT: return "Prewitt";
        case GRAD_ROBERTS: return "Roberts";
        default:           return "Inconnu";
    }
}
