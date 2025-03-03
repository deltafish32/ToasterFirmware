#include "hud_font.h"

// https://www.dafont.com/perfect-dos-vga-437.font
// https://rop.nl/truetype2gfx/

const uint8_t Perfect_DOS_VGA_437_Win8pt7bBitmaps[] PROGMEM = {
  0x00, 0x6F, 0xFF, 0x66, 0x60, 0x66, 0xCF, 0x3C, 0xD2, 0x6C, 0xDB, 0xFB,
  0x66, 0xCD, 0xBF, 0xB6, 0x6C, 0x18, 0x31, 0xF6, 0x3C, 0x38, 0x1F, 0x03,
  0x07, 0x0F, 0x1B, 0xE1, 0x83, 0x00, 0xC3, 0x8C, 0x30, 0xC3, 0x0C, 0x31,
  0xC3, 0x38, 0xD9, 0xB1, 0xC7, 0x7B, 0xB3, 0x66, 0xCC, 0xEC, 0x6D, 0xE0,
  0x36, 0xCC, 0xCC, 0xCC, 0x63, 0xC6, 0x33, 0x33, 0x33, 0x6C, 0x66, 0x3C,
  0xFF, 0x3C, 0x66, 0x30, 0xCF, 0xCC, 0x30, 0x6D, 0xE0, 0xFE, 0xF0, 0x02,
  0x0C, 0x30, 0xC3, 0x0C, 0x30, 0x40, 0x38, 0xDB, 0x1E, 0x3D, 0x7A, 0xF1,
  0xE3, 0x6C, 0x70, 0x31, 0xCF, 0x0C, 0x30, 0xC3, 0x0C, 0x33, 0xF0, 0x7D,
  0x8C, 0x18, 0x61, 0x86, 0x18, 0x60, 0xC7, 0xFC, 0x7D, 0x8C, 0x18, 0x33,
  0xC0, 0xC1, 0x83, 0xC6, 0xF8, 0x0C, 0x38, 0xF3, 0x6C, 0xDF, 0xC3, 0x06,
  0x0C, 0x3C, 0xFF, 0x83, 0x06, 0x0F, 0xC0, 0xC1, 0x83, 0xC6, 0xF8, 0x38,
  0xC3, 0x06, 0x0F, 0xD8, 0xF1, 0xE3, 0xC6, 0xF8, 0xFF, 0x8C, 0x18, 0x30,
  0xC3, 0x0C, 0x18, 0x30, 0x60, 0x7D, 0x8F, 0x1E, 0x37, 0xD8, 0xF1, 0xE3,
  0xC6, 0xF8, 0x7D, 0x8F, 0x1E, 0x37, 0xE0, 0xC1, 0x83, 0x0C, 0xF0, 0xF0,
  0x3C, 0x6C, 0x00, 0xDE, 0x0C, 0x63, 0x18, 0xC1, 0x83, 0x06, 0x0C, 0xFC,
  0x00, 0x3F, 0xC1, 0x83, 0x06, 0x0C, 0x63, 0x18, 0xC0, 0x7D, 0x8F, 0x18,
  0x61, 0x83, 0x06, 0x00, 0x18, 0x30, 0x7D, 0x8F, 0x1E, 0xFD, 0xFB, 0xF7,
  0x60, 0x7C, 0x10, 0x71, 0xB6, 0x3C, 0x7F, 0xF1, 0xE3, 0xC7, 0x8C, 0xFC,
  0xCD, 0x9B, 0x37, 0xCC, 0xD9, 0xB3, 0x67, 0xF8, 0x3C, 0xCF, 0x0E, 0x0C,
  0x18, 0x30, 0x61, 0x66, 0x78, 0xF8, 0xD9, 0x9B, 0x36, 0x6C, 0xD9, 0xB3,
  0x6D, 0xF0, 0xFE, 0xCD, 0x8B, 0x47, 0x8D, 0x18, 0x31, 0x67, 0xFC, 0xFE,
  0xCD, 0x8B, 0x47, 0x8D, 0x18, 0x30, 0x61, 0xE0, 0x3C, 0xCF, 0x0E, 0x0C,
  0x1B, 0xF1, 0xE3, 0x66, 0x74, 0xC7, 0x8F, 0x1E, 0x3F, 0xF8, 0xF1, 0xE3,
  0xC7, 0x8C, 0xF6, 0x66, 0x66, 0x66, 0x6F, 0x1E, 0x18, 0x30, 0x60, 0xC1,
  0xB3, 0x66, 0xCC, 0xF0, 0xE6, 0xCD, 0x9B, 0x67, 0x8F, 0x1B, 0x33, 0x67,
  0xCC, 0xF0, 0xC1, 0x83, 0x06, 0x0C, 0x18, 0x31, 0x67, 0xFC, 0xC7, 0xDF,
  0xFF, 0xFD, 0x78, 0xF1, 0xE3, 0xC7, 0x8C, 0xC7, 0xCF, 0xDF, 0xFD, 0xF9,
  0xF1, 0xE3, 0xC7, 0x8C, 0x7D, 0x8F, 0x1E, 0x3C, 0x78, 0xF1, 0xE3, 0xC6,
  0xF8, 0xFC, 0xCD, 0x9B, 0x37, 0xCC, 0x18, 0x30, 0x61, 0xE0, 0x7D, 0x8F,
  0x1E, 0x3C, 0x78, 0xF1, 0xEB, 0xDE, 0xF8, 0x30, 0x70, 0xFC, 0xCD, 0x9B,
  0x37, 0xCD, 0x99, 0xB3, 0x67, 0xCC, 0x7D, 0x8F, 0x1B, 0x03, 0x81, 0x81,
  0xE3, 0xC6, 0xF8, 0xFF, 0xFB, 0x4C, 0x30, 0xC3, 0x0C, 0x31, 0xE0, 0xC7,
  0x8F, 0x1E, 0x3C, 0x78, 0xF1, 0xE3, 0xC6, 0xF8, 0xC7, 0x8F, 0x1E, 0x3C,
  0x78, 0xF1, 0xB6, 0x38, 0x20, 0xC7, 0x8F, 0x1E, 0x3D, 0x7A, 0xF5, 0xFF,
  0xEE, 0xD8, 0xC7, 0x8D, 0xB3, 0xE3, 0x87, 0x1F, 0x36, 0xC7, 0x8C, 0xCF,
  0x3C, 0xF3, 0x78, 0xC3, 0x0C, 0x31, 0xE0, 0xFF, 0x8E, 0x18, 0x61, 0x86,
  0x18, 0x61, 0xC7, 0xFC, 0xFC, 0xCC, 0xCC, 0xCC, 0xCF, 0x81, 0x83, 0x83,
  0x83, 0x83, 0x83, 0x83, 0x02, 0xF3, 0x33, 0x33, 0x33, 0x3F, 0x10, 0x71,
  0xB6, 0x30, 0xFF, 0xC6, 0x30, 0x78, 0x19, 0xF6, 0x6C, 0xD9, 0x9D, 0x80,
  0xE0, 0xC1, 0x83, 0xC6, 0xCC, 0xD9, 0xB3, 0x66, 0xF8, 0x7D, 0x8F, 0x06,
  0x0C, 0x18, 0xDF, 0x00, 0x1C, 0x18, 0x31, 0xE6, 0xD9, 0xB3, 0x66, 0xCC,
  0xEC, 0x7D, 0x8F, 0xFE, 0x0C, 0x18, 0xDF, 0x00, 0x39, 0xB6, 0x58, 0xF1,
  0x86, 0x18, 0x63, 0xC0, 0x77, 0x9B, 0x36, 0x6C, 0xD9, 0x9F, 0x06, 0xCC,
  0xF0, 0xE0, 0xC1, 0x83, 0x67, 0x6C, 0xD9, 0xB3, 0x67, 0xCC, 0x66, 0x0E,
  0x66, 0x66, 0x6F, 0x0C, 0x30, 0x07, 0x0C, 0x30, 0xC3, 0x0C, 0x3C, 0xF3,
  0x78, 0xE0, 0xC1, 0x83, 0x36, 0xCF, 0x1E, 0x36, 0x67, 0xCC, 0xE6, 0x66,
  0x66, 0x66, 0x6F, 0xED, 0xFF, 0x5E, 0xBD, 0x7A, 0xF1, 0x80, 0xDC, 0xCD,
  0x9B, 0x36, 0x6C, 0xD9, 0x80, 0x7D, 0x8F, 0x1E, 0x3C, 0x78, 0xDF, 0x00,
  0xDC, 0xCD, 0x9B, 0x36, 0x6C, 0xDF, 0x30, 0x61, 0xE0, 0x77, 0x9B, 0x36,
  0x6C, 0xD9, 0x9F, 0x06, 0x0C, 0x3C, 0xDC, 0xED, 0x9B, 0x06, 0x0C, 0x3C,
  0x00, 0x7D, 0x8D, 0x81, 0xC0, 0xD8, 0xDF, 0x00, 0x10, 0x60, 0xC7, 0xE3,
  0x06, 0x0C, 0x18, 0x36, 0x38, 0xCD, 0x9B, 0x36, 0x6C, 0xD9, 0x9D, 0x80,
  0xC7, 0x8F, 0x1E, 0x3C, 0x6D, 0x8E, 0x00, 0xC7, 0x8F, 0x5E, 0xBD, 0x7F,
  0xDB, 0x00, 0xC6, 0xD8, 0xE1, 0xC3, 0x8D, 0xB1, 0x80, 0xC7, 0x8F, 0x1E,
  0x3C, 0x78, 0xDF, 0x83, 0x0D, 0xF0, 0xFF, 0x98, 0x61, 0x86, 0x18, 0xFF,
  0x80, 0x1C, 0xC3, 0x0C, 0xE0, 0xC3, 0x0C, 0x30, 0x70, 0xFF, 0xFF, 0xF0,
  0xE0, 0xC3, 0x0C, 0x1C, 0xC3, 0x0C, 0x33, 0x80, 0x77, 0xB8 };

