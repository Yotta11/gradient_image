/**
 * @file image_io.h
 * @brief Module de lecture/écriture d'images PGM (niveaux de gris) et PPM (couleur)
 *
 * Supporte les formats binaires :
 *   - P5 : PGM binaire (grayscale)
 *   - P6 : PPM binaire (RGB couleur)
 */

#ifndef IMAGE_IO_H
#define IMAGE_IO_H

#include <stdint.h>

/* ------------------------------------------------------------------ */
/*  Structures de données                                               */
/* ------------------------------------------------------------------ */

/**
 * @brief Image en niveaux de gris (PGM)
 */
typedef struct {
    int     width;       /**< Largeur en pixels                    */
    int     height;      /**< Hauteur en pixels                    */
    int     maxval;      /**< Valeur maximale (255 en général)     */
    uint8_t *data;       /**< Tableau plat row-major [height*width] */
} ImageGray;

/**
 * @brief Image couleur RGB (PPM)
 */
typedef struct {
    int     width;
    int     height;
    int     maxval;
    uint8_t *data;       /**< Tableau plat [height * width * 3], ordre R G B */
} ImageRGB;

/* ------------------------------------------------------------------ */
/*  Prototypes                                                          */
/* ------------------------------------------------------------------ */

/**
 * @brief Lit une image PGM (P5) depuis un fichier.
 * @param filename Chemin du fichier source.
 * @return Pointeur sur une ImageGray allouée dynamiquement, NULL en cas d'erreur.
 */
ImageGray *pgm_read(const char *filename);

/**
 * @brief Écrit une image PGM (P5) dans un fichier.
 * @param filename  Chemin du fichier de destination.
 * @param img       Image à écrire.
 * @return 0 en cas de succès, -1 en cas d'erreur.
 */
int pgm_write(const char *filename, const ImageGray *img);

/**
 * @brief Lit une image PPM (P6) depuis un fichier.
 * @param filename Chemin du fichier source.
 * @return Pointeur sur une ImageRGB allouée dynamiquement, NULL en cas d'erreur.
 */
ImageRGB *ppm_read(const char *filename);

/**
 * @brief Écrit une image PPM (P6) dans un fichier.
 * @param filename Chemin du fichier destination.
 * @param img      Image à écrire.
 * @return 0 en cas de succès, -1 en cas d'erreur.
 */
int ppm_write(const char *filename, const ImageRGB *img);

/**
 * @brief Convertit une image RGB en niveaux de gris (luminance standard).
 *        Formule : Y = 0.299*R + 0.587*G + 0.114*B
 * @param rgb Image source RGB.
 * @return Nouvelle ImageGray allouée, NULL en cas d'erreur.
 */
ImageGray *rgb_to_gray(const ImageRGB *rgb);

/**
 * @brief Libère la mémoire d'une ImageGray.
 */
void image_gray_free(ImageGray *img);

/**
 * @brief Libère la mémoire d'une ImageRGB.
 */
void image_rgb_free(ImageRGB *img);

#endif /* IMAGE_IO_H */
