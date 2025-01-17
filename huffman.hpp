#pragma once

#include <vector>

#include "utils.h"

namespace huffman {
static std::vector<unsigned> literal_lengths_freq = {
    22517, 2020, 1567, 1319, 1075, 889, 758, 811, 695, 766, 813, 667, 478, 423, 468, 566, 540, 501, 500, 446, 530, 511,
    537, 655, 677, 787, 824, 911, 807, 1037, 522, 430, 449, 366, 371, 339, 271, 239, 222, 224, 426, 209, 176, 203, 169,
    184, 180, 166, 166, 214, 136, 112, 137, 136, 164, 150, 157, 123, 126, 119, 147, 131, 146, 139, 142, 88, 101, 89, 84,
    73, 67, 81, 89, 64, 67, 54, 62, 59, 60, 67, 66, 68, 59, 51, 56, 67, 73, 75, 89, 52, 41, 43, 38, 42, 48, 38, 64, 28,
    29, 26, 34, 22, 33, 26, 24, 13, 31, 19, 15, 24, 19, 17, 21, 27, 10, 22, 28, 28, 20, 24, 14, 17, 11, 14, 28, 18, 18,
    14, 21, 17, 12, 24, 12, 21, 20, 15, 11, 15, 15, 12, 15, 15, 15, 19, 15, 12, 17, 17, 32, 14, 19, 12, 5, 10, 13, 11,
    12, 6, 7, 23, 13, 18, 6, 5, 14, 15, 18, 4, 8, 7, 9, 11, 5, 20, 5, 6, 6, 8, 11, 7, 9, 8, 8, 10, 7, 15, 3, 9, 5, 10,
    8, 9, 3, 4, 5, 10, 4, 6, 9, 13, 3, 3, 6, 2, 6, 5, 6, 7, 17, 5, 5, 3, 2, 6, 6, 8, 4, 11, 9, 10, 7, 9, 8, 5, 5, 6, 7,
    4, 5, 16, 15, 15, 14, 14, 28, 39, 37, 44, 66, 84, 100, 135, 160, 177, 252, 261, 336, 361, 419, 426, 405, 364, 295,
    257, 236, 215, 158, 114, 100, 37, 30, 31, 36, 38, 45, 32, 43, 61, 58, 37, 43, 37, 43, 44, 45, 46, 36, 55, 38, 55,
    53, 56, 61, 47, 32, 36, 31, 27, 32, 35, 39, 35, 43, 43, 33, 32, 28, 37, 24, 24, 25, 33, 33, 25, 36, 34, 38, 33, 40,
    36, 42, 18, 25, 19, 18, 32, 30, 25, 24, 22, 18, 14, 20, 7, 6, 8, 14, 10, 17, 22, 12, 8, 14, 7, 16, 10, 8, 12, 8, 7,
    10, 11, 24, 10, 5, 11, 11, 20, 10, 4, 6, 6, 5, 3, 7, 3, 3, 4, 2, 2, 8, 3, 5, 5, 7, 5, 2, 7, 6, 1, 2, 2, 3, 6, 3, 4,
    3, 6, 2, 5, 1, 1, 5, 2, 4, 2, 3, 3, 4, 1, 3, 5, 4, 5, 0, 4, 2, 2, 2, 2, 3, 1, 3, 4, 3, 2, 1, 3, 0, 1, 1, 3, 3, 5, 3,
    3, 0, 1, 1, 5, 2, 2, 2, 2, 1, 2, 2, 2, 4, 2, 2, 3, 3, 1, 3, 4, 3, 3, 0, 5, 2, 4, 3, 3, 3, 5, 2, 3, 7, 2, 1, 3, 3, 2,
    2, 3, 1, 1, 2, 1, 2, 2, 2, 4, 1, 3, 1, 0, 2, 0, 1, 4, 3, 2, 3, 3, 2, 1, 1, 3, 3, 2, 2, 1, 2, 4, 2, 3, 4, 1, 2, 1, 1,
    3, 6, 2, 8, 10, 6, 11, 4650
};

static std::vector<unsigned> cell_id_offset_freq = {
    923, 4455, 1159, 760, 810, 677, 501, 527, 501, 427, 381, 397, 294, 341, 322, 312, 297, 358, 347, 388, 382, 440, 346,
    345, 291, 235, 226, 194, 193, 174, 161, 173, 167, 146, 193, 209, 184, 215, 198, 206, 207, 228, 218, 225, 217, 190,
    192, 195, 156, 144, 121, 136, 112, 107, 130, 118, 128, 134, 152, 152, 157, 161, 154, 132, 139, 150, 148, 118, 121,
    108, 152, 94, 129, 136, 143, 113, 116, 78, 105, 99, 102, 108, 117, 93, 105, 119, 114, 97, 108, 109, 115, 97, 97, 98,
    87, 77, 77, 89, 93, 85, 70, 78, 94, 74, 99, 76, 98, 93, 102, 68, 65, 82, 79, 83, 81, 71, 69, 54, 81, 85, 102, 71,
    94, 85, 82, 93, 73, 61, 84, 74, 68, 66, 70, 90, 84, 96, 72, 67, 62, 62, 74, 82, 71, 85, 68, 80, 79, 70, 59, 51, 70,
    60, 73, 66, 58, 60, 56, 50, 72, 43, 48, 74, 51, 52, 63, 53, 60, 50, 49, 66, 55, 54, 57, 46, 47, 50, 61, 47, 41, 48,
    43, 65, 56, 50, 54, 50, 56, 51, 36, 51, 30, 39, 45, 54, 42, 48, 41, 48, 50, 54, 49, 37, 58, 59, 61, 51, 45, 47, 38,
    46, 38, 34, 62, 35, 40, 57, 44, 65, 52, 58, 49, 61, 45, 40, 23, 45, 35, 46, 48, 38, 40, 31, 36, 43, 36, 45, 37, 26,
    47, 29, 38, 40, 39, 37, 33, 38, 38, 48, 26, 35, 41, 45, 29, 43, 41, 37, 42, 42, 30, 38, 33, 33, 49, 38, 44, 35, 31,
    28, 37, 34, 37, 24, 35, 30, 28, 30, 33, 24, 44, 24, 36, 21, 25, 35, 29, 23, 34, 39, 40, 48, 19, 37, 46, 37, 26, 34,
    30, 30, 27, 22, 50, 34, 33, 37, 39, 26, 37, 29, 47, 26, 24, 24, 31, 59, 32, 61, 38, 19, 45, 38, 34, 24, 28, 28, 24,
    30, 31, 21, 20, 36, 26, 32, 36, 24, 28, 28, 24, 31, 15, 27, 38, 26, 38, 29, 26, 32, 31, 26, 17, 28, 38, 29, 70, 40,
    57, 22, 48, 29, 37, 21, 27, 39, 38, 27, 23, 34, 33, 26, 26, 40, 45, 23, 30, 35, 32, 71, 22, 107, 35, 38, 39, 37, 32,
    31, 23, 79, 26, 37, 30, 22, 27, 32, 24, 20, 16, 24, 11, 27, 23, 22, 28, 27, 20, 34, 39, 16, 35, 31, 19, 34, 36, 40,
    61, 21, 65, 31, 27, 31, 23, 34, 32, 23, 20, 26, 35, 21, 23, 30, 21, 20, 17, 29, 23, 18, 24, 15, 29, 27, 29, 22, 21,
    26, 16, 11, 22, 20, 17, 25, 19, 24, 28, 23, 21, 23, 20, 28, 30, 17, 26, 23, 28, 23, 27, 21, 23, 20, 29, 34, 38, 30,
    45, 29, 31, 29, 49, 22, 35, 32, 37, 29, 16, 23, 36, 15, 20, 22, 22, 16, 17, 22, 27, 28, 16, 21, 24, 24, 23, 22, 13,
    12, 16, 28, 17, 19, 24, 24, 42, 22, 12, 31, 22, 23, 31, 86, 27, 32, 27, 43, 24, 30, 17, 22, 21, 16, 16, 23, 30, 11,
    16, 23, 24, 19, 65, 25, 47, 18, 39, 19, 30, 18, 28, 21, 57, 25, 29, 27, 26, 109, 25, 53, 26, 48, 19, 46, 14, 36, 17,
    18, 18, 15, 55, 21, 27, 40, 25, 67, 20, 29, 16, 23, 17, 35, 14, 17, 20, 35, 28, 24, 16, 37, 16, 51, 26, 19, 10, 17,
    41, 42, 42, 40, 78, 28, 70, 28, 28, 23, 28, 61, 25, 32, 21, 30, 22, 30, 78, 28, 22, 28, 34, 16, 27, 20, 35, 25, 22,
    20, 17, 27, 41, 15, 25, 17, 29, 28, 46, 16, 23, 25, 24, 40, 22, 31, 18, 29, 15, 47, 15, 37, 20, 11, 22, 20, 15, 19,
    19, 12, 12, 12, 15, 15, 15, 14, 14, 17, 52, 20, 22, 17, 32, 15, 47, 17, 25, 25, 31, 19, 13, 32, 20, 28, 17, 30, 12,
    28, 21, 18, 12, 26, 12, 8, 21, 20, 19, 24, 15, 24, 18, 11, 41, 15, 13, 26, 17, 28, 13, 17, 12, 13, 11, 9, 14, 9, 18,
    11, 11, 20, 10, 13, 13, 13, 13, 16, 16, 8, 14, 17, 14, 19, 6, 16, 10, 14, 13, 16, 13, 12, 10, 6, 16, 13, 19, 13, 19,
    22, 17, 15, 13, 18, 7, 18, 16, 16, 13, 12, 21, 10, 20, 15, 15, 19, 15, 16, 21, 19, 17, 17, 15, 14, 19, 11, 13, 19,
    23, 16, 23, 13, 15, 15, 14, 20, 15, 14, 11, 12, 15, 13, 10, 13, 10, 15, 15, 15, 8, 17, 28, 41, 20, 22, 31, 14, 15,
    15, 19, 15, 11, 61, 19, 15, 17, 18, 15, 22, 13, 23, 13, 20, 15, 16, 14, 17, 12, 15, 13, 17, 22, 11, 29, 20, 23, 10,
    25, 10, 17, 20, 9, 14, 13, 14, 23, 12, 11, 18, 13, 14, 15, 14, 13, 13, 18, 8, 17, 22, 11, 7, 16, 22, 17, 13, 13, 12,
    12, 11, 12, 16, 13, 10, 15, 10, 13, 16, 12, 20, 12, 12, 14, 15, 18, 12, 15, 17, 8, 12, 17, 36, 20, 16, 18, 11, 7,
    14, 13, 21, 19, 19, 18, 16, 12, 18, 14, 14, 13, 12, 23, 16, 9, 24, 11, 48, 15, 75, 39, 13, 20, 13, 37, 17, 19, 22,
    17, 15, 22, 13, 24, 11, 17, 22, 89, 15, 49, 22, 17, 28, 47, 24, 17, 41, 11, 16, 31, 18, 26, 43, 55, 47, 70, 66, 53,
    77, 49, 48, 40, 24, 42, 41, 48, 47, 27, 58, 37, 58, 29, 30, 42, 11, 9, 24, 10, 42, 34, 42, 31, 35, 39, 15, 74, 19,
    48, 11, 127, 13, 30, 15, 30, 32, 26, 15, 13, 10, 18, 12, 35, 11, 11, 42, 29, 13, 23, 13, 33, 12, 44, 12, 21, 18, 23,
    11, 17, 9, 15, 7, 14870
};

static const std::string offset_within_cell_freq_str =
        "8v//////PQAABBha8W0lwPzXWhdD2mLXWixRlDTm2O3hS+hvrGFTJOTklRUC3GjtOiUthORqC+l7Maac3ZayvEDI6QolYzRlC+HWOgY5TRTDNPOLkUZCxLiDDIH8Ngag7BoCsFcIwPgXBaDgKEXg5x2CnGwNwho3C5ivI+HkdBmTpIUZENIt4gRHDmIEbkvCHxWC0C4FYLwUglCLDiIgYoYQVgkB2EIGIJgRgnB8HgOosg9h8DaEsIgSwkAwBkDkEYHAJgYAkBUFoHgLAWA8BcE4MghAyBIBgCoEALAhAcBgCIEgIAbAcBUDQFAOgSAsBABYEgPAQAaCICQHALAoAaBIBoEADASAEAgBICwHAPAWA4BQBQGAIAsBgA4CQGgKAOAgAwBABANAeAoBQAwGgEAcAsAQAwEgOAOAwBAFQDgJAGAgAYCQDACAEAYBIAwDgGAWAYAwCwFAIAoA0BwDAIARA4BUBgEwGgKAuBoD4LgqCSG6XM4qNEFm+OaYUspRANgoBgHsIAOwWgrBqEcLohxEh/DcC8IwPAlBCDMIIUgdRIBcDADUIAPAkBNDwGAJ4OArg1BCGcFgHgngoBSBwDoFAOgbBiDsIAQwmhiCUEMLwPgfAsBMBoD4LAWBHCsGoMIUQgAtBoCgEgJQYgjAmBcDgKQOgTBMDkG4NwmhICUB8IgVg3hpDMFMGoNgljYDgH8FIGQJASAsA0CgIgaBBBcDMF4MgRg/A2AgBYDADgWAYBICwFwMgkA0C4EYKwXgdA+BoDwFQJAYAYAsBYDgJgRAsCEFIPgLg5CQI0LIsAihIA8BwFwEgGgPAKAcBQEAHAUAYBECoFwNAYA+BQCoCQEAGAWAwBYDgHgGAoBACQDQHgTA8BwD4FgLAJAMAQAoBQGgGAcA8AYBwAAFAAAUA4CAEAKAcAYAwBAFAIAEAIAoAQBAKAWAYAoFADAEAKA0AwBgFAOAmAUBQCgHgNAWBcDgGwQgBAIAgA4CgBAIAKAoAoBQDgKAQBACIFgNggBACUH4LASgvBMCEEoHwbg4CCDQGQGgVAgBmBoDYJQZgjBKCQEIHwUg1DIFEVYPBRgyCoC0E4GQKgWAmBoCgGwSg1BWCAGYKAbAtA6BICAFwKAcAUBwCwEgOgsBGC4FAOQeg6C8DMGQKQNgbAmBMBwFAPAiAwB4DYJwKAcAoAgBIGQJgVAeCAFQFQUgWAkA0BwBwDgOAcBAC4EAPgSAaBMA4FwOAnA4BMEwGAQgRAuCACALgQAtCYEIIwQgagqBWCcGoWwiApCUDYDIJgPgdAkBICIEAJgZAgAwBABACgGAWA8BQCAJgNAWBUBYDwMgOAmBQD4HwLgOAYAsAgDAGgHAOAsDQDgTgTAgBICQCgEgOAUAcBgIQJAHAGAwBABgDAEAKAgAYCQBgCAGAAAYAwCAHAMAQA4BQEAFAIAEAAAwCAFAEAUAgBADAJAIAIAQBADABAEAMAgBwBACAQAkAgBgDAIAOAYAYBgFAOAYAEAIAwBgBAIAIAQAAAgCACAUAIAQCADACAYBIBAAgAAAAEAAAgAgCAMAEAQBwIADAEAQAQAwBADAEAEAoAACACAEAEAAAgCAEAIAMAYAwAABAGAUAwAwBAFACAMA4BQAgCAAAEAAAgCgDACAIAIAQCAFAKAMAIAwAABAEAQAoCAEgJAkA8AgAgAgDAAAAAIAgCABAIAMAIBABAGAAAMAIAgAAAACAEAAAAAAAAEAMAYAgBgDACAEAAAQAAAACAIAIAQBAAAAAMAAAAAgCACAAAAAQAgBAAAAAIAAAgCACAEAQAh5AIAQAAAgAAAAEEkACQIAISAA5wAAZwAECwAWCGMABAIANAIAQAsAEQALAJAgCACAEAIAAAg=";

static std::vector<unsigned> offset_within_cell_freq = decode_vector(offset_within_cell_freq_str);

static std::vector<unsigned> match_len_freq = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6245, 4828, 4048,
    3440, 2966, 2475, 2267, 1967, 1648, 1392, 1372, 1354, 1284, 1243, 1165, 1192, 1099, 988, 861, 799, 596, 544, 525,
    528, 520, 504, 475, 461, 387, 328, 373, 371, 433, 457, 602, 547, 487, 478, 344, 337, 294, 266, 239, 255, 259, 194,
    217, 191, 205, 190, 161, 125, 183, 150, 113, 136, 135, 164, 138, 146, 113, 73, 93, 98, 114, 118, 146, 138, 159, 117,
    107, 101, 94, 97, 84, 44, 66, 73, 84, 72, 60, 64, 56, 56, 36, 41, 34, 53, 48, 39, 36, 42, 43, 37, 51, 62, 48, 62,
    46, 36, 85, 87, 70, 80, 64, 73, 62, 87, 48, 75, 29, 32, 20, 16, 26, 22, 29, 16, 28, 13, 24, 19, 27, 24, 24, 21, 19,
    17, 27, 17, 22, 17, 12, 11, 22, 11, 16, 20, 8, 20, 13, 16, 13, 16, 10, 10, 12, 12, 12, 15, 13, 7, 19, 17, 20, 10,
    19, 16, 24, 16, 12, 30, 32, 39, 9, 20, 27, 20, 13, 18, 20, 5, 8, 8, 7, 14, 6, 10, 4, 10, 8, 10, 13, 14, 12, 10, 16,
    23, 15, 20, 11, 18, 21, 24, 24, 35, 46, 77, 78, 80, 96, 75, 85, 90, 52, 35, 31, 26, 21, 16, 18, 22, 21, 34, 55, 66,
    103, 125, 163, 227, 238, 241, 236, 231, 281, 783, 291, 110, 80, 56, 50, 45, 72, 69, 75, 297, 389, 478, 235, 121,
    114, 87, 90, 81, 82, 75, 46, 61, 49, 49, 62, 32, 40, 37, 44, 26, 24, 40, 18, 15, 27, 21, 26, 21, 24, 15, 17, 18,
    3659
};

