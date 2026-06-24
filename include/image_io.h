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

/**
 * @brief Image en niveaux de gris (PGM)
 */
typedef struct {
    int     width;       /**< Largeur en pixels                    */
    int     height;      /**< Hauteur en pixels                    */
    int     maxval;      /**< Valeur maximale (255 en général)     */
    uint8_t *data;       /**< Tableau plat row-major [height*width] */
} ImageGray;


typedef struct {
    int     width;
    int     height;
    int     maxval;
    uint8_t *data;       /**< Tableau plat [height * width * 3], ordre R G B */
} ImageRGB;


ImageGray *pgm_read(const char *filename);


int pgm_write(const char *filename, const ImageGray *img);


ImageRGB *ppm_read(const char *filename);


int ppm_write(const char *filename, const ImageRGB *img);


ImageGray *rgb_to_gray(const ImageRGB *rgb);

void image_gray_free(ImageGray *img);


void image_rgb_free(ImageRGB *img);

#endif /* IMAGE_IO_H */
