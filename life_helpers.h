void update_map(int *map, int nrow, int ncol);
char *compress(int map[], size_t len);
int *decompress(char *snippet, size_t len);
int save_as_csv(char *name, char *snippet, size_t len);
char *read_from_csv(char *name);
