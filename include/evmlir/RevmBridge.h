#pragma once
#include <cstddef>
#include <cstdint>

extern "C" {
void *revm_new();
bool revm_deploy(void *state, const uint8_t *bytecode, size_t len);
bool revm_call(void *state, const uint8_t *calldata, size_t calldata_len,
               uint8_t *out_data, size_t out_cap, size_t *out_len);
void revm_free(void *state);
}
