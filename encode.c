#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "common.h"
#include <unistd.h>

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("INFO : width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("INFO : height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, RED"ERROR: Unable to open file %s\n"RESET, encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr,RED "ERROR: Unable to open file %s\n"RESET, encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr,RED "ERROR: Unable to open file %s\n"RESET, encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}
// Check thr opertion type
OperationType check_operation_type(char *ptr[])
{
    if(strcmp(ptr[1],"-e") == 0)
        return e_encode;
    else if(strcmp(ptr[1], "-d") == 0)
        return e_decode;
    else
        return e_unsupported;
}
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if (argv[2] != NULL && strcmp(strchr(argv[2],'.'),".bmp") == 0)         // check whether the bmp file is given or not 
    {
        encInfo -> src_image_fname = argv[2];
    }
    else
    {
        printf(RED"Encoding: ./a.out -e <.bmp_file> <.text_file> [output file] \n"RESET);
        return e_failure;
    }
    if(argv[3] != NULL && strstr(argv[3],".") != NULL)   // check whether the secret file is given or not
    {
        encInfo -> secret_fname = argv[3];
    }
    else
    {
        printf(RED"Encoding: ./a.out -e <.bmp_file> <.text_file> [output file] \n"RESET);
        return e_failure;
    }
    if(argv[4] != NULL)                                    // check if the output file is given or not and also check if it is a bmp file or not 
    {
        if(strstr(argv[4], ".") != NULL && strcmp(strstr(argv[4], "."), ".bmp") == 0)
        {
            encInfo -> stego_image_fname = argv[4];         // if given store it.
        }
        else
        {
            printf(RED"ERROR : Please Enter the .bmp file \n"RESET);
            return e_failure;
        }
    }
    else
    {
        encInfo->stego_image_fname = "stego_img.bmp";                             // If not given create your own file
	    printf(GREEN"INFO : Encoded file Created as %s\n"RESET, encInfo->stego_image_fname);
    }
    return e_success;
}
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo -> image_capacity = get_image_size_for_bmp(encInfo -> fptr_src_image);
    encInfo -> size_secret_file = get_file_size(encInfo -> fptr_secret);
    char *extension_ptr = strstr(encInfo -> secret_fname , ".");
    //printf("%s\n",encInfo -> secret_fname);
    for(int i = 0; i <= 4;i++)
    {
        encInfo -> extn_secret_file[i] = *extension_ptr++;
    }
    //printf("%s\n",encInfo -> extn_secret_file);
    int exten_len = strlen(encInfo -> extn_secret_file);
    int required_size = (54 + 32 + 32 + 32 + (exten_len * 8) + (strlen(MAGIC_STRING) * 8) + ((encInfo->size_secret_file) * 8));
    if(encInfo -> image_capacity > required_size)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}
uint get_file_size(FILE *fptr)
{
    int file_size;
    fseek(fptr,0,SEEK_END);
    file_size = ftell(fptr);
    rewind(fptr);
    return file_size;
}
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char buff[54];
    fseek(fptr_src_image,0,SEEK_SET);
    fread(buff,54,1,fptr_src_image);
    fwrite(buff,54,1,fptr_dest_image);
    fseek(fptr_dest_image,0,SEEK_END);
    if(ftell(fptr_dest_image) == 54)
        return e_success;
    else
        return e_failure;
}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    if(encode_data_to_image(magic_string,strlen(magic_string),encInfo -> fptr_src_image ,encInfo -> fptr_stego_image) == e_success)
        return e_success;
    else
        return e_failure;
}

