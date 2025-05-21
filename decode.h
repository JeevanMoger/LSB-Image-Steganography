#ifndef DECODE_H
#define DECODE_H

#include "types.h"

typedef struct _DecodeInfo
{
    char *stego_image_fname;
    FILE *fptr_stego_image;

    char output_fname[20];
    int magic_string_size;
    char magic_string[20];
    int secret_file_extn_size;
    char secret_file_extn[10];
    int secret_file_size;
    FILE *fptr_output_file;
}DecodeInfo;

Status read_and_validate_decode_args(char *argv[] , DecodeInfo *decInfo);

Status open_files_decode(DecodeInfo *encInfo);

Status do_decoding(DecodeInfo *decInfo);

Status decode_size(FILE *fptr_stego_image);

Status decode_magic_string(int size , DecodeInfo *decInfo);

Status decode_lsb_from_byte(char *data);

Status decode_data_from_image(int size, FILE *fptr_stego_image, FILE *fptr_output_file);

Status check_magic_string(char *data);\

Status decode_secret_file_extension(int size, DecodeInfo *decInfo);

Status Create_output_file(char *extn, DecodeInfo *decInfo);

Status decode_secret_data(int size , DecodeInfo *decInfo);

Status close_files_decode(DecodeInfo *decInfo);

#endif