static const std::vector<unsigned> d1_freq = {4894, 6201, 36038, 1867, 121, 0, 0, 0, 14591, 6, 25, 0, 0, 0, 0, 0,};

static const std::string data_size_freq_str = "8KEAAAQAcALAC5AUACALAGOAlhQIBqAtAD4BsACACIAEAGABoAQACAA4AEAGAA4AKACAAgAIAAgAIAAABBfABWB9AFgFQAzgGYCqFSgMoDWARgJBvYAOgjgBwArACABWACth4AQAtAB4A8A7gCgA/E5ABYC0AFAJgCqQqg2CNCBbgRABcAYgF4B4BMg/AAiAFADoDeAAgAwAmACgAIAKBpgBYAAAEgAAAAACAAAGKAKgCwwAwAAABYAAAcgCoAEACA4AYgAAAAEYASUAIADAEAAwGAGgBwAQAKgA8AQOAEgC4AeANgDoBAASABIAMAFAxgABHABQAABYAEAHAKIAAKAIgAAAWAEgRgASACMAA0YAAwIAE2AIAAICABKABwADPwAhgBcPAAMOABBgjgAFAgAQgGUBFAATABIgGAAQgBAAEBChAAARAAG2AARwAAAaAAACAAN+AAAgAAJ+ABEGCQABAgADFQACAgDwGAEAAQCqAAAAIAOADgDABuASgG4CuAaAG4BkAYgEYBUAJAAwAAAggMkB8iHAAHFWACgAIACACgAoAcAKABYAaAIACoAeAHADoBKAYAIICAAQgCYAkAJgAYAAAGj1ADIQAMDwACAgAhQC5AgAgwFOAXAEYA6AJAAI7gAB0gAyQABAvwDwE2AAgBgAEACABQAUAIgCQAeAFgBQBGAUgGQByAhAFgAeACh6AXBqACgBAA0AVQIDYgACAgBQA4ALAADeASABALYA8AFgBYAIAAgCAAiAKAFIBuAjHAAAnwBBAAAABQoBEgi3AUAoB4AExwEgAAAeAIAwFuH/gAAAKEYAYQIAAAEgGcsAkgAEAAoAAAFgOQ4AAK8AUBACgKaANwLwAwD7gDIBKAkAbAFWBKgXQDKALEsAEUL8ABBgAwGCMABgEQAEABAOAAJZACAASMsAAAIAIeAWHAABkwAhAAIHABNAHAATARsCAgIAAGgCMgQAAC0BECgcAAAOAAErAiEgeeoBQCgAAAx3ACAB4BgAAQIAEBjjATEAADwHAGGAsAAYAEAjAAKkAQAwAgAxAAAjAAApAgRUAFABgBoACKgAAWIAUAEAJgAQkwAQFDIAUACAIgAIhQAQIgwA8AkEgEoAkASADgBQACAB4AoAagGgDaAlAHAXA/ANBgAkAPAQwBQAJgAoAUAFACwBKAhAEoAiACgAoE0DYTAN4ASApFoCsAAUAFArgAEACgAgxgPzCggACAUAAwA4AJADIA6ASgI4CSA6ALIBcAMSAzAgAIBQARCAowAQCDEAAc4BAAIAUIgB4AGAywDwB+AAgAoBQAQAMQBkAYAJIB6ASADIAyB3AAMCAPAFwAKgHwAuAAgEgAIACgDIA6DWAXAOAAAfAJBAA0AQAAIAAAMNADEAEAIkASEAXwcAUGgBIA2ARAQDZQEhIAncBAG5AQHeAQEHAAMCAAcOAEAQAEACVAAAAgBDEAAgAgIDATYERBgAgAFeAQATAAT4AXAIAAIAqAAgmQIDAgBQCACgRoBBADAABQABAGDAAgAaADCdAQACACGgA6MAAwIABJMAAgIAEuBbAAN/AgGjAwACAAExAAM2BCEAAfADQIAAAAwbAAJJARGAfgBwAqACAAYAGAUEAF4BZUAAgCQAEN4AAHIDJAACYgACZQEwQA+ArwBEADAAAvsCEDhoAgBTAhAgBQADAgAQ6GAAATMAIAOABwBCIACAJhsAAA4AQuABAB4OAABNAANzAQMCAAQmAQICAAPDAhHAlQAAzAUAPwAA9QMBxwEBWwAB7gAwCAHgZwQBDgAQFrsAAuUBAhwARwAgBYCcABGgTQADAgAEfQJgAAAAAABw";

