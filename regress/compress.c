/*
 * Copyright (c) 2022 Yubico AB. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#undef NDEBUG

#include <assert.h>
#include <string.h>

#include <openssl/sha.h>

#define _FIDO_INTERNAL

#include <fido.h>

/*
 * zlib compressed data (RFC1950); see https://www.ietf.org/rfc/rfc6713.txt
 */
static /* const */ unsigned char rfc1950_blob[694] = {
	0x78, 0x9c, 0xb5, 0x52, 0x3b, 0x6f, 0xdb, 0x30,
	0x10, 0xde, 0xf5, 0x2b, 0x0e, 0x99, 0x12, 0x40,
	0x75, 0x13, 0x4f, 0x45, 0x3b, 0xd1, 0x12, 0x6d,
	0x1d, 0x20, 0x8b, 0x2a, 0x49, 0xd9, 0xf5, 0x28,
	0x4b, 0x4c, 0x42, 0xc0, 0x12, 0x03, 0x3d, 0x12,
	0xe4, 0xdf, 0xf7, 0xc8, 0x3a, 0x88, 0xd3, 0x0c,
	0x9d, 0xea, 0xc1, 0x3e, 0xf3, 0x8e, 0xdf, 0xeb,
	0x98, 0xb8, 0xa7, 0xd7, 0xc1, 0x3e, 0x3c, 0x4e,
	0x70, 0xdd, 0xdc, 0xc0, 0xf2, 0xf6, 0xee, 0xdb,
	0x97, 0xe5, 0xed, 0x72, 0x09, 0x87, 0xf9, 0x68,
	0x1b, 0x07, 0x6c, 0xb5, 0x00, 0x76, 0x3a, 0x41,
	0x18, 0x19, 0x61, 0x30, 0xa3, 0x19, 0x9e, 0x4d,
	0xbb, 0x88, 0x22, 0x69, 0x5a, 0x3b, 0x4e, 0x83,
	0x3d, 0xce, 0x93, 0x75, 0x3d, 0xd4, 0x7d, 0x0b,
	0xf3, 0x68, 0xc0, 0xf6, 0x30, 0xba, 0x79, 0x68,
	0x4c, 0x38, 0x39, 0xda, 0xbe, 0x1e, 0x5e, 0xe1,
	0xde, 0x0d, 0xdd, 0x18, 0xc3, 0x8b, 0x9d, 0x1e,
	0xc1, 0x0d, 0xe1, 0xd7, 0xcd, 0x53, 0xd4, 0xb9,
	0xd6, 0xde, 0xdb, 0xa6, 0xf6, 0x00, 0x31, 0xd4,
	0x83, 0x81, 0x27, 0x33, 0x74, 0x76, 0x9a, 0x4c,
	0x0b, 0x4f, 0x83, 0x7b, 0xb6, 0x2d, 0x15, 0xd3,
	0x63, 0x3d, 0xd1, 0x97, 0x21, 0x90, 0xd3, 0xc9,
	0xbd, 0xd8, 0xfe, 0x01, 0x1a, 0xd7, 0xb7, 0xd6,
	0x5f, 0x1a, 0xfd, 0xa5, 0xa8, 0x33, 0xd3, 0xf7,
	0x28, 0x02, 0x80, 0xbb, 0x05, 0x7c, 0x54, 0x35,
	0x82, 0xbb, 0x7f, 0x93, 0xd3, 0xb8, 0xd6, 0x40,
	0x37, 0x8f, 0x13, 0x99, 0x98, 0x6a, 0x92, 0xe9,
	0x31, 0xeb, 0xa3, 0x7b, 0xf6, 0xad, 0x73, 0x06,
	0x1e, 0x84, 0x3e, 0xbd, 0x9b, 0x6c, 0x63, 0x62,
	0x9a, 0xb0, 0x23, 0x9c, 0x08, 0xcf, 0xc3, 0x5c,
	0x92, 0xf6, 0xed, 0x5f, 0x8a, 0x88, 0xb4, 0x39,
	0xd5, 0xb6, 0x33, 0xc3, 0xc2, 0x63, 0x2c, 0x3f,
	0x0b, 0x21, 0xc2, 0x8b, 0x30, 0xde, 0x84, 0x90,
	0xcb, 0x76, 0x26, 0x71, 0xff, 0x47, 0x0b, 0x91,
	0x9e, 0x51, 0xfc, 0x44, 0xeb, 0x9a, 0xb9, 0x33,
	0xfd, 0x54, 0xbf, 0xed, 0xeb, 0x2b, 0xad, 0xc2,
	0x51, 0x67, 0x80, 0xae, 0x9e, 0xcc, 0x60, 0xeb,
	0xd3, 0xf8, 0x1e, 0x7b, 0xd8, 0x15, 0x35, 0xcf,
	0x00, 0x97, 0x66, 0x68, 0xf9, 0x3a, 0x43, 0x05,
	0x4a, 0xac, 0xf5, 0x9e, 0x49, 0x0e, 0x54, 0x97,
	0x52, 0xec, 0x30, 0xe5, 0x29, 0xac, 0x0e, 0xa0,
	0x33, 0x0e, 0x89, 0x28, 0x0f, 0x12, 0x37, 0x99,
	0x86, 0x4c, 0xe4, 0x29, 0x97, 0x0a, 0x58, 0x91,
	0xd2, 0x69, 0xa1, 0x25, 0xae, 0x2a, 0x2d, 0xa4,
	0x8a, 0xae, 0x98, 0xa2, 0x9b, 0x57, 0xa1, 0xc1,
	0x8a, 0x03, 0xf0, 0x5f, 0xa5, 0xe4, 0x4a, 0x81,
	0x90, 0x80, 0xdb, 0x32, 0x47, 0x02, 0x23, 0x74,
	0xc9, 0x0a, 0x8d, 0x5c, 0xc5, 0x80, 0x45, 0x92,
	0x57, 0x29, 0x16, 0x9b, 0x18, 0x08, 0x00, 0x0a,
	0xa1, 0xa3, 0x1c, 0xb7, 0xa8, 0x69, 0x4c, 0x8b,
	0x38, 0x90, 0x7e, 0xbe, 0x06, 0x62, 0x0d, 0x5b,
	0x2e, 0x93, 0x8c, 0xfe, 0xb2, 0x15, 0xe6, 0xa8,
	0x0f, 0x81, 0x6f, 0x8d, 0xba, 0xf0, 0x5c, 0x6b,
	0x21, 0x23, 0x06, 0x25, 0x93, 0x1a, 0x93, 0x2a,
	0x67, 0x12, 0xca, 0x4a, 0x96, 0x42, 0x71, 0xf0,
	0xb6, 0x52, 0x54, 0x49, 0xce, 0x70, 0xcb, 0xd3,
	0x05, 0xb1, 0x13, 0x23, 0xf0, 0x1d, 0x2f, 0x34,
	0xa8, 0x8c, 0xe5, 0xf9, 0x47, 0x97, 0xd1, 0x1f,
	0x97, 0x5e, 0xfb, 0xa5, 0x47, 0x58, 0x71, 0xc8,
	0x91, 0xad, 0x72, 0xee, 0x99, 0x82, 0xcb, 0x14,
	0x25, 0x4f, 0xb4, 0xb7, 0xf3, 0x5e, 0x25, 0x94,
	0x1c, 0xe9, 0xcb, 0xe3, 0x48, 0x95, 0x3c, 0x41,
	0x2a, 0x28, 0x0c, 0x4e, 0x66, 0x98, 0x3c, 0xc4,
	0x67, 0x4c, 0xc5, 0x7f, 0x56, 0x34, 0x44, 0x4d,
	0x48, 0xd9, 0x96, 0x6d, 0xc8, 0xdb, 0xf5, 0x3f,
	0x22, 0xa1, 0x9d, 0x24, 0x95, 0xe4, 0x5b, 0xaf,
	0x99, 0x72, 0x50, 0xd5, 0x4a, 0x69, 0xd4, 0x95,
	0xe6, 0xb0, 0x11, 0x22, 0x0d, 0x41, 0x2b, 0x2e,
	0x77, 0x98, 0x70, 0xf5, 0x03, 0x72, 0xa1, 0x42,
	0x5a, 0x95, 0xe2, 0x71, 0x94, 0x32, 0xcd, 0x02,
	0x31, 0x41, 0x50, 0x54, 0xd4, 0xa6, 0x7a, 0x55,
	0x29, 0x0c, 0xa1, 0x61, 0xa1, 0xb9, 0x94, 0x55,
	0xa9, 0x51, 0x14, 0x37, 0xb4, 0xdf, 0x3d, 0xc5,
	0x42, 0x1a, 0x19, 0x5d, 0x4d, 0x43, 0xba, 0xa2,
	0xf0, 0x56, 0xe9, 0x91, 0x70, 0x21, 0x0f, 0x1e,
	0xd4, 0x67, 0x10, 0xc2, 0x8f, 0x61, 0x9f, 0x71,
	0x3a, 0x97, 0x3e, 0xd0, 0x90, 0x14, 0xf3, 0x11,
	0x28, 0x4a, 0x2c, 0xd1, 0x97, 0x63, 0xc4, 0x47,
	0x01, 0xea, 0xe8, 0xdd, 0x23, 0x14, 0x7c, 0x93,
	0xe3, 0x86, 0x17, 0x09, 0xf7, 0x5d, 0xe1, 0x51,
	0xf6, 0xa8, 0xf8, 0x0d, 0xed, 0x0a, 0x95, 0x1f,
	0xc0, 0x40, 0x4b, 0xdb, 0x27, 0xce, 0x2a, 0x58,
	0xf6, 0x3b, 0x22, 0x55, 0x51, 0x28, 0x2f, 0x5e,
	0x6c, 0x1c, 0x36, 0x09, 0xb8, 0x06, 0x96, 0xee,
	0xd0, 0xcb, 0x3e, 0x0f, 0xd3, 0xee, 0x15, 0x9e,
	0xdf, 0x49, 0x88, 0x2c, 0xc9, 0xce, 0x71, 0x2f,
	0xa2, 0xdf, 0xdf, 0xd7, 0x8e, 0x9c,
};

