#include <stdio.h>
#include <stdlib.h>
#include <complex.h>

#include <omp.h>

#define die(...) do { \
		fprintf(stderr, __VA_ARGS__); \
		fprintf(stderr, "\n"); \
		exit(1); \
	} while(0)

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

void
render(
	int height, int steps,
	double xmin, double xmax, double ymin, double ymax
) {
	int width = height * (xmax - xmin) / (ymax - ymin) + 0.5;

	complex double (*zarr)[height] = calloc(width, sizeof(*zarr));
	int (*inds)[height] = calloc(width, sizeof(*inds));

	if (!zarr || !inds)
		die("calloc failed");

	printf("P2\n%d %d\n%d\n", width, height, steps);

	for (int n = 0; n < steps; ++n) {
		for (int j = 0; j < height; ++j) {
			#pragma omp parallel for
			for (int i = 0; i < width; ++i) {
				double a = xmin + (xmax - xmin) * i / width;
				double b = ymax - (ymax - ymin) * j / height;
				zarr[i][j] = m(zarr[i][j], a + I * b);
				inds[i][j] |= !inds[i][j] * escaped(zarr[i][j]) * n;
			}
		}
	}

	for (int j = 0; j < height; ++j) {
		for (int i = 0; i < width; ++i) {
			printf("%d ", inds[i][j]);
		}
		putchar('\n');
	}

	free(zarr);
	free(inds);
}

int
main(int argc, char *argv[])
{
	double xmin = -2.0;
	double xmax = 0.5;
	double ymin = -1.25;
	double ymax = 1.25;

	if (argc == 1) {
		render(1080, 100, xmin, xmax, ymin, ymax);
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
			render(1080, 20, xmin, xmax, ymin, ymax);
		}
	}
}