static std::vector<unsigned> data_size_freq = decode_vector(data_size_freq_str);

static const std::vector<unsigned> bytes_freq = {
    27607, 11162, 8390, 7775, 7088, 6488, 6849, 6130, 6481, 5568, 5865, 5248, 6321, 5567, 5578, 5533, 6295, 5288, 5176,
    5189, 5734, 4983, 4862, 5078, 6110, 4935, 5008, 4958, 5423, 4941, 4934, 5245, 6883, 5049, 4810, 4845, 5182, 4753,
    4880, 4792, 5529, 4689, 4810, 4837, 4920, 4797, 4844, 4982, 6243, 4856, 4848, 4919, 4815, 4868, 4820, 4794, 5467,
    4901, 4695, 4711, 4933, 4829, 4997, 5184, 8468, 4874, 5011, 5097, 5060, 4738, 4886, 4850, 5239, 4591, 4504, 4696,
    4833, 4696, 4626, 4936, 5823, 4766, 4759, 4652, 4667, 4544, 4741, 4766, 5070, 4589, 4783, 4784, 4913, 4548, 4774,
    4975, 6755, 4885, 4853, 4842, 4813, 4598, 4809, 4833, 5013, 4455, 4534, 4729, 4848, 4567, 4655, 4855, 5451, 4762,
    4887, 4716, 4932, 4704, 4488, 4755, 4923, 4826, 4774, 4806, 5140, 4939, 4989, 5347, 11100, 5340, 5113, 4952, 4948,
    4772, 4919, 4884, 5138, 4742, 4741, 4490, 4928, 4560, 4700, 4932, 5712, 4808, 4617, 4720, 4733, 4595, 4715, 4840,
    5097, 4772, 4686, 4531, 4929, 4626, 4624, 5149, 6417, 4762, 4771, 4700, 4771, 4567, 4635, 4599, 5017, 4618, 4846,
    4559, 4690, 4666, 4744, 4934, 5163, 4543, 4700, 4450, 4710, 4646, 4664, 4765, 5009, 4710, 4669, 4548, 4750, 4829,
    4951, 4911, 7789, 5020, 4774, 4754, 4931, 4651, 4604, 4780, 5234, 4617, 4672, 4925, 4936, 4585, 4624, 5021, 5449,
    4754, 4609, 4536, 4743, 4631, 4621, 4797, 4797, 4496, 4805, 4613, 4884, 4667, 4619, 4920, 6296, 4935, 4817, 4645,
    5004, 4931, 4794, 4979, 5172, 4620, 4707, 4678, 4550, 4601, 4710, 4934, 5618, 4723, 4977, 4816, 5022, 4751, 4746,
    4814, 5304, 5241, 4975, 5052, 5150, 5162, 5308, 9605,
};


