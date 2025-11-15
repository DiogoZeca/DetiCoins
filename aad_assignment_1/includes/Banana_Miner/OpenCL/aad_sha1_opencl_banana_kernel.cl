#define SHA1_H0 0x67452301u
#define SHA1_H1 0xEFCDAB89u
#define SHA1_H2 0x98BADCFEu
#define SHA1_H3 0x10325476u
#define SHA1_H4 0xC3D2E1F0u

#define ROTL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

inline void sha1_hash(uint *data, uint *hash)
{
  uint h0 = SHA1_H0, h1 = SHA1_H1, h2 = SHA1_H2, h3 = SHA1_H3, h4 = SHA1_H4;
  uint a, b, c, d, e, w[80];
  
  for(int i = 0; i < 16; i++)
    w[i] = data[i];
  
  for(int i = 16; i < 80; i++)
    w[i] = ROTL(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);
  
  a = h0; b = h1; c = h2; d = h3; e = h4;
  
  for(int i = 0; i < 20; i++) {
    uint f = (b & c) | (~b & d);
    uint temp = ROTL(a, 5) + f + e + 0x5A827999u + w[i];
    e = d; d = c; c = ROTL(b, 30); b = a; a = temp;
  }
  for(int i = 20; i < 40; i++) {
    uint f = b ^ c ^ d;
    uint temp = ROTL(a, 5) + f + e + 0x6ED9EBA1u + w[i];
    e = d; d = c; c = ROTL(b, 30); b = a; a = temp;
  }
  for(int i = 40; i < 60; i++) {
    uint f = (b & c) | (b & d) | (c & d);
    uint temp = ROTL(a, 5) + f + e + 0x8F1BBCDCu + w[i];
    e = d; d = c; c = ROTL(b, 30); b = a; a = temp;
  }
  for(int i = 60; i < 80; i++) {
    uint f = b ^ c ^ d;
    uint temp = ROTL(a, 5) + f + e + 0xCA62C1D6u + w[i];
    e = d; d = c; c = ROTL(b, 30); b = a; a = temp;
  }
  
  hash[0] = h0 + a;
  hash[1] = h1 + b;
  hash[2] = h2 + c;
  hash[3] = h3 + d;
  hash[4] = h4 + e;
}

__kernel void mine_banana_coins(
    __global uint *coins_storage_area,
    uint base_value1,
    uint base_value2,
    uint iteration_offset)
{
  uint n = get_global_id(0);
  ulong counter = (ulong)iteration_offset + (ulong)n;
  
  uint data[14];
  uint hash[5];
  
  // BANANA FORMAT - Initialize coin data
  data[0] = 0x44455449u;  // "DETI"
  data[1] = 0x20636F69u;  // " coi"
  data[2] = 0x6E203220u;  // "n 2 "
  
  // BANANA: Words 3-6 = "TOSbananaCOIN "
  data[3] = 0x544F5362u;  // "TOSb"
  data[4] = 0x616E616Eu;  // "anan"
  data[5] = 0x61434F49u;  // "aCOI"
  data[6] = 0x4E202020u;  // "N   "
  
  // BANANA: Counter values moved to words 7-9
  data[7] = (uint)(counter & 0xFFFFFFFFu);
  data[8] = (uint)((counter >> 32) & 0xFFFFFFFFu);
  data[9] = base_value1;
  
  // Words 10-12: ZEROS
  data[10] = 0x00000000u;
  data[11] = 0x00000000u;
  data[12] = 0x00000000u;
  
  // Word 13: newline + padding
  data[13] = 0x00000A80u;
  
  // Compute SHA1
  {
    uint a = SHA1_H0;
    uint b = SHA1_H1;
    uint c = SHA1_H2;
    uint d = SHA1_H3;
    uint e = SHA1_H4;
    
    uint w[80];
    
    for(int i = 0; i < 14; i++)
      w[i] = data[i];
    
    w[14] = 0x00000000u;
    w[15] = 0x000001B8u;
    
    for(int i = 16; i < 80; i++)
      w[i] = ROTL(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);
    
    for(int i = 0; i < 20; i++) {
      uint f = (b & c) | (~b & d);
      uint temp = ROTL(a, 5) + f + e + 0x5A827999u + w[i];
      e = d; d = c; c = ROTL(b, 30); b = a; a = temp;
    }
    for(int i = 20; i < 40; i++) {
      uint f = b ^ c ^ d;
      uint temp = ROTL(a, 5) + f + e + 0x6ED9EBA1u + w[i];
      e = d; d = c; c = ROTL(b, 30); b = a; a = temp;
    }
    for(int i = 40; i < 60; i++) {
      uint f = (b & c) | (b & d) | (c & d);
      uint temp = ROTL(a, 5) + f + e + 0x8F1BBCDCu + w[i];
      e = d; d = c; c = ROTL(b, 30); b = a; a = temp;
    }
    for(int i = 60; i < 80; i++) {
      uint f = b ^ c ^ d;
      uint temp = ROTL(a, 5) + f + e + 0xCA62C1D6u + w[i];
      e = d; d = c; c = ROTL(b, 30); b = a; a = temp;
    }
    
    hash[0] = SHA1_H0 + a;
    hash[1] = SHA1_H1 + b;
    hash[2] = SHA1_H2 + c;
    hash[3] = SHA1_H3 + d;
    hash[4] = SHA1_H4 + e;
  }
  
  // Check signature
  if(hash[0] == 0xAAD20250u)
  {
    uint idx = atomic_add(coins_storage_area, 14u);
    
    if(idx + 14u <= 1024u)
    {
      for(uint i = 0; i < 14u; i++)
      {
        coins_storage_area[idx + i] = data[i];
      }
    }
  }
}
