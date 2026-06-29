

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "stats.h"

/* ------------------------------------------------------------------ */
/*  Statistiques sur une image en niveaux de gris                      */
/* ------------------------------------------------------------------ */

ImageStats stats_compute_gray(const ImageGray *img)
{
    ImageStats st;
    memset(&st, 0, sizeof(st));
    if (!img || !img->data) return st;

    int n = img->width * img->height;
    st.n_pixels = n;

    float mn = 255.0f, mx = 0.0f;
    double sum = 0.0, sum2 = 0.0;

    for (int i = 0; i < n; i++) {
        float v = (float)img->data[i];
        if (v < mn) mn = v;
        if (v > mx) mx = v;
        sum  += v;
        sum2 += v * v;
        st.histogram[(int)v]++;
    }

    st.min     = mn;
    st.max     = mx;
    st.mean    = (float)(sum / n);
    float var  = (float)(sum2 / n - (sum / n) * (sum / n));
    st.std_dev = sqrtf(var > 0.0f ? var : 0.0f);
    return st;
}

/* ------------------------------------------------------------------ */
/*  Statistiques sur la magnitude d'un gradient                        */
/* ------------------------------------------------------------------ */

ImageStats stats_compute_gradient(const GradientResult *gr)
{
    ImageStats st;
    memset(&st, 0, sizeof(st));
    if (!gr || !gr->magnitude) return st;

    int n = gr->width * gr->height;
    st.n_pixels = n;

    float mn = 255.0f, mx = 0.0f;
    double sum = 0.0, sum2 = 0.0;

    for (int i = 0; i < n; i++) {
        float v = gr->magnitude[i];
        if (v < mn) mn = v;
        if (v > mx) mx = v;
        sum  += v;
        sum2 += v * v;
        int bin = (int)(v + 0.5f);
        if (bin > 255) bin = 255;
        if (bin < 0)   bin = 0;
        st.histogram[bin]++;
    }

    st.min     = mn;
    st.max     = mx;
    st.mean    = (float)(sum / n);
    float var  = (float)(sum2 / n - (sum / n) * (sum / n));
    st.std_dev = sqrtf(var > 0.0f ? var : 0.0f);
    return st;
}


void stats_print(const ImageStats *st, const char *label)
{
    printf("  [%s]\n", label);
    printf("    Pixels     : %d\n",  st->n_pixels);
    printf("    Min        : %.2f\n", st->min);
    printf("    Max        : %.2f\n", st->max);
    printf("    Moyenne    : %.2f\n", st->mean);
    printf("    Ecart-type : %.2f\n", st->std_dev);

    /* Affichage d'un histogramme condensé (16 bins de 16 niveaux chacun) */
    printf("    Histogramme (16 bins) :\n");
    for (int b = 0; b < 16; b++) {
        long cnt = 0;
        for (int k = b*16; k < (b+1)*16; k++)
            cnt += st->histogram[k];
        int bar = (int)(cnt * 40.0 / st->n_pixels);
        printf("      %3d–%3d | ", b*16, b*16+15);
        for (int k = 0; k < bar; k++) putchar('#');
        printf(" %ld\n", cnt);
    }
}
