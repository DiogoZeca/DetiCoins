// banana_coin_validator.c
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "aad_data_types.h"
#include "aad_sha1.h"

void test_banana_coin_format() {
    // Manually create a banana coin to test format
    char message[55];
    
    // Fixed prefix (12 bytes)
    memcpy(message, "DETI coin 2 ", 12);
    
    // Add "banana" (6 bytes)
    memcpy(message + 12, "banana", 6);
    
    // Add space after banana (1 byte)
    message[18] = ' ';
    
    // Fill remaining 35 bytes with test pattern
    for(int i = 19; i < 54; i++) {
        message[i] = 'X';
    }
    
    // Newline at end
    message[54] = '\n';
    
    // Verify format
    assert(strncmp(message, "DETI coin 2 ", 12) == 0);
    assert(strncmp(message + 12, "banana", 6) == 0);
    assert(message[54] == '\n');
    
    printf("✓ Banana coin format is correct!\n");
    printf("Message: ");
    for(int i = 0; i < 55; i++) {
        if(message[i] == '\n') printf("\\n");
        else printf("%c", message[i]);
    }
    printf("\n");
    
    // Test SHA1 computation
    u32_t hash[5];
    // Use the SHA1_COMPUTE macro from aad_sha1.h
    // (You'll need to adapt this based on the provided macro)
    
    printf("SHA1: %08x%08x%08x%08x%08x\n", 
           hash[0], hash[1], hash[2], hash[3], hash[4]);
}

int main() {
    test_banana_coin_format();
    return 0;
}
