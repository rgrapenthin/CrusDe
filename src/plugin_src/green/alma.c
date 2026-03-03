/***************************************************************************
 * File:        ./plugins/green/alma.c
 * Author:      Ronni Grapenthin, UAF-GI
 * Created:     2009-07-08
 * Updated:     2026-03-03  — ported to ALMA3 (Melini et al. 2022)
 * Licence:     GPL
 ****************************************************************************/

/**
 *  @ingroup GreensFunction
 *  @ingroup Plugin
 **/

/*@{*/
/** \file alma.c
 *
 * Interface to ALMA3 (the plAnetary Love nuMbers cAlculator, Melini et al. 2022).
 * https://github.com/danielemelini/ALMA3
 *
 * $ALMA must point to the directory that contains alma.exe.
 *
 * ALMA3 is invoked with a temporary config file written to /tmp.
 * The 'file' parameter must point to an ALMA3 rheological model file
 * (absolute path, or path relative to $ALMA).
 *
 * get_value_at() implements the Farrell (1972) loading Green's function
 * using a Legendre polynomial series truncated at deg_max.
 * NOTE: for accurate near-field results with small loads (radius r), deg_max
 * should be of order pi*a/r (e.g. ~10000 for a 2 km disk on Earth).
 */
/*@}*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "crusde_api.h"

/* --- registered parameter pointers --- */
double *p_deg_min;
double *p_deg_max;
double *p_deg_step;
char   **p_file;        /* path to ALMA3 rheological model file */
double *p_ng;           /* order of Gaver/Salzer sequence       */
double *p_nla;          /* number of layers in model file       */
double *p_p;            /* number of time subdivisions (p+1 pts)*/
double *p_sd;           /* number of significant digits         */
double *p_time_min;     /* start of time range (kyrs, linear)   */
double *p_time_max;     /* end   of time range (kyrs, linear)   */

int x_pos, y_pos, z_pos;

float **love_number_h;
float **love_number_k;
float **love_number_l;

void read_alma_output(const char *file, float **love_array, int times, int degrees);

extern const char* get_name()     { return "alma"; }
extern const char* get_version()  { return "0.2"; }
extern const char* get_authors()  { return "ronni grapenthin"; }
extern PluginCategory get_category() { return GREEN_PLUGIN; }
extern const char* get_description() {
    return "Interface to ALMA3 (Melini et al. 2022) for computing viscoelastic "
           "Love numbers. Set $ALMA to the directory containing alma.exe. "
           "The 'file' parameter must point to an ALMA3 rheological model file.";
}

extern void register_parameter()
{
    p_deg_min  = crusde_register_param_double("deg_min",  get_category());
    p_deg_max  = crusde_register_param_double("deg_max",  get_category());
    p_deg_step = crusde_register_param_double("deg_step", get_category());
    p_file     = crusde_register_param_string("file",     get_category());
    p_ng       = crusde_register_param_double("ng",       get_category());
    p_nla      = crusde_register_param_double("nla",      get_category());
    p_p        = crusde_register_param_double("p",        get_category());
    p_sd       = crusde_register_param_double("sd",       get_category());
    p_time_min = crusde_register_param_double("time_min", get_category());
    p_time_max = crusde_register_param_double("time_max", get_category());
}

extern void register_output_fields()
{
    crusde_info("%s registers output fields...", get_name());

    crusde_register_output_field(&x_pos, X_FIELD);
    crusde_register_output_field(&y_pos, Y_FIELD);
    crusde_register_output_field(&z_pos, Z_FIELD);

    crusde_info("\t x: %d", x_pos);
    crusde_info("\t y: %d", y_pos);
    crusde_info("\t z: %d", z_pos);
}

extern void request_plugins() {}

