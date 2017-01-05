/*
Original source from hdf2hdf256.c 
(C)2012 Miguel Angel Rodriguez Jodar (McLeod/IdeaFix). GPL licensed

The simple 8-bit IDE interface stores in the LSB, and the MSB contains zeros:
00 12 00 AB 00 34 00 CD

But HDF file format can discard the MSB to recover space:
12 AB 34 CD

This tool can convert between the above formats and can remove the HDF header
 - Images can be reverted back to the native 8bit format using '-8'
 - The HDF header can be discarded using '-r'
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void convert_sectors(FILE *fi, FILE *fo, int src_size, int dst_size);
void usage(void);

int main (int argc, char *argv[])
{
        FILE *fi, *fo;
        int c, result;
	unsigned char hdr[128];
	int header = 1;
        int lsbonly = 1; // 0 = 16bit>8bit, 1 = 8bit>16bit

	while ( (c = getopt(argc, argv, "8r")) != -1) {
		switch (c) {
		case 'r':
			header = 0;
			break;
		case '8':
			lsbonly = 0;
			break;
	    }
	}

	if ( 2 != (argc - optind) ) {
		usage();
        	return EXIT_FAILURE;
	};

        fi = fopen(argv[optind],"rb");
        if (!fi) {
                fprintf(stderr, "Unable to create file '%s'\n", argv[optind]);
                return EXIT_FAILURE;
        }

        fo = fopen(argv[optind+1],"wb");
        if (!fo) {
                fprintf(stderr, "Unable to create file '%s'\n", argv[optind+1]);
                return EXIT_FAILURE;
        }

	if (header) {
        	if (fread(hdr, 1, 128, fi) < 128) {
                	fprintf(stderr, "ERROR. Premature end of file for input file.\n");
                	return EXIT_FAILURE;
		}
        	hdr[8] = lsbonly; // HDF flag to indicate only LSB data is expected
        	fwrite(hdr, 1, 128, fo);
	}

        if (lsbonly)
                convert_sectors(fi, fo, 512, 256);
	else
                convert_sectors(fi, fo, 256, 512);

        fclose(fi);
        fclose(fo);
        return EXIT_SUCCESS;
}

void convert_sectors(FILE *fi, FILE *fo, int src_size, int dst_size) {
        unsigned char src[src_size];
        unsigned char dst[dst_size];
        int i;

	// Read every sector
        while (fread(src, 1, src_size, fi) == src_size) {
                for (i=0; i<256; i++) {
                        if (src_size == 512)
                                dst[i] = src[i*2]; // discard MSB data, store just LSB
                        else {
                                dst[i*2] = src[i]; // LSB = data
                                dst[i*2+1] = 0;    // MSB = 0x00
                        }
                }
                fwrite(dst, 1, dst_size, fo);
        }
}

void usage (void) {
	fprintf (stderr, "Usage:\n");
	fprintf (stderr, " hdfconv [options] <inputfile> <outputfile>\n");
	fprintf (stderr, "\n");
	fprintf (stderr, "Options:\n");
	fprintf (stderr, " -8   Convert to 8-bit instead of 16bit\n");
	fprintf (stderr, " -r   Do not add HDF header\n");
}
