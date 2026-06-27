/**
 * @file gradient.h
 * @brief Module de calcul du gradient d'images en niveaux de gris.
 *
 * Trois opérateurs sont implémentés :
 *   - Sobel   (noyaux 3×3, pondération centrale)
 *   - Prewitt (noyaux 3×3, différences finies simples)
 *   - Roberts (noyaux 2×2, diagonales)
 *
 * Pour chaque opérateur, on calcule :
 *   Gx  : composante horizontale
 *   Gy  : composante verticale
 *   G   : magnitude  G = sqrt(Gx² + Gy²)  (normalisée dans [0,255])
 *   Dir : direction  θ = atan2(Gy, Gx)     (en degrés)
 */

#ifndef GRADIENT_H
#define GRADIENT_H

#include "image_io.h"

/
typedef struct {
    int    width;
    int    height;
    float *Gx;        /**< Gradient horizontal (float brut)            */
    float *Gy;        /**< Gradient vertical   (float brut)            */
    float *magnitude; /**< Magnitude normalisée [0.0, 255.0]           */
    float *direction; /**< Direction en degrés [-180°, +180°]          */
    float  max_mag;   /**< Magnitude maximale avant normalisation      */
    float  mean_mag;  /**< Magnitude moyenne                           */
} GradientResult;


typedef enum {
    GRAD_SOBEL   = 0,
    GRAD_PREWITT = 1,
    GRAD_ROBERTS = 2
} GradientOperator;


GradientResult *gradient_compute(const ImageGray *img, GradientOperator op);

ImageGray *gradient_magnitude_to_image(const GradientResult *gr);


ImageGray *gradient_direction_to_image(const GradientResult *gr);


ImageGray *gradient_gx_to_image(const GradientResult *gr);

ImageGray *gradient_gy_to_image(const GradientResult *gr);

void gradient_free(GradientResult *gr);


const char *gradient_operator_name(GradientOperator op);

#endif 