Status do_encoding(EncodeInfo *encInfo)
{
    printf("INFO : Opening required files\n");
    if(open_files(encInfo) == e_success)       // opening the required files
    {
        printf(GREEN"INFO : DONE\n"RESET);
        printf("INFO : Checking Image Capacity\n");
        if(check_capacity(encInfo) == e_success)
        {
            printf(GREEN"INFO : DONE\n"RESET);
            printf(YELLOW"INFO : ========Encoding Procedure Started========\n"RESET);
            printf("INFO : Copying Header from thr BMP File\n");
            show_loading();
            if(copy_bmp_header(encInfo -> fptr_src_image, encInfo -> fptr_stego_image) == e_success)
            {
                printf(GREEN"INFO : DONE\n"RESET);
                printf("INFO : Encoding Magic string size\n");
                show_loading();
                if(encode_size(strlen(MAGIC_STRING),encInfo -> fptr_src_image, encInfo -> fptr_stego_image) == e_success)
                {
                    printf(GREEN"INFO : DONE \n"RESET);
                }
                else
                {
                    printf(RED"ERROR : Encoding Magic String Failed\n"RESET);
                    return e_failure;
                }
                printf("INFO : Encoding the magic string\n");
                show_loading();
                if(encode_magic_string(MAGIC_STRING , encInfo) == e_success)
                {
                    printf(GREEN"INFO : DONE \n"RESET);
                    printf("INFO : Encoding the secret file extension size\n");
                    show_loading();
                    if(encode_size(strlen(encInfo -> extn_secret_file), encInfo -> fptr_src_image ,encInfo -> fptr_stego_image) == e_success)
                    {
                        printf(GREEN"INFO : DONE\n"RESET);
                        printf("INFO : Encoding Secret file extension\n");
                        show_loading();
                        if(encode_secret_file_extn(encInfo -> extn_secret_file , encInfo) == e_success)
                        {
                            printf(GREEN"INFO : DONE\n"RESET);
                            printf("INFO : Encoding Secret File Size\n");
                            show_loading();
                            if(encode_secret_file_size(encInfo -> size_secret_file,encInfo) == e_success)
                            {
                                printf(GREEN"INFO : DONE\n"RESET);
                                printf("INFO : Encoding Secret file Data \n");
                                show_loading();
                                if(encode_secret_file_data(encInfo) == e_success)
                                {
                                    printf(GREEN"INFO : DONE\n"RESET);
                                    printf("INFO : Copying the remaining data\n");
                                    show_loading();
                                    if(copy_remaining_img_data(encInfo -> fptr_src_image , encInfo -> fptr_stego_image) == e_success)
                                    {
                                        printf(GREEN"INFO : DONE\n"RESET);

                                    }
                                    else
                                    {
                                        printf(RED"ERROR : Copying remaining data falied\n"RESET);
                                        return e_failure;
                                    }

                                }
                                else
                                {
                                    printf(RED"ERROR : Encoding secret file data failed\n"RESET);
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf(RED"ERROR : Encoding secret file size failed \n"RESET);
                                return e_failure;
                            }

                        }
                        else
                        {
                            printf(RED"ERROR : Encoding Secret file extension failed\n"RESET);
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf(RED"ERROR : Encoding file extension size failed\n"RESET);
                        return e_failure;
                    } 
                }
                else
                {
                    printf(RED"ERROR : Magic String Encoding Failed\n"RESET);
                    return e_failure;
                }
            }
            else
            {
                printf(RED"ERROR : Header copy failed \n"RESET);
                return e_failure;
            }
             
        }
        else
        {
            printf(RED"ERROR: Check capacity is a failure \n");
            return e_failure;
        }
    }
    else
    {
        printf(RED"ERROR : Opening required files is a failure\n");
        return e_failure;
    }
    return e_success;
}
Status encode_size(int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char buffer[32],data[32];
    fread(buffer,1,32,fptr_src_image);
    for(int i = 0; i < 32 ; i++)
        buffer[31-i] = (buffer[31-i] & ~1) | ((size >> i) & 1);
    fwrite(buffer,1,32,fptr_stego_image);
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i = 0; i < 8; i++)
        image_buffer[7-i] = (image_buffer[7-i] & ~1) | ((data >> i) & 1);
}
Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char buffer[8];
    for(int i = 0; i < size ; i++)
    {
        fread(buffer,1,8,fptr_src_image);
        encode_byte_to_lsb(data[i], buffer);
        fwrite(buffer,1,8,fptr_stego_image);
    }
    return e_success;
}
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    if(encode_data_to_image(file_extn,strlen(file_extn),encInfo -> fptr_src_image ,encInfo -> fptr_stego_image) == e_success)
        return e_success;
    else
        return e_failure;
}
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    if(encode_size(file_size, encInfo -> fptr_src_image , encInfo -> fptr_stego_image) == e_success)
        return e_success;
    else
        return e_failure;
}
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    int size;
    fseek(encInfo -> fptr_secret , 0,SEEK_END);
    size = ftell(encInfo -> fptr_secret);
    rewind(encInfo -> fptr_secret);
    char buffer[size];
    fread(buffer,1,size,encInfo -> fptr_secret);
    encode_data_to_image(buffer,size,encInfo -> fptr_src_image ,encInfo -> fptr_stego_image);
    return e_success;
}
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while(fread(&ch,1,1,fptr_src) > 0)
    {
        fwrite(&ch,1,1,fptr_dest);
    }
    return e_success;
}

Status close_files_encode(EncodeInfo *encInfo)
{
    fclose(encInfo -> fptr_secret);
    fclose(encInfo -> fptr_src_image);
    fclose(encInfo -> fptr_stego_image);
    return e_success;
}