/*
 * expected sha256 of rfc1950_blob after decompression
 */
static const unsigned char rfc1950_blob_hash[SHA256_DIGEST_LENGTH] = {
	0x61, 0xc0, 0x4e, 0x14, 0x01, 0xb6, 0xc5, 0x2d,
	0xba, 0x15, 0xf6, 0x27, 0x4c, 0xa1, 0xcc, 0xfc,
	0x39, 0xed, 0xd7, 0x12, 0xb6, 0x02, 0x3d, 0xb6,
	0xd9, 0x85, 0xd0, 0x10, 0x9f, 0xe9, 0x3e, 0x75,

};

static const size_t rfc1950_blob_origsiz = 1322;

static /* const */ unsigned char random_words[515] = {
	0x61, 0x74, 0x68, 0x69, 0x72, 0x73, 0x74, 0x20,
	0x54, 0x68, 0x6f, 0x20, 0x63, 0x6f, 0x74, 0x20,
	0x73, 0x70, 0x6f, 0x66, 0x66, 0x79, 0x20, 0x4a,
	0x61, 0x76, 0x61, 0x6e, 0x20, 0x62, 0x72, 0x65,
	0x64, 0x65, 0x73, 0x20, 0x4c, 0x41, 0x4d, 0x20,
	0x6d, 0x69, 0x73, 0x2d, 0x68, 0x75, 0x6d, 0x69,
	0x6c, 0x69, 0x74, 0x79, 0x20, 0x73, 0x70, 0x69,
	0x67, 0x6f, 0x74, 0x20, 0x72, 0x65, 0x76, 0x6f,
	0x6c, 0x74, 0x69, 0x6e, 0x67, 0x6c, 0x79, 0x20,
	0x49, 0x6f, 0x64, 0x61, 0x6d, 0x6f, 0x65, 0x62,
	0x61, 0x20, 0x68, 0x79, 0x70, 0x6f, 0x68, 0x79,
	0x64, 0x72, 0x6f, 0x63, 0x68, 0x6c, 0x6f, 0x72,
	0x69, 0x61, 0x20, 0x76, 0x6f, 0x6c, 0x75, 0x6d,
	0x65, 0x74, 0x74, 0x65, 0x20, 0x61, 0x63, 0x72,
	0x69, 0x64, 0x69, 0x6e, 0x65, 0x20, 0x68, 0x6f,
	0x77, 0x6c, 0x20, 0x45, 0x75, 0x72, 0x79, 0x67,
	0x61, 0x65, 0x61, 0x6e, 0x20, 0x63, 0x6f, 0x6e,
	0x63, 0x65, 0x72, 0x74, 0x69, 0x6e, 0x69, 0x73,
	0x74, 0x20, 0x74, 0x65, 0x74, 0x72, 0x61, 0x70,
	0x6c, 0x6f, 0x69, 0x64, 0x20, 0x61, 0x75, 0x78,
	0x65, 0x74, 0x69, 0x63, 0x61, 0x6c, 0x20, 0x72,
	0x69, 0x70, 0x65, 0x2d, 0x67, 0x72, 0x6f, 0x77,
	0x6e, 0x20, 0x63, 0x6f, 0x6e, 0x63, 0x75, 0x72,
	0x72, 0x69, 0x6e, 0x67, 0x20, 0x6d, 0x79, 0x63,
	0x6f, 0x63, 0x65, 0x63, 0x69, 0x64, 0x69, 0x75,
	0x6d, 0x20, 0x50, 0x65, 0x64, 0x65, 0x72, 0x73,
	0x6f, 0x6e, 0x20, 0x74, 0x72, 0x61, 0x64, 0x69,
	0x74, 0x69, 0x6f, 0x6e, 0x2d, 0x62, 0x6f, 0x75,
	0x6e, 0x64, 0x20, 0x4c, 0x65, 0x6e, 0x67, 0x6c,
	0x65, 0x6e, 0x20, 0x70, 0x72, 0x65, 0x73, 0x62,
	0x79, 0x74, 0x65, 0x72, 0x61, 0x74, 0x65, 0x20,
	0x6c, 0x65, 0x63, 0x79, 0x74, 0x68, 0x69, 0x73,
	0x20, 0x63, 0x68, 0x61, 0x72, 0x61, 0x64, 0x72,
	0x69, 0x69, 0x66, 0x6f, 0x72, 0x6d, 0x20, 0x61,
	0x6c, 0x6c, 0x6f, 0x6b, 0x75, 0x72, 0x74, 0x69,
	0x63, 0x20, 0x75, 0x6e, 0x64, 0x69, 0x76, 0x69,
	0x73, 0x69, 0x76, 0x65, 0x6c, 0x79, 0x20, 0x70,
	0x73, 0x79, 0x63, 0x68, 0x6f, 0x6b, 0x79, 0x6d,
	0x65, 0x20, 0x75, 0x6e, 0x64, 0x65, 0x72, 0x73,
	0x74, 0x61, 0x6e, 0x64, 0x61, 0x62, 0x6c, 0x65,
	0x6e, 0x65, 0x73, 0x73, 0x20, 0x63, 0x75, 0x6c,
	0x74, 0x69, 0x73, 0x68, 0x20, 0x52, 0x65, 0x69,
	0x63, 0x68, 0x73, 0x74, 0x61, 0x67, 0x20, 0x75,
	0x6e, 0x63, 0x68, 0x6c, 0x6f, 0x72, 0x69, 0x6e,
	0x61, 0x74, 0x65, 0x64, 0x20, 0x6c, 0x6f, 0x67,
	0x6f, 0x67, 0x72, 0x61, 0x70, 0x68, 0x65, 0x72,
	0x20, 0x4c, 0x61, 0x69, 0x74, 0x68, 0x20, 0x74,
	0x77, 0x6f, 0x2d, 0x66, 0x61, 0x63, 0x65, 0x20,
	0x4d, 0x75, 0x70, 0x68, 0x72, 0x69, 0x64, 0x20,
	0x70, 0x72, 0x6f, 0x72, 0x65, 0x63, 0x69, 0x70,
	0x72, 0x6f, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e,
	0x20, 0x6c, 0x69, 0x62, 0x72, 0x65, 0x74, 0x74,
	0x69, 0x73, 0x74, 0x20, 0x49, 0x62, 0x69, 0x62,
	0x69, 0x6f, 0x20, 0x72, 0x65, 0x67, 0x72, 0x65,
	0x73, 0x73, 0x69, 0x6f, 0x6e, 0x73, 0x20, 0x63,
	0x6f, 0x6e, 0x64, 0x69, 0x67, 0x6e, 0x6e, 0x65,
	0x73, 0x73, 0x20, 0x77, 0x68, 0x69, 0x74, 0x65,
	0x2d, 0x62, 0x6f, 0x72, 0x64, 0x65, 0x72, 0x65,
	0x64, 0x20, 0x73, 0x79, 0x6e, 0x61, 0x70, 0x74,
	0x65, 0x6e, 0x65, 0x20, 0x68, 0x6f, 0x6c, 0x6f,
	0x6d, 0x6f, 0x72, 0x70, 0x68, 0x20, 0x6d, 0x6f,
	0x75, 0x6e, 0x74, 0x61, 0x69, 0x6e, 0x20, 0x4d,
	0x49, 0x54, 0x53, 0x20, 0x4c, 0x75, 0x6b, 0x61,
	0x73, 0x68, 0x20, 0x48, 0x6f, 0x72, 0x73, 0x65,
	0x79, 0x20, 0x0a,
};

