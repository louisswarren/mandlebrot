#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>

#include <omp.h>
#include <assert.h>

#define die(...) do { \
		fprintf(stderr, __VA_ARGS__); \
		fprintf(stderr, "\n"); \
		exit(1); \
	} while(0)

struct workspace {
	int width, height;
	int n;
	void *z_arr;
	void *esc_n;
};

inline
static
double complex
m(complex z, complex c)
{
	return z * z + c;
}

int
escaped(double complex z)
{
	double a = creal(z), b = cimag(z);
	return a * a + b * b > 4;
}

int
workspace_init(struct workspace *w, int width, int height)
{
	complex double (*zarr)[height] = calloc(width, sizeof(*zarr));
	unsigned int (*inds)[height] = calloc(width, sizeof(*inds));
	w->width = width;
	w->height = height;
	w->n = 0;
	w->z_arr = zarr;
	w->esc_n = inds;
	return !w->z_arr || !w->esc_n;
}

void
workspace_reset(struct workspace *w)
{
	w->n = 0;
	memset(w->z_arr, 0, w->width * w->height * sizeof(w->z_arr));
	memset(w->esc_n, 0, w->width * w->height * sizeof(w->esc_n));
}

/* I probably won't need this */
void
workspace_deinit(struct workspace *w)
{
	w->width = 0;
	w->height = 0;
	w->n = 0;
	free(w->z_arr);
	free(w->esc_n);
}

void
output(const struct workspace *w)
{
	unsigned int (*inds)[w->height] = w->esc_n;

	printf("P2\n%d %d\n%d\n", w->width, w->height, w->n);
	for (int j = 0; j < w->height; ++j) {
		for (int i = 0; i < w->width; ++i) {
			printf("%d ", inds[i][j]);
		}
		putchar('\n');
	}

}

void
render_once(
	struct workspace *w,
	double xmin, double xmax, double ymin, double ymax
) {
	complex double (*zarr)[w->height] = w->z_arr;
	unsigned int (*inds)[w->height] = w->esc_n;

	for (int j = 0; j < w->height; ++j) {
		#pragma omp parallel for
		for (int i = 0; i < w->width; ++i) {
			double a = xmin + (xmax - xmin) * i / w->width;
			double b = ymax - (ymax - ymin) * j / w->height;
			zarr[i][j] = m(zarr[i][j], a + I * b);
			inds[i][j] |= !inds[i][j] * escaped(zarr[i][j]) * w->n;
		}
	}
	w->n++;
}

void
render(
	struct workspace *w,
	int steps,
	double xmin, double xmax, double ymin, double ymax
) {
	for (int n = 0; n < steps; ++n) {
		render_once(w, xmin, xmax, ymin, ymax);
	}
}

int
main(int argc, char *argv[])
{
	double xmin = -2.0;
	double xmax = 0.5;
	double ymin = -1.25;
	double ymax = 1.25;

	int height = 2160;
	int width = height * (xmax - xmin) / (ymax - ymin) + 0.5;
	assert(width == height);

	struct workspace w;
	if (workspace_init(&w, width, height))
		die("Failed to allocate %dx%d workspace", width, height);

	if (argc == 1) {
		render(&w, 200, xmin, xmax, ymin, ymax);
		output(&w);
	}
	else if (argc > 1 && argv[1][0] == 'z') {
		// Zoom towards (-1.40065, 0)
		double target = -1.40065;
		while (1) {
			double ratio = (target - xmin) / (xmax - xmin);
			xmin += 0.01 * ratio;
			xmax -= 0.01 * (1 - ratio);
			ymin += 0.005;
			ymax -= 0.005;
			render(&w, 20, xmin, xmax, ymin, ymax);
			workspace_reset(&w);
			output(&w);
		}
	}
}
