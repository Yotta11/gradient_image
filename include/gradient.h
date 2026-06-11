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

/* ------------------------------------------------------------------ */
/*  Structure de résultat                                               */
/* ------------------------------------------------------------------ */

/**
 * @brief Résultats complets d'un calcul de gradient.
 *
 * Toutes les images sont de mêmes dimensions (width × height).
 * Les données float permettent des calculs intermédiaires précis.
 */
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

/* ------------------------------------------------------------------ */
/*  Opérateurs disponibles                                              */
/* ------------------------------------------------------------------ */

typedef enum {
    GRAD_SOBEL   = 0,
    GRAD_PREWITT = 1,
    GRAD_ROBERTS = 2
} GradientOperator;

/* ------------------------------------------------------------------ */
/*  Prototypes                                                          */
/* ------------------------------------------------------------------ */

/**
 * @brief Calcule le gradient d'une image en niveaux de gris.
 *
 * @param img  Image source (PGM).
 * @param op   Opérateur choisi (SOBEL, PREWITT ou ROBERTS).
 * @return GradientResult alloué dynamiquement, NULL en cas d'erreur.
 */
GradientResult *gradient_compute(const ImageGray *img, GradientOperator op);

/**
 * @brief Convertit la magnitude en ImageGray (pour sauvegarde PGM).
 * @param gr  Résultat du gradient.
 * @return ImageGray allouée, NULL en cas d'erreur.
 */
ImageGray *gradient_magnitude_to_image(const GradientResult *gr);

/**
 * @brief Convertit la direction en ImageGray (visualisation couleur ramenée
 *        à un gris : [−180°,+180°] → [0,255]).
 */
ImageGray *gradient_direction_to_image(const GradientResult *gr);

/**
 * @brief Convertit Gx en image pour visualisation (centré sur 128).
 */
ImageGray *gradient_gx_to_image(const GradientResult *gr);

/**
 * @brief Convertit Gy en image pour visualisation (centré sur 128).
 */
ImageGray *gradient_gy_to_image(const GradientResult *gr);

/**
 * @brief Libère la mémoire d'un GradientResult.
 */
void gradient_free(GradientResult *gr);

/**
 * @brief Retourne le nom textuel de l'opérateur.
 */
const char *gradient_operator_name(GradientOperator op);

#endif /* GRADIENT_H */
