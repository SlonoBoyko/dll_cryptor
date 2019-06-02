/* ========================== */
/*     © SlonoBoyko 2018      */
/* ========================== */



void Log(const char *text, ...);

void xtea_decode(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]);
void xtea_encode(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]);

uint32_t *generate_key();

char *b64_encode(const unsigned char *src, size_t len);
unsigned char *b64_decode(const char *src, size_t len);