const GFXglyph Perfect_DOS_VGA_437_Win8pt7bGlyphs[] PROGMEM = {
  {     0,   1,   1,   9,    0,   12 },   // 0x20 ' '
  {     1,   4,  10,   9,    2,    3 },   // 0x21 '!'
  {     6,   6,   4,   9,    1,    2 },   // 0x22 '"'
  {     9,   7,   9,   9,    0,    4 },   // 0x23 '#'
  {    17,   7,  14,   9,    0,    1 },   // 0x24 '$'
  {    30,   7,   8,   9,    0,    5 },   // 0x25 '%'
  {    37,   7,  10,   9,    0,    3 },   // 0x26 '&'
  {    46,   3,   4,   9,    1,    2 },   // 0x27 '''
  {    48,   4,  10,   9,    2,    3 },   // 0x28 '('
  {    53,   4,  10,   9,    2,    3 },   // 0x29 ')'
  {    58,   8,   5,   9,    0,    6 },   // 0x2A '*'
  {    63,   6,   5,   9,    1,    6 },   // 0x2B '+'
  {    67,   3,   4,   9,    2,   10 },   // 0x2C ','
  {    69,   7,   1,   9,    0,    8 },   // 0x2D '-'
  {    70,   2,   2,   9,    3,   11 },   // 0x2E '.'
  {    71,   7,   8,   9,    0,    5 },   // 0x2F '/'
  {    78,   7,  10,   9,    0,    3 },   // 0x30 '0'
  {    87,   6,  10,   9,    1,    3 },   // 0x31 '1'
  {    95,   7,  10,   9,    0,    3 },   // 0x32 '2'
  {   104,   7,  10,   9,    0,    3 },   // 0x33 '3'
  {   113,   7,  10,   9,    0,    3 },   // 0x34 '4'
  {   122,   7,  10,   9,    0,    3 },   // 0x35 '5'
  {   131,   7,  10,   9,    0,    3 },   // 0x36 '6'
  {   140,   7,  10,   9,    0,    3 },   // 0x37 '7'
  {   149,   7,  10,   9,    0,    3 },   // 0x38 '8'
  {   158,   7,  10,   9,    0,    3 },   // 0x39 '9'
  {   167,   2,   7,   9,    3,    5 },   // 0x3A ':'
  {   169,   3,   8,   9,    2,    5 },   // 0x3B ';'
  {   172,   6,   9,   9,    1,    4 },   // 0x3C '<'
  {   179,   6,   4,   9,    1,    6 },   // 0x3D '='
  {   182,   6,   9,   9,    1,    4 },   // 0x3E '>'
  {   189,   7,  10,   9,    0,    3 },   // 0x3F '?'
  {   198,   7,   9,   9,    0,    4 },   // 0x40 '@'
  {   206,   7,  10,   9,    0,    3 },   // 0x41 'A'
  {   215,   7,  10,   9,    0,    3 },   // 0x42 'B'
  {   224,   7,  10,   9,    0,    3 },   // 0x43 'C'
  {   233,   7,  10,   9,    0,    3 },   // 0x44 'D'
  {   242,   7,  10,   9,    0,    3 },   // 0x45 'E'
  {   251,   7,  10,   9,    0,    3 },   // 0x46 'F'
  {   260,   7,  10,   9,    0,    3 },   // 0x47 'G'
  {   269,   7,  10,   9,    0,    3 },   // 0x48 'H'
  {   278,   4,  10,   9,    2,    3 },   // 0x49 'I'
  {   283,   7,  10,   9,    0,    3 },   // 0x4A 'J'
  {   292,   7,  10,   9,    0,    3 },   // 0x4B 'K'
  {   301,   7,  10,   9,    0,    3 },   // 0x4C 'L'
  {   310,   7,  10,   9,    0,    3 },   // 0x4D 'M'
  {   319,   7,  10,   9,    0,    3 },   // 0x4E 'N'
  {   328,   7,  10,   9,    0,    3 },   // 0x4F 'O'
  {   337,   7,  10,   9,    0,    3 },   // 0x50 'P'
  {   346,   7,  12,   9,    0,    3 },   // 0x51 'Q'
  {   357,   7,  10,   9,    0,    3 },   // 0x52 'R'
  {   366,   7,  10,   9,    0,    3 },   // 0x53 'S'
  {   375,   6,  10,   9,    1,    3 },   // 0x54 'T'
  {   383,   7,  10,   9,    0,    3 },   // 0x55 'U'
  {   392,   7,  10,   9,    0,    3 },   // 0x56 'V'
  {   401,   7,  10,   9,    0,    3 },   // 0x57 'W'
  {   410,   7,  10,   9,    0,    3 },   // 0x58 'X'
  {   419,   6,  10,   9,    1,    3 },   // 0x59 'Y'
  {   427,   7,  10,   9,    0,    3 },   // 0x5A 'Z'
  {   436,   4,  10,   9,    2,    3 },   // 0x5B '['
  {   441,   7,   9,   9,    0,    4 },   // 0x5C '\'
  {   449,   4,  10,   9,    2,    3 },   // 0x5D ']'
  {   454,   7,   4,   9,    0,    1 },   // 0x5E '^'
  {   458,   8,   1,   9,    0,   14 },   // 0x5F '_'
  {   459,   4,   3,   9,    2,    2 },   // 0x60 '`'
  {   461,   7,   7,   9,    0,    6 },   // 0x61 'a'
  {   468,   7,  10,   9,    0,    3 },   // 0x62 'b'
  {   477,   7,   7,   9,    0,    6 },   // 0x63 'c'
  {   484,   7,  10,   9,    0,    3 },   // 0x64 'd'
  {   493,   7,   7,   9,    0,    6 },   // 0x65 'e'
  {   500,   6,  10,   9,    1,    3 },   // 0x66 'f'
  {   508,   7,  10,   9,    0,    6 },   // 0x67 'g'
  {   517,   7,  10,   9,    0,    3 },   // 0x68 'h'
  {   526,   4,  10,   9,    2,    3 },   // 0x69 'i'
  {   531,   6,  13,   9,    1,    3 },   // 0x6A 'j'
  {   541,   7,  10,   9,    0,    3 },   // 0x6B 'k'
  {   550,   4,  10,   9,    2,    3 },   // 0x6C 'l'
  {   555,   7,   7,   9,    0,    6 },   // 0x6D 'm'
  {   562,   7,   7,   9,    0,    6 },   // 0x6E 'n'
  {   569,   7,   7,   9,    0,    6 },   // 0x6F 'o'
  {   576,   7,  10,   9,    0,    6 },   // 0x70 'p'
  {   585,   7,  10,   9,    0,    6 },   // 0x71 'q'
  {   594,   7,   7,   9,    0,    6 },   // 0x72 'r'
  {   601,   7,   7,   9,    0,    6 },   // 0x73 's'
  {   608,   7,  10,   9,    0,    3 },   // 0x74 't'
  {   617,   7,   7,   9,    0,    6 },   // 0x75 'u'
  {   624,   7,   7,   9,    0,    6 },   // 0x76 'v'
  {   631,   7,   7,   9,    0,    6 },   // 0x77 'w'
  {   638,   7,   7,   9,    0,    6 },   // 0x78 'x'
  {   645,   7,  10,   9,    0,    6 },   // 0x79 'y'
  {   654,   7,   7,   9,    0,    6 },   // 0x7A 'z'
  {   661,   6,  10,   9,    1,    3 },   // 0x7B '{'
  {   669,   2,  10,   9,    3,    3 },   // 0x7C '|'
  {   672,   6,  10,   9,    1,    3 },   // 0x7D '}'
  {   680,   7,   2,   9,    0,    2 } }; // 0x7E '~'

const GFXfont Perfect_DOS_VGA_437_Win8pt7b PROGMEM = {
  (uint8_t  *)Perfect_DOS_VGA_437_Win8pt7bBitmaps,
  (GFXglyph *)Perfect_DOS_VGA_437_Win8pt7bGlyphs,
  0x20, 0x7E, 16 };

// Approx. 1354 bytes
