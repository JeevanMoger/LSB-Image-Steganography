#include <stdio.h>
#include "types.h"
#include <string.h>
#include "common.h"
#include <unistd.h>
#include "decode.h"

char ch;
int magic_string_len;
int flag,j,length;
char main_buffer[5];

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if (argv[2] != NULL && strstr(argv[2], ".bmp") != NULL)
    {
        decInfo->stego_image_fname = argv[2];
    }
    else
    {
        printf(RED"Decoding: ./lsb_steg -d <.bmp_file> [output file]\n"RESET);
        return e_failure;
    }

    if (argv[3] != NULL)
    {
        char *output_name = strtok(argv[3], ".");
        if (output_name != NULL)
        {
            strcpy(decInfo->output_fname, output_name);
        }
    }
    else
    {
        strcpy(decInfo->output_fname, "decoded");
    }

    return e_success;
}

Status open_files_decode(DecodeInfo *decInfo)
{
    decInfo -> fptr_stego_image = fopen(decInfo -> stego_image_fname , "r" );
    if(decInfo -> fptr_stego_image == NULL)
    {
        perror("fopen");
    	fprintf(stderr, RED"ERROR: Unable to open file %s\n"RESET, decInfo->stego_image_fname);

    	return e_failure;
    }
    return e_success;
}

Status decode_size(FILE *fptr_stego_image)
{
    length= 0;
    char buffer[32];
    fread(buffer,1,32,fptr_stego_image);
    for(int i = 0; i < 32 ; i++)
    {
        length = length | ((buffer[31 - i] & 1) << i);
    }
    return e_success;
}

Status decode_magic_string(int size , DecodeInfo *decInfo)
{
    if(decode_data_from_image(size,decInfo -> fptr_stego_image ,decInfo -> fptr_output_file) == e_success)
        return e_success;
    else
        return e_failure;
}

Status decode_data_from_image(int size, FILE *fptr_stego_image, FILE *fptr_output_file)
{
    char buffer[8];
    j = 0;
    for(int i = 0; i < size ; i++)
    {
        fread(buffer,1,8,fptr_stego_image);
        decode_lsb_from_byte(buffer);
        if(flag == 1)
            fputc(ch , fptr_output_file);
    }
    return e_success;  
}

Status decode_lsb_from_byte(char *data)
{
    ch = 0;
    for(int i = 0; i < 8 ; i++)
        ch = ch | ((data[7 - i] & 1) << i);
    if(flag == 0)
    {
        main_buffer[j++] = ch;
    }
}

Status check_magic_string(char *data)
{
    if(strcmp(data, MAGIC_STRING) == 0)
        return e_success;
    else
        return e_failure;
}

Status decode_secret_file_extension(int size, DecodeInfo *decInfo)
{
    if(decode_data_from_image(size,decInfo -> fptr_stego_image ,decInfo -> fptr_output_file) == e_success)
        return e_success;
    else
        return e_failure;
}
Status Create_output_file(char *extn, DecodeInfo *decInfo)
{
    strcat(decInfo -> output_fname,extn);
    decInfo -> fptr_output_file = fopen(decInfo -> output_fname , "w" );
    if(decInfo -> fptr_stego_image == NULL)
    {
        perror("fopen");
    	fprintf(stderr,RED"ERROR: Unable to open file %s\n"RESET, decInfo->output_fname);

    	return e_failure;
    }
    printf("INFO : Created Output File %s\n",decInfo -> output_fname);
    return e_success;
}

Status check_magic_string_len(int len)
{
    if(len == strlen(MAGIC_STRING))
        return e_success;
    else
        return e_failure;
}

Status decode_secret_data(int size , DecodeInfo *decInfo)
{
    flag = 1;
    if(decode_data_from_image(size,decInfo -> fptr_stego_image ,decInfo -> fptr_output_file) == e_success)
        return e_success;
    else
        return e_failure;   
}

