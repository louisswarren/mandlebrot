#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <complex.h>

#include <omp.h>
#include <assert.h>

#define IL ((complex double) I)

#define die(...) do { \
		fprintf(stderr, __VA_ARGS__); \
		fprintf(stderr, "\n"); \
		exit(1); \
	} while(0)

struct workcell {
	complex double z;
	int e;
};

struct workspace {
	int width, height;
	int n;
	struct workcell c[];
};


size_t
workspace_size(int width, int height)
{
	size_t sz = sizeof(struct workcell);
	if (SIZE_MAX / (unsigned)width < sz)
		return 0;
	sz *= (unsigned)width;
	if (SIZE_MAX / (unsigned)height < sz)
		return 0;
	sz *= (unsigned)height;
	if (SIZE_MAX - sizeof(struct workspace) < sz)
		return 0;
	sz += sizeof(struct workspace);
	return sz;
}

void
workspace_reset(struct workspace *w)
{
	w->n = 0;
	memset(w->c, 0, sizeof(*w->c) * (unsigned)w->width * (unsigned)w->height);
}

void
workspace_init(struct workspace *w, int width, int height)
{
	w->width = width;
	w->height = height;
	workspace_reset(w);
}

long
colour(unsigned int s)
{
	const long start_and_shift[][2] = {
		{0xff0000,  0x000100},
		{0xffff00, -0x010000},
		{0x00ff00,  0x000001},
		{0x00ffff, -0x000100},
		{0x0000ff,  0x010000},
		{0xff00ff, -0x000001}
	};
	int t = s % 1530;
	int section = t / 255;
	int mul = t % 255;
	return start_and_shift[section][0] + mul * start_and_shift[section][1];
}

void
output(struct workspace *w)
{
	struct workcell (*c)[w->height] = (void *)w->c;

	printf("P6\n%d %d 255\n", w->width, w->height);
	for (int j = 0; j < w->height; ++j) {
		for (int i = 0; i < w->width; ++i) {
			long col = 0;
			if (c[i][j].e > 0)
				col = colour(1020 - (unsigned int)(c[i][j].e * 1020 / w->n));
			unsigned char x;
			x = (col >> 16) & 0xff; fwrite(&x, 1, 1, stdout);
			x = (col >>  8) & 0xff; fwrite(&x, 1, 1, stdout);
			x = (col >>  0) & 0xff; fwrite(&x, 1, 1, stdout);
		}
	}

}

inline
double complex
m(complex z, complex c)
{
	return z * z + c;
}

inline
int
escaped(double complex z)
{
	double a = creal(z), b = cimag(z);
	return a * a + b * b > 4;
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
			if (!c[i][j].e) {
				double a = xmin + (xmax - xmin) * i / w->width;
				double b = ymax - (ymax - ymin) * j / w->height;
				c[i][j].z = m(c[i][j].z, a + IL * b);
				c[i][j].e = escaped(c[i][j].z) * w->n;
			}
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
	int width = (int)(height * (xmax - xmin) / (ymax - ymin) + 0.5);
	assert(width == height);

	size_t w_sz = workspace_size(width, height);
	struct workspace *w;

	if (!w_sz)
		die("Bad workspace size for %dx%d image", width, height);
	w = malloc(w_sz);
	if (!w)
		die("Failed to allocate %dx%d image workspace", width, height);
	workspace_init(w, width, height);

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
