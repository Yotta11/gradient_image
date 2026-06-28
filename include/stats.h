
#ifndef STATS_H
#define STATS_H

#include "image_io.h"
#include "gradient.h"


typedef struct {
    float   min;
    float   max;
    float   mean;
    float   std_dev;
    long    histogram[256]; 
    int     n_pixels;
} ImageStats;


// Calcule les statistiques d'une image en niveaux de gris.
 
ImageStats stats_compute_gray(const ImageGray *img);
// Calcule les statistiques de la magnitude d'un gradient.
 
ImageStats stats_compute_gradient(const GradientResult *gr);

// Affiche un résumé textuel des statistiques.
 
void stats_print(const ImageStats *st, const char *label);

#endif 