extern void init()
{
    const char *alma_dir = getenv("ALMA");
    if (alma_dir == NULL) {
        crusde_error("Error: Environment variable ALMA is not defined!\n"
                     "Set it to the directory that contains alma.exe.\n");
        crusde_exit(-1);
    }

    int times   = (int)(*p_p) + 1;
    int degrees = (int)ceil((*p_deg_max - *p_deg_min + 1.0) / *p_deg_step);
    pid_t pid   = getpid();

    /* paths for temp files */
    char config_file[512], log_file[512];
    char h_file[512], l_file[512], k_file[512];
    sprintf(config_file, "/tmp/crusde_alma_%d.cfg", pid);
    sprintf(log_file,    "/tmp/crusde_alma_%d.log", pid);
    sprintf(h_file,      "/tmp/crusde_alma_h_%d.dat", pid);
    sprintf(l_file,      "/tmp/crusde_alma_l_%d.dat", pid);
    sprintf(k_file,      "/tmp/crusde_alma_k_%d.dat", pid);

    /* resolve model file: use as-is if absolute, otherwise prepend $ALMA */
    char model_file[512];
    if ((*p_file)[0] == '/') {
        strncpy(model_file, *p_file, sizeof(model_file) - 1);
        model_file[sizeof(model_file) - 1] = '\0';
    } else {
        snprintf(model_file, sizeof(model_file), "%s/%s", alma_dir, *p_file);
    }

    /* write ALMA3 config file */
    FILE *cfg = fopen(config_file, "w");
    if (cfg == NULL) {
        crusde_error("Could not create ALMA3 config file: %s", config_file);
        crusde_exit(-1);
    }

    fprintf(cfg, "%d           ! number of significant digits\n", (int)*p_sd);
    fprintf(cfg, "%d           ! order of Gaver sequence\n",      (int)*p_ng);
    fprintf(cfg, "Loading      ! LN type\n");
    fprintf(cfg, "%d           ! minimum degree\n",  (int)*p_deg_min);
    fprintf(cfg, "%d           ! maximum degree\n",  (int)*p_deg_max);
    fprintf(cfg, "%d           ! degree step\n",     (int)*p_deg_step);
    fprintf(cfg, "lin          ! time scale (linear)\n");
    fprintf(cfg, "%d           ! number of time subdivisions\n", (int)*p_p);
    fprintf(cfg, "%d %d        ! time range (kyrs)\n", (int)*p_time_min, (int)*p_time_max);
    fprintf(cfg, "step         ! load time history\n");
    fprintf(cfg, "1.0          ! ramp length (unused for step)\n");
    fprintf(cfg, "%d           ! number of layers\n", (int)*p_nla);
    fprintf(cfg, "%s\n",       model_file);
    fprintf(cfg, "%s\n",       log_file);
    fprintf(cfg, "Real         ! Real LNs\n");
    fprintf(cfg, "ln_vs_n      ! output format: LNs vs harmonic degree\n");
    fprintf(cfg, "%s\n", h_file);
    fprintf(cfg, "%s\n", l_file);
    fprintf(cfg, "%s\n", k_file);
    fclose(cfg);

    /* call ALMA3 */
    char alma_call[1024];
    snprintf(alma_call, sizeof(alma_call), "%s/alma.exe %s", alma_dir, config_file);
    crusde_info("Calling ALMA3: %s", alma_call);
    int ret = system(alma_call);
    if (ret != 0) {
        crusde_error("ALMA3 returned non-zero exit code: %d", ret);
        crusde_exit(-1);
    }

    /* allocate love number arrays: [time][degree] */
    love_number_h = (float**) malloc(sizeof(float*) * times);
    love_number_k = (float**) malloc(sizeof(float*) * times);
    love_number_l = (float**) malloc(sizeof(float*) * times);
    if (!love_number_h || !love_number_k || !love_number_l) { crusde_bad_alloc(); }

    int t = -1;
    while (++t < times) {
        love_number_h[t] = (float*) calloc(degrees, sizeof(float));
        love_number_k[t] = (float*) calloc(degrees, sizeof(float));
        love_number_l[t] = (float*) calloc(degrees, sizeof(float));
        if (!love_number_h[t] || !love_number_k[t] || !love_number_l[t]) { crusde_bad_alloc(); }
    }

    read_alma_output(h_file, love_number_h, times, degrees);
    read_alma_output(l_file, love_number_l, times, degrees);
    read_alma_output(k_file, love_number_k, times, degrees);

    /* clean up temp files */
    remove(config_file);
    remove(log_file);
    remove(h_file);
    remove(l_file);
    remove(k_file);
}

