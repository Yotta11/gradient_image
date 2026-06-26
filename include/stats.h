/**
 
 * Fournit : min, max, moyenne, écart-type, histogramme.
 */

#ifndef STATS_H
#define STATS_H

#include "image_io.h"
#include "gradient.h"

/* ------------------------------------------------------------------ */
/*  Structure de statistiques                                           */
/* ------------------------------------------------------------------ */

typedef struct {
    float   min;
    float   max;
    float   mean;
    float   std_dev;
    long    histogram[256];   /**< Histogramme des valeurs entières [0..255] */
    int     n_pixels;
} ImageStats;
//prototypes

// Calcule les statistiques d'une image en niveaux de gris.
 
ImageStats stats_compute_gray(const ImageGray *img);
// Calcule les statistiques de la magnitude d'un gradient.
 
ImageStats stats_compute_gradient(const GradientResult *gr);

//f Affiche un résumé textuel des statistiques.
 
void stats_print(const ImageStats *st, const char *label);

#endif 
