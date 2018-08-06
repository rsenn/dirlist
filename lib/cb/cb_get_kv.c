void
cb_get_kv(const void* kv, void* value, size_t len) {
  const char* key = (const char*)kv;
  size_t keylen = str_len(key) + 1;
  memmove(value, key + keylen, len);
}