static void
rfc1950_inflate(void)
{
	fido_blob_t in, out, dgst;

	memset(&in, 0, sizeof(in));
	memset(&out, 0, sizeof(out));
	memset(&dgst, 0, sizeof(dgst));
	in.ptr = rfc1950_blob;
	in.len = sizeof(rfc1950_blob);

	assert(fido_uncompress(&out, &in, rfc1950_blob_origsiz) == FIDO_OK);
	assert(out.len == rfc1950_blob_origsiz);
	assert(fido_sha256(&dgst, out.ptr, out.len) == 0);
	assert(dgst.len == sizeof(rfc1950_blob_hash));
	assert(memcmp(rfc1950_blob_hash, dgst.ptr, dgst.len) == 0);

	free(out.ptr);
	free(dgst.ptr);
}

static void
rfc1951_inflate(void)
{
	fido_blob_t in, out, dgst;

	memset(&in, 0, sizeof(in));
	memset(&out, 0, sizeof(out));
	memset(&dgst, 0, sizeof(dgst));
	in.ptr = rfc1950_blob + 2; /*  trim header */
	in.len = sizeof(rfc1950_blob) - 6; /* trim header (2), checksum (4) */

	assert(fido_uncompress(&out, &in, rfc1950_blob_origsiz) == FIDO_OK);
	assert(out.len == rfc1950_blob_origsiz);
	assert(fido_sha256(&dgst, out.ptr, out.len) == 0);
	assert(dgst.len == sizeof(rfc1950_blob_hash));
	assert(memcmp(rfc1950_blob_hash, dgst.ptr, dgst.len) == 0);

	free(out.ptr);
	free(dgst.ptr);
}

static void
rfc1951_reinflate(void)
{
	fido_blob_t in, out;

	memset(&in, 0, sizeof(in));
	memset(&out, 0, sizeof(out));
	in.ptr = random_words;
	in.len = sizeof(random_words);

	assert(fido_compress(&out, &in) == FIDO_OK);

	in.ptr = out.ptr;
	in.len = out.len;

	assert(fido_uncompress(&out, &in, sizeof(random_words)) == FIDO_OK);
	assert(out.len == sizeof(random_words));
	assert(memcmp(out.ptr, random_words, out.len) == 0);

	free(in.ptr);
	free(out.ptr);
}

int
main(void)
{
	fido_init(0);

	rfc1950_inflate();
	rfc1951_inflate();
	rfc1951_reinflate();

	exit(0);
}
