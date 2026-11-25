/* compress_lz0.cpp -- EXPERIMENTAL glue for https://github.com/seb3773/lz0

   This file is part of the UPX executable compressor.

   UPX and the UCL library are free software; you can redistribute them
   and/or modify them under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.
 */

#include "../util/system_headers.h"
#include "../../vendor/lz0/lz0/lz0.h"
#include "../conf.h"
#include "compress.h"
#include "../util/membuffer.h"

int upx_lz0_compress(const upx_bytep src, unsigned src_len, upx_bytep dst, unsigned *dst_len,
                     upx_callback_t *cb_parm, int method, int level,
                     const upx_compress_config_t *cconf_parm, upx_compress_result_t *cresult) {
    assert(method == M_LZ0);
    UNUSED(cb_parm);
    UNUSED(cconf_parm);
    UNUSED(cresult);

    lz0_config cfg;
    lz0_config_level(&cfg, level > 9 ? 9 : level < 1 ? 1 : level);
    unsigned char *out = nullptr;
    size_t out_sz = 0;
    if (lz0_compress(src, src_len, &out, &out_sz, &cfg, nullptr) != 0)
        return UPX_E_ERROR;
    int r = UPX_E_OK;
    if (out_sz > *dst_len)
        r = UPX_E_OUTPUT_OVERRUN;
    else {
        memcpy(dst, out, out_sz);
        *dst_len = (unsigned) out_sz;
    }
    ::free(out);
    return r;
}

int upx_lz0_decompress(const upx_bytep src, unsigned src_len, upx_bytep dst, unsigned *dst_len,
                       int method, const upx_compress_result_t *cresult) {
    assert(method == M_LZ0);
    UNUSED(method);
    UNUSED(cresult);
    int64_t n = lz0_decompress_ex(src, src_len, dst, *dst_len);
    if (n < 0)
        return UPX_E_ERROR;
    *dst_len = (unsigned) n;
    return UPX_E_OK;
}

int upx_lz0_test_overlap(const upx_bytep buf, const upx_bytep tbuf, unsigned src_off,
                         unsigned src_len, unsigned *dst_len, int method,
                         const upx_compress_result_t *cresult) {
    assert(method == M_LZ0);
    MemBuffer b(src_off + src_len);
    memcpy(b + src_off, buf + src_off, src_len);
    unsigned saved_dst_len = *dst_len;
    int r = upx_lz0_decompress(raw_index_bytes(b, src_off, src_len), src_len,
                               raw_bytes(b, *dst_len), dst_len, method, cresult);
    if (r != UPX_E_OK)
        return r;
    if (*dst_len != saved_dst_len)
        return UPX_E_ERROR;
    if (tbuf != nullptr && memcmp(tbuf, b, *dst_len) != 0)
        return UPX_E_ERROR;
    return UPX_E_OK;
}

/* vim:set ts=4 sw=4 et: */