struct Node {
    Ptr<Node> left{};
    Ptr<Node> right{};
    unsigned freq{};
    unsigned symbol{-1u};
    unsigned min_symbol{-1u};
    unsigned min_depth{-1u};

    explicit Node(unsigned symbol, unsigned freq) : symbol(symbol), freq(freq) {
        min_symbol = symbol;
        min_depth = 0;
    }

    explicit Node(Ptr<Node> left, Ptr<Node> right) : left(left), right(right) {
        freq = left->freq + right->freq;
        min_symbol = std::min(min_symbol, left->min_symbol);
        min_symbol = std::min(min_symbol, right->min_symbol);
        min_depth = std::min(left->min_depth, right->min_depth) + 1;
    }
};

struct HuffmanEncoderDecoder {
    // 0 ... n -> bitstrings
    // (code, length)
    std::vector<std::pair<uint64_t, uint8_t> > codes{};
    Ptr<Node> root{};

    explicit HuffmanEncoderDecoder(const std::vector<unsigned>& freq, bool verbose = false) {
        // build a tree
        auto cmp = [&](const Ptr<Node>& a, const Ptr<Node>& b) {
            if (a == b) {
                return false;
            }
            if (a->freq != b->freq) {
                return a->freq < b->freq;
            }
            if (a->min_depth != b->min_depth) {
                return a->min_depth < b->min_depth;
            }
            CHECK(a->min_symbol != b->min_symbol);
            return a->min_symbol < b->min_symbol;
        };

        unsigned sum_freq = 0;
        std::set<Ptr<Node>, decltype(cmp)> nodes(cmp);
        for (unsigned s = 0; s < freq.size(); ++s) {
            const auto f = std::max(1u, freq[s]);
            // if (freq[s] > 0) {
            nodes.insert(std::make_shared<Node>(s, f));
            sum_freq += f;
            // }
        }
        const unsigned cnt_alive_symbols = nodes.size();
        CHECK(!nodes.empty());
        while (nodes.size() > 1) {
            auto a = *nodes.begin();
            nodes.erase(nodes.begin());
            auto b = *nodes.begin();
            nodes.erase(nodes.begin());

            nodes.insert(std::make_shared<Node>(a, b));
        }
        root = *nodes.begin();

        // calculate codes
        codes.resize(freq.size());
        unsigned sum_code_lengths = 0;
        unsigned sum_code_lengths_weighted = 0;
        uint8_t max_code_length = 0;
        auto dfs = [&](auto&& self, Ptr<Node> v, uint64_t code, uint8_t code_length) {
            if (v->symbol != -1u) {
                CHECK(code_length <= 64);
                codes[v->symbol] = {code, code_length};
                sum_code_lengths += code_length;
                sum_code_lengths_weighted += code_length * v->freq;
                max_code_length = std::max(max_code_length, code_length);
                return;
            }

            self(self, v->left, code * 2, code_length + 1);

            self(self, v->right, code * 2 + 1, code_length + 1);
        };

        dfs(dfs, root, 0, 0);

        if (verbose) {
            std::cout << "HUFFMAN stats: " << std::endl;
            std::cout << "naive bits per symbol: " << int(len_in_bits(freq.size() - 1)) << std::endl;
            std::cout << "huffman bits per symbol: " << sum_code_lengths * 1.0 / freq.size() << std::endl;
            std::cout << "huffman bits per symbol weighted: " << sum_code_lengths_weighted * 1.0 / sum_freq <<
                    std::endl;
            std::cout << "cnt symbols with freq > 0: " << cnt_alive_symbols << " out of " << freq.size() << std::endl;
            std::cout << "max_code_length: " << int(max_code_length) << std::endl;
        }
    }

