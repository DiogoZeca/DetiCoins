#ifndef AAD_COIN_TYPES_H
#define AAD_COIN_TYPES_H

#include <string.h>
#include "aad_data_types.h"


typedef enum {
    COIN_TYPE_DETI = 0,     
    COIN_TYPE_CUSTOM = 1    
} coin_type_t;


// Coin configuration structure
typedef struct {
    coin_type_t type;
    const char *custom_text;
} coin_config_t;

// Initialize coin configuration
static inline coin_config_t coin_config_init(coin_type_t type, const char *custom_text) {
    coin_config_t config;
    config.type = type;
    config.custom_text = custom_text;
    return config;
}


// custom text
static inline int validate_custom_text(const char *text) {
    if (!text || strlen(text) == 0) {
        return 0;  
    }
    // length check
    size_t len = strlen(text);
    if (len > 27) {
        return 0;  
    }

    // newlines
    for (size_t i = 0; i < len; i++) {
        if (text[i] == '\n') {
            return 0;  
        }
    }

    return 1;  
}

//little-endian
static inline int encode_custom_text(u32_t coin[14], const char *text, int start_word) {
    size_t len = strlen(text);
    const u08_t *bytes = (const u08_t *)text;
    int word_idx = start_word;

    for (size_t i = 0; i < len; i += 4) {
        u32_t word = 0;
        for (int j = 0; j < 4 && (i + j) < len; j++) {
            word |= ((u32_t)bytes[i + j]) << (8 * (3 - j));  
        }
        coin[word_idx++] = word;

        if (word_idx >= 13) {
            break;  
        }
    }

    return word_idx;
}

#endif