Status do_decoding(DecodeInfo *decInfo)
{
    printf("INFO : Opening Required Files \n");
    if(open_files_decode(decInfo) == e_success)
    {
        printf(GREEN"INFO : Done\n"RESET);
        printf(YELLOW"==========Decoding procedure started==========\n"RESET);
        rewind(decInfo -> fptr_stego_image);
        fseek(decInfo -> fptr_stego_image, 54, SEEK_SET);
        printf("INFO : Decoding magic string size \n");
        show_loading();
        if(decode_size(decInfo -> fptr_stego_image) == e_success)
        {
            printf(GREEN"INFO : Done\n"RESET);
            decInfo ->  magic_string_size = length;
            printf("INFO : Magic String Size is %d\n",decInfo -> magic_string_size);
            printf("INFO : Check Magic String Length\n");
            show_loading();
            if(check_magic_string_len(decInfo -> magic_string_size) == e_success)
            {
                printf(GREEN"INFO : Done\n"RESET);
                printf("INFO : Decoding Magic String\n");
                show_loading();
            }
            else
            {
                printf(RED"ERROR : Magic String did not match\n"RESET);
                return e_failure;
            }
            if(decode_magic_string(decInfo -> magic_string_size ,decInfo) == e_success)
            {
                printf(GREEN"INFO : Done \n"RESET);
                strcpy(decInfo -> magic_string , main_buffer);
                printf("INFO : Checking Magic String is valid or not\n");
                show_loading();
                if(check_magic_string(decInfo -> magic_string) == e_success)
                {
                    printf(GREEN"INFO : Done \n"RESET);
                    printf("Info : Decoding the File Extension size\n");
                    show_loading();
                        if(decode_size(decInfo -> fptr_stego_image) == e_success)
                        {
                            decInfo -> secret_file_extn_size = length;
                            printf(GREEN"INFO : Done \n"RESET);
                            printf("INFO : Extension Size is %d \n",decInfo -> secret_file_extn_size);
                            printf("INFO : Decoding Secret File Extension \n");
                            show_loading();
                            if(decode_secret_file_extension(decInfo -> secret_file_extn_size ,decInfo) == e_success)
                            {
                                printf(GREEN"INFO : Done\n"RESET);
                                strcpy(decInfo -> secret_file_extn ,main_buffer);
                                printf("INFO : Creating Output File \n");
                                show_loading();
                                if(Create_output_file(decInfo -> secret_file_extn , decInfo) == e_success)
                                {
                                    printf(GREEN"INFO : Done\n"RESET);
                                    printf("INFO : Decoding Secret file Size\n");
                                    show_loading();
                                    if(decode_size(decInfo -> fptr_stego_image) == e_success)
                                    {
                                        printf(GREEN"INFO : DONE \n"RESET);
                                        decInfo -> secret_file_size = length;
                                        printf("INFO : Secret file Size is %d \n",decInfo -> secret_file_size);
                                        printf("INFO : Decoding The Secret Data\n");
                                        show_loading();
                                        if(decode_secret_data(decInfo -> secret_file_size , decInfo) == e_success)
                                        {
                                            printf(GREEN"INFO : Done \n"RESET);
                                        }
                                        else
                                        {
                                            printf(RED"ERROR : Decoding Secret Data Failed\n"RESET);
                                            return e_failure;
                                        }

                                    }
                                    else
                                    {
                                        printf(RED"ERROR : Decoding Secret File Size Failed\n"RESET);
                                        return e_failure;
                                    }
                                }
                                else
                                {
                                    printf(RED"ERORR : Creation of Output File Failed\n"RESET);
                                    return e_failure;
                                }
                                
                            }
                            else
                            {
                                printf(RED"INFO : Decoding Secret File Extension Failed\n"RESET);
                                return e_failure;
                            }
                        }
                        else
                        {
                            printf(RED"ERROR : Decoding Secret file extension size failed\n"RESET);
                            return e_failure;
                        }
                    

                }
                else
                {
                    printf(RED"ERROR : Magic String validation failed\n"RESET);
                    return e_failure;
                }
            }
            else
            {
                printf(RED"ERROR : Decoding Magic String Failed\n"RESET);
                return e_failure;
            }
        }
        else
        {
            printf(RED"ERROR : Decoding magic string size failed\n"RESET);
            return e_failure;
        }

    }
    else
    {
        printf(RED"ERROR : Opening Files failed\n"RESET);
        return e_failure;
    }
    return e_success;
}

void show_loading()
{
    for(int i = 0; i <= 100; i+=2)
    {
        printf("\rLoading : [%-50s] %d%%","--------------------------------------------------"+(50-i/2),i);
        fflush(stdout);
        usleep(20000);
    }
    printf("\n");
}

Status close_files_decode(DecodeInfo *decInfo)
{
    fclose(decInfo -> fptr_stego_image);
    fclose(decInfo -> fptr_output_file);
}