    unsigned encode_symbol(td::BitPtr& bit_ptr, const unsigned s) const {
        const auto& code = codes[s];
        // std::cout << "s = " << s << std::endl;
        CHECK(code.second > 0);
        bit_ptr.store_uint(code.first, code.second);
        bit_ptr.offs += code.second;
        return code.second;
    }

    unsigned decode_symbol(td::BitPtr& bit_ptr, const unsigned ptr_len = -1u) const {
        const unsigned initial_offs = bit_ptr.offs;
        CHECK(root);
        Ptr<Node> v = root;
        while (v->symbol == -1u) {
            if (ptr_len <= bit_ptr.offs) {
                CHECK(bit_ptr.offs == ptr_len);
                bit_ptr.offs = initial_offs;
                return -1u;
            }
            bool bit = bit_ptr.get_uint(1);
            bit_ptr.offs += 1u;
            if (bit) {
                v = v->right;
            } else {
                v = v->left;
            }
            CHECK(v);
        }
        CHECK(bit_ptr.offs <= ptr_len);
        CHECK(!v->left);
        CHECK(!v->right);
        return v->symbol;
    }
};

std::basic_string<uint8_t> encode_8(const std::basic_string<uint8_t>& data) {
    HuffmanEncoderDecoder encoder(bytes_freq);

    std::basic_string<uint8_t> result(data.size() * 2, 0);
    td::BitPtr bits(result.data(), 0);
    for (const auto byte: data) {
        encoder.encode_symbol(bits, byte);
    }
    bits.store_uint(1, 1);
    bits.offs += 1;
    const unsigned new_size = (bits.offs + 7) / 8;
    CHECK(result.size() >= new_size);
    result.resize(new_size);
    return result;
}

std::basic_string<uint8_t> decode_8(std::basic_string<uint8_t>& data) {
    HuffmanEncoderDecoder decoder(bytes_freq);

    std::basic_string<uint8_t> result;
    td::BitPtr bits(data.data(), 0);
    unsigned cnt_bits = data.size() * 8;
    CHECK(data.back() != 0);
    for (unsigned tail = 0; tail < 8; ++tail) {
        if ((data.back() >> tail) & 1) {
            cnt_bits -= (tail + 1);
            break;
        }
    }

    while (bits.offs < cnt_bits) {
        const auto symbol = decoder.decode_symbol(bits);
        result.push_back(symbol);
    }
    return result;
}
};
