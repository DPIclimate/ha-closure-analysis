#include "Ubidots/authenticate.h"

int8_t Ubidots_GetToken(const char* env_var_name){
    char* token = getenv(env_var_name);
    if(token == NULL) return 1; // Not found error

    strncpy(UBIDOTS_TOKEN, token, UBIDOTS_TOKEN_SIZE);
    fprintf(stdout, "[Info]: Ubidots access token found and initialised.\n");
    return 0;
}

int8_t Ubidots_CheckAccess(){
    if(strlen(UBIDOTS_TOKEN) == 0){
        fprintf(stderr, "[Error]: Ubidots access token not found. Trying again"
                        " with the default environmental variable name: %s\n",
                        UBIDOTS_DEFAULT_ENV_NAME);
        if(Ubidots_GetToken(UBIDOTS_DEFAULT_ENV_NAME) == 0){
            return 0;
        }
        fprintf(stderr, "[Error]: Ubidots access token not found with default"
                        " token name: %s\n", UBIDOTS_DEFAULT_ENV_NAME);
        return 1;
    }
    return 0;
}