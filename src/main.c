

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <sys/stat.h>
#include "image_io.h"
#include "gradient.h"
#include "stats.h"

#define MAX_PATH 2200

static const GradientOperator OPERATORS[] = {
    GRAD_SOBEL, GRAD_PREWITT, GRAD_ROBERTS
};
static const int N_OPERATORS = 3;

/* Retourne l'extension d'un nom de fichier */
static const char *file_extension(const char *name)
{
    const char *dot = strrchr(name, '.');
    return dot ? dot + 1 : "";
}

/* Nom de base sans extension */
static void base_name(const char *name, char *out, int max_len)
{
    const char *dot = strrchr(name, '.');
    int len = dot ? (int)(dot - name) : (int)strlen(name);
    if (len >= max_len) len = max_len - 1;
    strncpy(out, name, len);
    out[len] = '\0';
}

/* Traitement d'une image : applique les 3 opérateurs et sauvegarde */
static void process_image(const char *img_path,
                           const char *base,
                           const char *out_dir,
                           int is_ppm)
{
    printf("\n========================================\n");
    printf("Image : %s\n", img_path);
    printf("========================================\n");

    ImageGray *gray = NULL;
    if (is_ppm) {
        ImageRGB *rgb = ppm_read(img_path);
        if (!rgb) {
            fprintf(stderr, "  ERREUR: Lecture PPM echouee.\n");
            return;
        }
        printf("  Dimensions : %d x %d (PPM couleur -> conversion gris)\n",
               rgb->width, rgb->height);
        gray = rgb_to_gray(rgb);
        image_rgb_free(rgb);
    } else {
        gray = pgm_read(img_path);
        if (!gray) {
            fprintf(stderr, "  ERREUR: Lecture PGM echouee.\n");
            return;
        }
        printf("  Dimensions : %d x %d (PGM niveaux de gris)\n",
               gray->width, gray->height);
    }

    if (!gray) {
        fprintf(stderr, "  ERREUR: Conversion impossible.\n");
        return;
    }

    /* Statistiques de l'image source */
    ImageStats src_stats = stats_compute_gray(gray);
    printf("\n  --- Statistiques image source ---\n");
    stats_print(&src_stats, "Source");

    /* Application de chaque opérateur */
    for (int oi = 0; oi < N_OPERATORS; oi++) {
        GradientOperator op     = OPERATORS[oi];
        const char      *opname = gradient_operator_name(op);

        printf("\n  --- Operateur : %s ---\n", opname);

        GradientResult *gr = gradient_compute(gray, op);
        if (!gr) {
            fprintf(stderr, "  ERREUR: Calcul gradient (%s) echoue.\n", opname);
            continue;
        }

        ImageStats gst = stats_compute_gradient(gr);
        printf("  Magnitude max (avant normalisation) : %.2f\n", gr->max_mag);
        printf("  Magnitude moyenne (avant norm.)     : %.2f\n", gr->mean_mag);
        stats_print(&gst, "Magnitude normalisee");

        /* Nom opérateur en minuscules */
        char op_lower[16];
        int j = 0;
        for (const char *p = opname; *p && j < 15; p++, j++)
            op_lower[j] = (*p >= 'A' && *p <= 'Z') ? (*p + 32) : *p;
        op_lower[j] = '\0';

        char path_out[MAX_PATH];

        /* Magnitude */
        ImageGray *img_mag = gradient_magnitude_to_image(gr);
        snprintf(path_out, sizeof(path_out),
                 "%s/%s_%s_magnitude.pgm", out_dir, base, op_lower);
        if (img_mag) {
            pgm_write(path_out, img_mag);
            printf("  Sauvegarde : %s\n", path_out);
            image_gray_free(img_mag);
        }

        /* Direction */
        ImageGray *img_dir = gradient_direction_to_image(gr);
        snprintf(path_out, sizeof(path_out),
                 "%s/%s_%s_direction.pgm", out_dir, base, op_lower);
        if (img_dir) {
            pgm_write(path_out, img_dir);
            printf("  Sauvegarde : %s\n", path_out);
            image_gray_free(img_dir);
        }

        /* Gx */
        ImageGray *img_gx = gradient_gx_to_image(gr);
        snprintf(path_out, sizeof(path_out),
                 "%s/%s_%s_gx.pgm", out_dir, base, op_lower);
        if (img_gx) {
            pgm_write(path_out, img_gx);
            printf("  Sauvegarde : %s\n", path_out);
            image_gray_free(img_gx);
        }

        /* Gy */
        ImageGray *img_gy = gradient_gy_to_image(gr);
        snprintf(path_out, sizeof(path_out),
                 "%s/%s_%s_gy.pgm", out_dir, base, op_lower);
        if (img_gy) {
            pgm_write(path_out, img_gy);
            printf("  Sauvegarde : %s\n", path_out);
            image_gray_free(img_gy);
        }

        gradient_free(gr);
    }

    image_gray_free(gray);
}

/* Point d'entrée */
int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Usage : %s <dossier_images> <dossier_sortie>\n", argv[0]);
        return 1;
    }

    const char *img_dir = argv[1];
    const char *out_dir = argv[2];

    mkdir(out_dir, 0755);

    printf("============================================================\n");
    printf("  CALCUL DU GRADIENT D'IMAGES - Sobel / Prewitt / Roberts\n");
    printf("  Source  : %s\n", img_dir);
    printf("  Sortie  : %s\n", out_dir);
    printf("============================================================\n");

    DIR *dp = opendir(img_dir);
    if (!dp) {
        fprintf(stderr, "ERREUR: Impossible d'ouvrir '%s'\n", img_dir);
        return 1;
    }

    struct dirent *entry;
    int count = 0;

    while ((entry = readdir(dp)) != NULL) {
        const char *ext = file_extension(entry->d_name);

        int is_pgm = (strcasecmp(ext, "pgm") == 0);
        int is_ppm = (strcasecmp(ext, "ppm") == 0);

        if (!is_pgm && !is_ppm) continue;

        char full_path[MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s/%s", img_dir, entry->d_name);

        char bname[MAX_PATH];
        base_name(entry->d_name, bname, sizeof(bname));

        process_image(full_path, bname, out_dir, is_ppm);
        count++;
    }

    closedir(dp);

    printf("\n============================================================\n");
    printf("  Traitement termine. %d image(s) traitee(s).\n", count);
    printf("  Resultats dans : %s\n", out_dir);
    printf("============================================================\n");

    return 0;
}
