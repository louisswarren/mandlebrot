#include <stdio.h>
#include <stdlib.h>
#include <complex.h>

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
	long long int i, j;
	double a, b;
	double complex c;

	int width = height * (xmax - xmin) / (ymax - ymin);

	complex double (*zarr)[height] = calloc(width, sizeof(*zarr));
	int (*inds)[height] = calloc(width, sizeof(*inds));

	if (!zarr || !inds)
		die("calloc failed");

	printf("P2\n%d %d\n%d\n", width, height, steps);

	for (int n = 0; n < steps; ++n) {
		for (j = 0; j < height; ++j) for (i = 0; i < width; ++i) {
			a = xmin + (xmax - xmin) * i / width;
			b = ymin + (ymax - ymin) * j / height;
			zarr[i][j] = m(zarr[i][j], a + I * b);
			inds[i][j] |= !inds[i][j] * escaped(zarr[i][j]) * n;
		}
	}

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; ++i) {
			printf("%d ", inds[i][j]);
		}
		putchar('\n');
	}

	free(zarr);
	free(inds);
}

int
main(void)
{
	render(1080, 100, -2.0, 0.5, -1.2, 1.2);
}
