#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include <unistd.h>

void usage(char **argv) {
	printf("Usage: %s [-t] [-a] image.png mask.png\n", argv[0]);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
	int transparent = 0;
	int add = -1;

	extern int optind;
	extern char *optarg;
	int i;

	while ((i = getopt(argc, argv, "ta")) != -1) {
		switch (i) {
		case 't':
			transparent = 1;
			break;

		case 'a':
			add = 1;
			break;

		default:
			usage(argv);
		}
	}

	if (argc - optind != 2) {
		usage(argv);
	}
	
	png_image image1, image2;

	memset(&image1, 0, sizeof image1);
	image1.version = PNG_IMAGE_VERSION;
	image2.version = PNG_IMAGE_VERSION;

	unsigned char *buffer1;
	unsigned char *buffer2;

	if (png_image_begin_read_from_file(&image1, argv[optind])) {
		image1.format = PNG_FORMAT_RGBA;
		buffer1 = malloc(PNG_IMAGE_SIZE(image1));

		if (!png_image_finish_read(&image1, NULL, buffer1, 0, NULL)) {
			fprintf(stderr, "read failure in %s\n", argv[optind]);
			exit(EXIT_FAILURE);
		}
	} else {
		fprintf(stderr, "read failure in %s\n", argv[optind]);
		exit(EXIT_FAILURE);
	}

	if (png_image_begin_read_from_file(&image2, argv[optind + 1])) {
		image2.format = PNG_FORMAT_RGBA;
		buffer2 = malloc(PNG_IMAGE_SIZE(image2));

		if (!png_image_finish_read(&image2, NULL, buffer2, 0, NULL)) {
			fprintf(stderr, "read failure in %s\n", argv[optind + 1]);
			exit(EXIT_FAILURE);
		}
	} else {
		fprintf(stderr, "read failure in %s\n", argv[optind + 1]);
		exit(EXIT_FAILURE);
	}

	if (image1.width != image2.width || image1.height != image2.height) {
		fprintf(stderr, "images not the same size\n");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < image1.width * image1.height; i++) {
		double a = buffer1[4 * i + 3] / 255.0;

		int r = buffer1[4 * i + 0] * a;
		int g = buffer1[4 * i + 1] * a;
		int b = buffer1[4 * i + 2] * a;

		double a2 = buffer2[4 * i + 3] / 255.0;

		int r2 = buffer2[4 * i + 0] * a2;
		int g2 = buffer2[4 * i + 1] * a2;
		int b2 = buffer2[4 * i + 2] * a2;

		r += r2 * add;
		g += g2 * add;
		b += b2 * add;

		if (r < 0) {
			r = 0;
		}
		if (g < 0) {
			g = 0;
		}
		if (b < 0) {
			b = 0;
		}

		if (r > 255) {
			r = 255;
		}
		if (g > 255) {
			g = 255;
		}
		if (b > 255) {
			b = 255;
		}

		if (transparent) {
			int max = r;
			if (g > max) {
				max = g;
			}
			if (b > max) {
				max = b;
			}

			buffer1[4 * i + 0] = (double) r / max * 255;
			buffer1[4 * i + 1] = (double) g / max * 255;
			buffer1[4 * i + 2] = (double) b / max * 255;
			buffer1[4 * i + 3] = max;
		} else {
			buffer1[4 * i + 0] = r;
			buffer1[4 * i + 1] = g;
			buffer1[4 * i + 2] = b;
			buffer1[4 * i + 3] = 255;
		}
	}

        png_image_write_to_stdio(&image1, stdout, 0, buffer1, 4 * image1.width, NULL);
        png_image_free(&image1);

	return 0;
}