extern void run() {}

extern void clear()
{
    if (!love_number_h) return;

    int times = (int)(*p_p) + 1;
    int t = -1;
    while (++t < times) {
        free(love_number_h[t]);
        free(love_number_k[t]);
        free(love_number_l[t]);
    }
    free(love_number_h);  love_number_h = NULL;
    free(love_number_k);  love_number_k = NULL;
    free(love_number_l);  love_number_l = NULL;
}

/*! Returns the Green's function value at point (x, y).
 *
 *  Implements the Farrell (1972) loading Green's function for a spherical Earth:
 *
 *    G_z(r) = (a/M_E) * sum_n  h_n(t) * P_n(cos theta)
 *    G_h(r) = (a/M_E) * sum_n  l_n(t) * dP_n(cos theta)/d_theta
 *
 *  where theta = r/a is the angular distance from the load (at the origin)
 *  to the observation point, a = Earth radius, M_E = Earth mass, h_n and l_n
 *  are loading Love numbers computed by ALMA3.
 *
 *  The time index is taken from crusde_model_step(), clamped to [0, p].
 *
 *  Note: the series is truncated at deg_max.  For accurate near-field results
 *  deg_max should be several hundred or more.
 *
 *  References:
 *    Farrell (1972) Deformation of the Earth by surface loads.
 *                  Rev. Geophys. Space Phys. 10(3), 761-797.
 */

#define EARTH_RADIUS  6.371e6    /* m  */
#define EARTH_MASS    5.972e24   /* kg */

