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

struct workcell {
	complex double z;
	unsigned int e;
};

struct workspace {
	int width, height;
	int n;
	struct workcell c[];
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

struct workspace *
workspace_create(int width, int height)
{
	struct workspace *w = malloc(sizeof(*w) + sizeof(w->c[0]) * width * height);
	if (w) {
		w->width = width;
		w->height = height;
	}
	return w;
}

void
workspace_reset(struct workspace *w)
{
	w->n = 0;
	memset(w->c, 0, w->width * sizeof(struct workcell (*)[w->height]));
}


void
output(const struct workspace *w)
{
	struct workcell (*c)[w->height] = (void *)w->c;

	printf("P2\n%d %d\n%d\n", w->width, w->height, w->n);
	for (int j = 0; j < w->height; ++j) {
		for (int i = 0; i < w->width; ++i) {
			printf("%d ", c[i][j].e);
		}
		putchar('\n');
	}

}

void
render_once(
	struct workspace *w,
	double xmin, double xmax, double ymin, double ymax
) {
	struct workcell (*c)[w->height] = (void *)w->c;

	for (int j = 0; j < w->height; ++j) {
		#pragma omp parallel for
		for (int i = 0; i < w->width; ++i) {
			double a = xmin + (xmax - xmin) * i / w->width;
			double b = ymax - (ymax - ymin) * j / w->height;
			c[i][j].z = m(c[i][j].z, a + I * b);
			c[i][j].e |= !c[i][j].e * escaped(c[i][j].z) * w->n;
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

	struct workspace *w = workspace_create(width, height);
	if (!w)
		die("Failed to allocate %dx%d workspace", width, height);

	if (argc == 1) {
		render(w, 200, xmin, xmax, ymin, ymax);
		output(w);
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
			workspace_reset(w);
			render(w, 20, xmin, xmax, ymin, ymax);
			output(w);
		}
	}
}
