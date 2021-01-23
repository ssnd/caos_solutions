
#include <assert.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#define MAGIC_VALUE 0X4D42
#define BPP 32

typedef struct {
    uint16_t magic_val;         // Magic identifier
    uint32_t size;              // File size in bytes
    uint16_t reserved1;         // Not used
    uint16_t reserved2;         // Not used
    uint32_t offset;            //
    uint32_t header_size;       // Header size in bytes
    uint32_t width;             // Width of the image
    uint32_t height;            // Height of image
    uint16_t planes;            // Number of color planes
    uint16_t bits;              // Bits per pixel
    uint32_t compression;       // Compression type
    uint32_t image_size;        // Image size in bytes
    uint32_t x_resolution;      // Pixels per meter
    uint32_t y_resolution;      // Pixels per meter
    uint32_t n_colors;          // Number of colors
    uint32_t important_colours; // Important colors
    uint32_t red_bitmask;       // Important colors
    uint32_t green_bitmask;     // Important colors
    uint32_t blue_bitmask;      // Important colors
    uint32_t alpha_bitmask;     // Important colors
} __attribute__((packed)) BMPHeader;

typedef struct {
    BMPHeader header;
    uint32_t size;
    uint16_t height;
    uint16_t width;
    uint16_t bpp;
    uint32_t* data;
} __attribute__((packed)) BMPImg;

BMPImg* readBMP(char*);
void writeBMP(BMPImg*, char*);

void writeBMP(BMPImg* img, char* name)
{
    FILE* file_ptr = NULL;
    file_ptr = fopen(name, "w");
    if (file_ptr == NULL)
        return;

    fwrite(&(img->header), sizeof(BMPHeader), 1, file_ptr);
    fwrite(img->data, sizeof(char), img->size, file_ptr);
    fclose(file_ptr);
}

BMPImg* readBMP(char* filename)
{
    FILE* file_ptr = fopen(filename, "r");
    BMPImg* img = malloc(sizeof(BMPImg));
    fread(&(img->header), sizeof(BMPHeader), 1, file_ptr);

    img->width = img->header.width;
    img->height = img->header.height;

    img->size = (img->header).size - sizeof(BMPHeader);
    img->bpp = img->header.bits;

    img->data = malloc(sizeof(uint32_t) * (img->size));
    fread(img->data, sizeof(uint32_t), img->height * img->width, file_ptr);
    uint32_t b;
    assert(fread(&b, sizeof(uint32_t), 1, file_ptr) == 0);

    fclose(file_ptr);
    return img;
}

uint32_t ComputeShiftCount(uint32_t v)
{
    uint32_t c = 32;
    v &= -v;
    if (v)
        c--;
    if (v & 0x0000FFFF)
        c -= 16;
    if (v & 0x00FF00FF)
        c -= 8;
    if (v & 0x0F0F0F0F)
        c -= 4;
    if (v & 0x33333333)
        c -= 2;
    if (v & 0x55555555)
        c -= 1;

    return c;
}

__m128i GetColor(uint32_t* i, int8_t shift, uint32_t bitmask)
{
    __m128i d_red, mm_bitmask;
    uint32_t bitmask_arr[4] = {bitmask, bitmask, bitmask, bitmask};

    d_red = _mm_loadu_si128((const __m128i_u*)i);
    mm_bitmask = _mm_loadu_si128((const __m128i_u*)&bitmask_arr[0]);
    d_red = _mm_and_si128(d_red, mm_bitmask);
    d_red = _mm_srli_epi32(d_red, shift);
    return d_red;
}

__m128 kOne;
__m128i CalculateResultingColor(
    __m128i s_red,
    __m128i d_red,
    __m128 s_alpha,
    __m128 d_alpha)
{

    kOne = _mm_set_ps1((float)1);
    __m128 buff, r_red;
    __m128i r_red_i;
    __m128 s_red_f, d_red_f;
    s_red_f = _mm_cvtepi32_ps(s_red);
    d_red_f = _mm_cvtepi32_ps(d_red);
    r_red = _mm_mul_ps(d_red_f, d_alpha);
    r_red = _mm_mul_ps(r_red, _mm_sub_ps(kOne, s_alpha));
    buff = _mm_mul_ps(s_red_f, s_alpha);
    r_red = _mm_add_ps(r_red, buff);
    r_red_i = _mm_cvtps_epi32(r_red);

    return r_red_i;
}

int main(int argc, char* argv[])
{
    BMPImg* destination = readBMP(argv[1]);

    BMPImg* source = readBMP(argv[2]);

    uint32_t red_shift, green_shift, blue_shift, alpha_shift;
    uint32_t red_bitmask, green_bitmask, blue_bitmask, alpha_bitmask;
    red_shift = ComputeShiftCount(destination->header.red_bitmask);
    green_shift = ComputeShiftCount(destination->header.green_bitmask);
    blue_shift = ComputeShiftCount(destination->header.blue_bitmask);
    alpha_shift = ComputeShiftCount(destination->header.alpha_bitmask);

    red_bitmask = destination->header.red_bitmask;
    green_bitmask = destination->header.green_bitmask;
    blue_bitmask = destination->header.blue_bitmask;
    alpha_bitmask = destination->header.alpha_bitmask;

    __m128i d_red, d_green, d_blue;
    __m128i s_red, s_green, s_blue;
    __m128i r_red, r_green, r_blue;
    __m128i d_alpha_int, s_alpha_int;

    __m128 d_alpha, s_alpha;

    for (size_t i = 0; i < destination->height * destination->width; i += 4) {

        d_red = GetColor(&destination->data[i], red_shift, red_bitmask);
        d_green = GetColor(&destination->data[i], green_shift, green_bitmask);
        d_blue = GetColor(&destination->data[i], blue_shift, blue_bitmask);
        d_alpha_int =
            GetColor(&destination->data[i], alpha_shift, alpha_bitmask);

        d_alpha = _mm_cvtepi32_ps(d_alpha_int);

        __m128 kFF = _mm_set_ps1((float)0xff);

        d_alpha = _mm_div_ps(d_alpha, kFF);

        s_red = GetColor(&source->data[i], red_shift, red_bitmask);
        s_green = GetColor(&source->data[i], green_shift, green_bitmask);
        s_blue = GetColor(&source->data[i], blue_shift, blue_bitmask);
        s_alpha_int = GetColor(&source->data[i], alpha_shift, alpha_bitmask);
        s_alpha = _mm_cvtepi32_ps(s_alpha_int);

        s_alpha = _mm_div_ps(s_alpha, kFF);

        r_red = CalculateResultingColor(s_red, d_red, s_alpha, d_alpha);
        r_green = CalculateResultingColor(s_green, d_green, s_alpha, d_alpha);
        r_blue = CalculateResultingColor(s_blue, d_blue, s_alpha, d_alpha);

        __m128i res;
        __m128i red_shifted, green_shifted, blue_shifted;

        red_shifted = _mm_slli_epi32(r_red, red_shift);
        green_shifted = _mm_slli_epi32(r_green, green_shift);
        blue_shifted = _mm_slli_epi32(r_blue, blue_shift);

        res = _mm_set_epi32(
            alpha_bitmask, alpha_bitmask, alpha_bitmask, alpha_bitmask);
        res = _mm_add_epi32(res, red_shifted);
        res = _mm_add_epi32(res, green_shifted);
        res = _mm_add_epi32(res, blue_shifted);

        _mm_store_si128((__m128i*)&destination->data[i], res);
    }

    writeBMP(destination, argv[3]);
    free(destination->data);
    free(destination);
    free(source->data);
    free(source);

    return 0;
}
