#include <stdio.h>
#include "encode.h"
#include "types.h"
#include "decode.h"
#include <stdlib.h>
#include "common.h"


int main(int argc, char *argv[])
{
    system("clear");
    if(argc == 1)
    {
        printf(RED"Encoding: ./a.out -e <.bmp_file> <.text_file> [output file]\n"RESET);    
        printf(RED"Decoding: ./a.out -d <.bmp_file> [output file]\n"RESET);    
        return 0;
    }
    if(check_operation_type(argv) == e_encode)
    {
        printf("INFO : Selected Encoding\n");
        EncodeInfo encInfo;
        printf("INFO : Validating the given arguments\n");
        // Read and validate the arguments 
        if(read_and_validate_encode_args(argv,&encInfo) == e_success)
        {
            printf(GREEN"INFO : DONE\n"RESET);
            if(do_encoding(&encInfo) == e_success)          // Start Encoding
            {
                printf(GREEN"INFO : Encoding is successfull\n"RESET);
                close_files_encode(&encInfo);
            }
            else
            {
                printf(RED"ERROR : Encoding Failed\n"RESET);
            }
        }
        else
        {
            printf(RED"ERROR : Read and Validate is unsuccessful\n"RESET);
        }
    }
    else if(check_operation_type(argv) == e_decode)
    {
        printf("INFO : Selected Decoding\n");
        DecodeInfo decInfo;
        printf("INFO : Validating the given arguments\n");
        if(read_and_validate_decode_args(argv,&decInfo) == e_success)
        {
            printf(GREEN"INFO : DONE\n"RESET);
            // DO DECODING 
            if(do_decoding(&decInfo) == e_success)
            {
                printf(GREEN"INFO : Decoding is successfull\n"RESET);
                close_files_decode(&decInfo);
            }
            else
            {
                printf(RED"ERROR : Decoding failed\n"RESET);
            }

        }
        else
        {
            printf(RED"INFO : Read and Validate Failed \n"RESET);
        }
    }
    else
    {
        printf(RED"ERROR : Invalid Operation\n"RESET);
    }

}