extern int get_value_at(double **result, int x, int y)
{
    (*result)[x_pos] = 0.0;
    (*result)[y_pos] = 0.0;
    (*result)[z_pos] = 0.0;

    /* Convert grid indices to metres */
    double x_m = x * crusde_get_gridsize();
    double y_m = y * crusde_get_gridsize();
    double r   = sqrt(x_m*x_m + y_m*y_m);

    if (r == 0.0) return NOERROR;   /* series diverges at origin */

    /* Angular distance (flat-Earth / small-angle: theta = r/a) */
    double theta     = r / EARTH_RADIUS;
    double cos_theta = cos(theta);
    double sin_theta = sin(theta);

    /* Time index: use current model step, clamped to available range */
    int t     = crusde_model_step();
    int t_max = (int)(*p_p);
    if (t > t_max) t = t_max;

    int deg_min  = (int)(*p_deg_min);
    int deg_max  = (int)(*p_deg_max);
    int deg_step = (int)(*p_deg_step);

    double sum_z = 0.0;   /* vertical   displacement sum */
    double sum_h = 0.0;   /* horizontal displacement sum (tangential direction) */

    /*
     * Legendre polynomial three-term recurrence:
     *   P_{n+1}(x) = ((2n+1)*x*P_n(x) - n*P_{n-1}(x)) / (n+1)
     *
     * At entry to loop iteration n: Pn_prev = P_{n-2}, Pn = P_{n-1}.
     * Inside iteration: compute P_n, then dP_n/dtheta via:
     *   dP_n(cos t)/dt = -n * (P_{n-1}(cos t) - cos_t * P_n(cos t)) / sin_t
     *
     * For sin_t ≈ 0 use Taylor:  dP_n/dt ≈ -n(n+1)/2 * sin_t
     */

    /* P_0 = 1, dP_0/dt = 0 */
    double P0 = 1.0;
    if (0 >= deg_min && 0 <= deg_max && (0 - deg_min) % deg_step == 0) {
        int idx = (0 - deg_min) / deg_step;
        sum_z += (double)love_number_h[t][idx] * P0;
        /* dP_0/dtheta = 0 */
    }

    /* P_1 = cos_theta, dP_1/dt = -sin_theta */
    double P1 = cos_theta;
    if (1 <= deg_max) {
        double dP1 = -sin_theta;
        if (1 >= deg_min && (1 - deg_min) % deg_step == 0) {
            int idx = (1 - deg_min) / deg_step;
            sum_z += (double)love_number_h[t][idx] * P1;
            sum_h += (double)love_number_l[t][idx] * dP1;
        }
    }

    /* n = 2 .. deg_max: compute P_n from P_{n-1} and P_{n-2} */
    double Pn_prev = P0;
    double Pn_curr = P1;
    int n;
    for (n = 2; n <= deg_max; n++) {
        double Pn = ((2.0*n - 1.0)*cos_theta*Pn_curr - (n - 1.0)*Pn_prev) / (double)n;

        double dPn;
        if (sin_theta > 1e-10) {
            /* dP_n/dt = -n * (P_{n-1} - cos_t * P_n) / sin_t */
            dPn = -(double)n * (Pn_curr - cos_theta * Pn) / sin_theta;
        } else {
            dPn = -(double)n * ((double)n + 1.0) / 2.0 * sin_theta;
        }

        if (n >= deg_min && (n - deg_min) % deg_step == 0) {
            int idx = (n - deg_min) / deg_step;
            sum_z += (double)love_number_h[t][idx] * Pn;
            sum_h += (double)love_number_l[t][idx] * dPn;
        }

        Pn_prev = Pn_curr;
        Pn_curr = Pn;
    }

    /*
     * Scale factor: a/M_E  (Farrell 1972, loading Green's function)
     * Negated because loading Love numbers h_n < 0 for vertical subsidence,
     * and the formula is G = -(a/M_E) * sum, giving positive z = downward.
     */
    double scale = EARTH_RADIUS / EARTH_MASS;

    (*result)[z_pos] = -scale * sum_z;
    (*result)[x_pos] = -scale * sum_h * (x_m / r);
    (*result)[y_pos] = -scale * sum_h * (y_m / r);

    return NOERROR;
}

/*
 * Read one ALMA3 output file in ln_vs_n format:
 *
 *   # comment lines...
 *   n   val_t0   val_t1   ...   val_tp
 *
 * Each row = one harmonic degree; each column (after degree) = one time.
 * We store: love_array[t][n_idx]
 */
void read_alma_output(const char *file, float **love_array, int times, int degrees)
{
    FILE *fi = fopen(file, "r");
    if (fi == NULL) {
        crusde_warning("FILE PROBLEM: <%s>", file);
        perror(file);
        crusde_exit(1);
    }
    if (ferror(fi)) {
        crusde_warning("FILE PROBLEM: <%s>", file);
        perror(file);
        fclose(fi);
        crusde_exit(1);
    }

    crusde_debug("Reading ALMA3 file <%s>, degrees=%d, times=%d", file, degrees, times);

    char line[4096];
    int n_idx = 0;

    while (fgets(line, sizeof(line), fi) && n_idx < degrees) {
        /* skip comment/blank lines */
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        if (strlen(line) < 2) continue;

        /* parse: degree  val_t0  val_t1  ...  val_tp */
        char *ptr = line;
        char *endptr;

        /* skip the degree column */
        strtol(ptr, &endptr, 10);
        if (endptr == ptr) continue;   /* not a data line */
        ptr = endptr;

        /* read time values */
        int t;
        for (t = 0; t < times; t++) {
            float val = (float) strtod(ptr, &endptr);
            if (endptr == ptr) break;
            love_array[t][n_idx] = val;
            crusde_debug("  degree_idx=%d t=%d val=%.6e", n_idx, t, val);
            ptr = endptr;
        }

        n_idx++;
    }

    if (n_idx < degrees) {
        crusde_warning("ALMA3 output <%s>: expected %d degrees, got %d", file, degrees, n_idx);
    }

    fclose(fi);
}
