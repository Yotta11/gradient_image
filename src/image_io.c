/**
 * @file image_io.c
 * @brief Implémentation de la lecture/écriture PGM et PPM binaires.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "image_io.h"

/* ------------------------------------------------------------------ */
/*  Fonctions auxiliaires internes                                      */
/* ------------------------------------------------------------------ */

/**
 * @brief Saute les commentaires et espaces dans un entête PNM.
 *        Un commentaire commence par '#' et finit à la fin de ligne.
 */
static void skip_whitespace_and_comments(FILE *fp)
{
    int c;
    while ((c = fgetc(fp)) != EOF) {
        if (c == '#') {
            /* Consomme jusqu'à la fin de ligne */
            while ((c = fgetc(fp)) != EOF && c != '\n')
                ;
        } else if (!isspace((unsigned char)c)) {
            ungetc(c, fp);
            return;
        }
    }
}

/**
 * @brief Lit un entier décimal depuis le flux (après avoir sauté
 *        commentaires et espaces).
 */
static int read_int(FILE *fp)
{
    int v = 0;
    skip_whitespace_and_comments(fp);
    int c;
    while ((c = fgetc(fp)) != EOF && isdigit((unsigned char)c))
        v = v * 10 + (c - '0');
    if (c != EOF) ungetc(c, fp);
    return v;
}

/* ------------------------------------------------------------------ */
/*  Lecture PGM (P5)                                                    */
/* ------------------------------------------------------------------ */

ImageGray *pgm_read(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "[pgm_read] Impossible d'ouvrir '%s'\n", filename);
        return NULL;
    }

    /* Vérification du magic number */
    char magic[3];
    if (!fgets(magic, sizeof(magic), fp) || magic[0] != 'P' || magic[1] != '5') {
        fprintf(stderr, "[pgm_read] Format non-PGM P5 dans '%s'\n", filename);
        fclose(fp);
        return NULL;
    }

    /* Lecture entête */
    int width  = read_int(fp);
    int height = read_int(fp);
    int maxval = read_int(fp);

    if (width <= 0 || height <= 0 || maxval <= 0 || maxval > 255) {
        fprintf(stderr, "[pgm_read] Entête invalide dans '%s'\n", filename);
        fclose(fp);
        return NULL;
    }

    /* Consomme l'unique espace/newline après maxval */
    fgetc(fp);

    /* Allocation */
    ImageGray *img = (ImageGray *)malloc(sizeof(ImageGray));
    if (!img) { fclose(fp); return NULL; }

    img->width  = width;
    img->height = height;
    img->maxval = maxval;
    img->data   = (uint8_t *)malloc((size_t)width * height);
    if (!img->data) {
        free(img);
        fclose(fp);
        return NULL;
    }

    /* Lecture des données binaires */
    size_t read = fread(img->data, 1, (size_t)width * height, fp);
    if (read != (size_t)width * height) {
        fprintf(stderr, "[pgm_read] Données incomplètes dans '%s' (%zu/%d)\n",
                filename, read, width * height);
        free(img->data);
        free(img);
        fclose(fp);
        return NULL;
    }

    fclose(fp);
    return img;
}

/* ------------------------------------------------------------------ */
/*  Écriture PGM (P5)                                                   */
/* ------------------------------------------------------------------ */

int pgm_write(const char *filename, const ImageGray *img)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "[pgm_write] Impossible de créer '%s'\n", filename);
        return -1;
    }
    fprintf(fp, "P5\n%d %d\n%d\n", img->width, img->height, img->maxval);
    fwrite(img->data, 1, (size_t)img->width * img->height, fp);
    fclose(fp);
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Lecture PPM (P6)                                                    */
/* ------------------------------------------------------------------ */

ImageRGB *ppm_read(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "[ppm_read] Impossible d'ouvrir '%s'\n", filename);
        return NULL;
    }

    char magic[3];
    if (!fgets(magic, sizeof(magic), fp) || magic[0] != 'P' || magic[1] != '6') {
        fprintf(stderr, "[ppm_read] Format non-PPM P6 dans '%s'\n", filename);
        fclose(fp);
        return NULL;
    }

    int width  = read_int(fp);
    int height = read_int(fp);
    int maxval = read_int(fp);
    fgetc(fp);  /* consomme le séparateur final */

    if (width <= 0 || height <= 0 || maxval <= 0 || maxval > 255) {
        fprintf(stderr, "[ppm_read] Entête invalide dans '%s'\n", filename);
        fclose(fp);
        return NULL;
    }

    ImageRGB *img = (ImageRGB *)malloc(sizeof(ImageRGB));
    if (!img) { fclose(fp); return NULL; }

    img->width  = width;
    img->height = height;
    img->maxval = maxval;
    img->data   = (uint8_t *)malloc((size_t)width * height * 3);
    if (!img->data) { free(img); fclose(fp); return NULL; }

    size_t read = fread(img->data, 1, (size_t)width * height * 3, fp);
    if (read != (size_t)width * height * 3) {
        fprintf(stderr, "[ppm_read] Données incomplètes dans '%s'\n", filename);
        free(img->data);
        free(img);
        fclose(fp);
        return NULL;
    }

    fclose(fp);
    return img;
}

/* ------------------------------------------------------------------ */
/*  Écriture PPM (P6)                                                   */
/* ------------------------------------------------------------------ */

int ppm_write(const char *filename, const ImageRGB *img)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "[ppm_write] Impossible de créer '%s'\n", filename);
        return -1;
    }
    fprintf(fp, "P6\n%d %d\n%d\n", img->width, img->height, img->maxval);
    fwrite(img->data, 1, (size_t)img->width * img->height * 3, fp);
    fclose(fp);
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Conversion RGB → Gris                                              */
/* ------------------------------------------------------------------ */

ImageGray *rgb_to_gray(const ImageRGB *rgb)
{
    if (!rgb) return NULL;
    ImageGray *gray = (ImageGray *)malloc(sizeof(ImageGray));
    if (!gray) return NULL;

    gray->width  = rgb->width;
    gray->height = rgb->height;
    gray->maxval = rgb->maxval;
    gray->data   = (uint8_t *)malloc((size_t)rgb->width * rgb->height);
    if (!gray->data) { free(gray); return NULL; }

    int n = rgb->width * rgb->height;
    for (int i = 0; i < n; i++) {
        /* Formule de luminance BT.601 */
        float r = rgb->data[3*i + 0];
        float g = rgb->data[3*i + 1];
        float b = rgb->data[3*i + 2];
        float y = 0.299f * r + 0.587f * g + 0.114f * b;
        gray->data[i] = (uint8_t)(y + 0.5f);
    }
    return gray;
}

/* ------------------------------------------------------------------ */
/*  Libération mémoire                                                  */
/* ------------------------------------------------------------------ */

void image_gray_free(ImageGray *img)
{
    if (img) {
        free(img->data);
        free(img);
    }
}

void image_rgb_free(ImageRGB *img)
{
    if (img) {
        free(img->data);
        free(img);
    }
}
