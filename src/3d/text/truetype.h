// See end of file for license.

#ifndef TRUETYPE_H_INCLUDED
#define TRUETYPE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>


// You may provide your own assert function by defining TT_ASSERT before
// including this header. Otherwise, assertions are disabled.

#ifdef TT_ASSERT
#define ttAssert(x) TT_ASSERT(x)
#else
#define ttAssert(x) do { (void) (x); } while (0)
#endif


// Forward declarations of public structure types.

typedef struct TTFont TTFont;
typedef struct TTPoint TTPoint;
typedef struct TTDecomposeFuncs TTDecomposeFuncs;


// Structure representing a read-only range of TrueType file data. Only used
// internally; it's declared here because it's needed for the TTFont structure.

typedef struct TTRange {
    const uint8_t* data;
    size_t size;
} TTRange;


// A context structure for a single font. A pointer to an object of this type is
// passed as the first parameter to all functions that source data out of a font
// file. This type is declared publicly so that you can allocate it any way you
// like, but its data members should be considered private.

struct TTFont
{
    TTRange cmap;
    TTRange glyf;
    TTRange head;
    TTRange hhea;
    TTRange hmtx;
    TTRange kern;
    TTRange loca;
    TTRange maxp;
};


// Initializes a font context with the data of an in-memory font file
// `fontData`, which is `fileSize` bytes long. Returns a nonzero value on
// success, or zero if the file is invalid or of an unsupported format. This
// function must be called before any other function that uses the font context.
// There is no corresponding "finalizer" function; you must just make sure that
// the memory at `fontData` remains valid until you are finished using the
// context.

int ttInitFont(TTFont* font,
               const void* fileData,
               size_t fileSize);


// Returns the total number of glyphs in a font. This number is at most 65535,
// due to limitations of the TrueType file format. All valid glyph IDs in a font
// range from 0 to N - 1, inclusive, where N is the value returned by this
// function.

int ttGetGlyphCount(const TTFont* font);


// Gets the coordinates of a font's bounding box. This is a box that should be
// large enough to enclose all of the font's glyphs. The minimum coordinates are
// written to `xMin` and `yMin`, and the maximum coordinates to `xMax` and
// `yMax`. Any output parameter that's set to null is safely ignored. Returns a
// nonzero value on success.

int ttGetFontBox(const TTFont* font,
                 int* xMin, int* yMin,
                 int* xMax, int* yMax);


// Gets the vertical metrics of a font. This includes `ascent`, which is the
// highest rise of a glyph outline above the baseline; `descent`, which is the
// lowest descent below the baseline; and `lineGap`, which is the amount of
// extra vertical space that should be added between consecutive lines of text,
// on top of the visible height implied by the difference between `ascent` and
// `descent`. Any output parameter that's set to null is safely ignored. Returns
// a nonzero value on success.

int ttGetFontMetrics(const TTFont* font,
                     int* ascent,
                     int* descent,
                     int* lineGap);


// Returns a scaling factor for laying out text that's a given number of pixels
// tall. Any unscaled coordinates and metrics should be multiplied by this value
// to get their visual extents for a font of the specified pixel size.

float ttGetPixelScale(const TTFont* font,
                      float pixels);


// Returns a scaling factor for laying out text using a given point size. Any
// unscaled coordinates and metrics should be multiplied by this value to get
// their visual extents for a font of that size. The parameter `dpi` is the
// resolution, in dots per inch, of the display device, and is used to determine
// the size of a typographic point. You can use a DPI of 96 to get a scale that
// matches that of the CSS `pt` unit.

float ttGetPointScale(const TTFont* font,
                      float points,
                      float dpi);


// Returns the glyph ID for a given Unicode code point. If the font doesn't have
// a glyph for that code point, then the special glyph ID 0 is returned. Glyph
// zero conventionally represents a "missing" glyph, which most fonts render as
// a plain rectangle.

int ttGetGlyphID(const TTFont* font,
                 int codePoint);


// Retrieves the points representing the shape of a glyph outline. Writes at
// most `maxPoints` points to the user-allocated array `points`, and returns the
// number of points that were actually written. This number will be zero for
// glyphs that have no visible features, such as those for whitespace
// characters. If `points` is null, then `maxPoints` is ignored, and instead the
// function returns the total number of points in the glyph's outline.

size_t ttGetGlyphOutline(const TTFont* font,
                         int glyphID,
                         TTPoint* points,
                         size_t maxPoints);


// A type representing a single point in a glyph outline. `x` and `y` are the
// point's coordinates, in unscaled font units, and `flags` is a bit mask
// combining any of the following flags:
//
//   - `TT_POINT_ON_CURVE`, which indicates that the point is on the curve.
//     Otherwise, it's an off-curve point that only contributes to the curvature
//     of the outline.
//   - `TT_POINT_FIRST`, which marks the first point in a contour.
//   - `TT_POINT_LAST`, which marks the last point in a contour.

struct TTPoint
{
    int16_t x, y;
    uint8_t flags;
};

enum TTPointFlags
{
    TT_POINT_ON_CURVE = 0x01,
    TT_POINT_FIRST    = 0x02,
    TT_POINT_LAST     = 0x04,
};


// Calculates the bounding box of a glyph outline. `points` should be an array
// of `numPoints` points, such as one written by `ttGetGlyphOutline`. The
// minimum coordinates are written to `xMin` and `yMin`, and the maximum
// coordinates to `xMax` and `yMax`. The difference between the minimum and
// maximum coordinates can be used to calculate the size of the glyph, and the
// minimum coordinates give the offset relative to the current point when laying
// out the glyph in a string of text. Returns a nonzero value on success. Any
// output parameter that's set to zero is safely ignored.

int ttGetGlyphBox(const TTPoint* points,
                  size_t numPoints,
                  int* xMin, int* yMin,
                  int* xMax, int* yMax);


// Gets the horizontal advance metric of a glyph. This is the amount by which
// the current point should move after drawing the glyph. Any kerning that's
// applied should be added to this value when laying out the next glyph in a
// string of text.

int ttGetGlyphAdvance(const TTFont* font,
                      int glyphID);


// Gets the kerning advance for a pair of glyphs. This value should be added to
// the normal advance value of the left-hand glyph before laying out the right-
// hand glyph. Returns zero if the font doesn't provide kerning data, either at
// all or for the specified pair of glyphs. Note that the kerning advance may be
// negative, in which case the gap between the glyphs is narrowed instead of
// widened.

int ttGetGlyphKerning(const TTFont* font,
                      int leftGlyphID,
                      int rightGlyphID);


// Decomposes a glyph outline, converting it to a series of line segments and
// quadratic Bézier curves. `points` should be an array of `numPoints` points,
// such as one written by `ttGetGlyphOutline`. The `funcs` structure should be
// initialized with callbacks that are invoked for each segment in the
// decomposed outline (see details below). The `userPtr` parameter is an opaque
// pointer forwarded to the callbacks, and is not otherwise used by the
// function. Returns a nonzero value on success.

int ttDecomposeOutline(const TTPoint* points,
                       size_t numPoints,
                       const TTDecomposeFuncs* funcs,
                       void* userPtr);


// Helper type bundling user callback functions used by the decomposition
// interface. Each callback takes a `userPtr` parameter, which will be the
// corresponding value passed by the user to `ttDecomposeOutline`. A callback
// should return a nonzero value to signal success. If a callback returns zero,
// decomposition stops immediately. The callback functions are:
//
//   - `moveTo`, which is called whenever a new contour begins (including the
//     first one). (`x`, `y`) is the starting point of the contour.
//   - `lineTo`, which is called for a straight line segment from the previous
//     point in the contour to the endpoint (`x`, `y`).
//   - `curveTo`, which is called for a quadratic Bézier curve from the previous
//     point in the contour to the endpoint (`x`, `y`) via the control point
//     (`cx`, `cy`).

struct TTDecomposeFuncs
{
    int (*moveTo)(void* userPtr, int x, int y);
    int (*lineTo)(void* userPtr, int x, int y);
    int (*curveTo)(void* userPtr, int cx, int cy, int x, int y);
};


////////////////////////////////////////////////////////////////////////////////
//
//                        END OF PUBLIC INTERFACE
//
//             Everything below this line is implementation
//             details, and is only expanded if the including
//             source file defines TT_IMPLEMENTATION.
//
////////////////////////////////////////////////////////////////////////////////

#ifdef TT_IMPLEMENTATION

#include <math.h> // For sqrtf() and roundf().


// 32-bit integer values of four-character table tags.

#define TT_TAG(a, b, c, d) ((a << 24u) | (b << 16u) | (c << 8u) | d)

enum {
    TT_TAG_CMAP = TT_TAG('c', 'm', 'a', 'p'),
    TT_TAG_GLYF = TT_TAG('g', 'l', 'y', 'f'),
    TT_TAG_HEAD = TT_TAG('h', 'e', 'a', 'd'),
    TT_TAG_HHEA = TT_TAG('h', 'h', 'e', 'a'),
    TT_TAG_HMTX = TT_TAG('h', 'm', 't', 'x'),
    TT_TAG_KERN = TT_TAG('k', 'e', 'r', 'n'),
    TT_TAG_LOCA = TT_TAG('l', 'o', 'c', 'a'),
    TT_TAG_MAXP = TT_TAG('m', 'a', 'x', 'p')
};


// Range functions that are easy to define in terms of other functions.

#define ttSkipShorts(range, count) ttSkipBytes((range), (count) * 2)
#define ttSkipInts(range, count) ttSkipBytes((range), (count) * 4)

#define ttSkipByte(range) ttSkipBytes((range), 1)
#define ttSkipShort(range) ttSkipShorts((range), 1)
#define ttSkipInt(range) ttSkipInts((range), 1)

#define ttReadByte(range) ((int8_t) ttReadUbyte(range))
#define ttReadShort(range) ((int16_t) ttReadUshort(range))
#define ttReadInt(range) ((int32_t) ttReadUint(range))

#define ttGetByte(range, index) ((int8_t) ttGetUbyte((range), (index)))
#define ttGetShort(range, index) ((int16_t) ttGetUshort((range), (index)))
#define ttGetInt(range, index) ((int32_t) ttGetUint((range), (index)))


// Helper min/max macros.

#define ttMin(x, y) ((x) < (y) ? (x) : (y))
#define ttMax(x, y) ((x) > (y) ? (x) : (y))


// Get a subrange of a given length in bytes, starting at a given byte offset in
// another range.

static TTRange ttSubrange(const TTRange* range, size_t offset, size_t length)
{
    ttAssert(offset <= range->size && length <= range->size - offset);
    offset = ttMin(offset, range->size);
    length = ttMin(range->size - offset, length);
    TTRange result = {range->data + offset, length};
    return result;
}


// Get the Nth 8-bit value from the beginning of a range, treating it like an
// array of that type.

static uint8_t ttGetUbyte(const TTRange* range, size_t index)
{
    ttAssert(index < range->size);
    return index < range->size ? range->data[index] : 0;
}


// Get the Nth 16-bit value from the beginning of a range, treating it like an
// array of that type.

static uint16_t ttGetUshort(const TTRange* range, size_t index)
{
    ttAssert(index * 2 + 2 <= range->size);
    uint16_t value = 0;
    size_t offset = index * 2;
    if (offset + 2 <= range->size) {
        value |= range->data[offset + 0] << 8u;
        value |= range->data[offset + 1] << 0u;
    }
    return value;
}


// Get the Nth 32-bit value from the beginning of a range, treating it like an
// array of that type.

static uint32_t ttGetUint(const TTRange* range, size_t index)
{
    ttAssert(index * 4 + 4 <= range->size);
    uint32_t value = 0;
    size_t offset = index * 4;
    if (offset + 4 <= range->size) {
        value |= range->data[offset + 0] << 24u;
        value |= range->data[offset + 1] << 16u;
        value |= range->data[offset + 2] <<  8u;
        value |= range->data[offset + 3] <<  0u;
    }
    return value;
}


// Skip ahead by a given number of bytes in a range.

static void ttSkipBytes(TTRange* range, size_t bytes)
{
    ttAssert(bytes <= range->size);
    bytes = ttMin(bytes, range->size);
    range->data += bytes;
    range->size -= bytes;
}


// Read an 8-bit value from the beginning of a range, also advancing the range
// by that much.

static uint8_t ttReadUbyte(TTRange* range)
{
    uint8_t value = ttGetUbyte(range, 0);
    ttSkipByte(range);
    return value;
}


// Read a 16-bit value from the beginning of a range, also advancing the range
// by that much.

static uint16_t ttReadUshort(TTRange* range)
{
    uint16_t value = ttGetUshort(range, 0);
    ttSkipShort(range);
    return value;
}


// Read a 32-bit value from the beginning of a range, also advancing the range
// by that much.

static uint32_t ttReadUint(TTRange* range)
{
    uint32_t value = ttGetUint(range, 0);
    ttSkipInt(range);
    return value;
}


// Read an array of 16-bit values from the beginning of a range, returning it as
// a subrange. Also advances the source range by that amount.

static TTRange ttReadShorts(TTRange* range, size_t arrayLength)
{
    TTRange array = ttSubrange(range, 0, arrayLength * sizeof(uint16_t));
    ttSkipBytes(range, array.size);
    return array;
}


// Initialize a font context.

int ttInitFont(TTFont* font, const void* fileData, size_t fileSize)
{
    if (!font || !fileData || !fileSize) {
        return 0;
    }

    // Initialize data members.
    static const TTRange nullRange = {0, 0};
    font->cmap = nullRange;
    font->glyf = nullRange;
    font->head = nullRange;
    font->hhea = nullRange;
    font->hmtx = nullRange;
    font->kern = nullRange;
    font->loca = nullRange;
    font->maxp = nullRange;

    // Read the directory header.
    TTRange file = {(const uint8_t*) fileData, fileSize};
    TTRange dir = file;
    if (ttReadUint(&dir) != 0x00010000) {
        return 0;
    }
    uint16_t numTables = ttReadUshort(&dir);
    ttSkipShorts(&dir, 3); // Skip searchRange, entrySelector, rangeShift.
    for (uint16_t i = 0; i < numTables; i++) {

        // Get the location of the table in the file.
        uint32_t tag = ttReadUint(&dir);
        uint32_t checkSum = ttReadUint(&dir);
        uint32_t offset = ttReadUint(&dir);
        uint32_t length = ttReadUint(&dir);
        TTRange table = ttSubrange(&file, offset, (length + 3) & ~3);

        // Verify the table's checksum.
        uint32_t sum = 0;
        uint32_t numLongs = (length + 3) / 4;
        for (uint32_t i = 0; i < numLongs; i++) {
            sum += ttGetUint(&table, i);
        }

        // The "head" table's "checkSumAdjustment" field does not participate
        // in the check sum. We can correct for this by subtracting its value
        // from the sum.
        if (tag == TT_TAG_HEAD) {
            sum -= ttGetUint(&table, 2);
        }

        // Set the table to an invalid value if the checksum didn't match.
        if (sum != checkSum) {
            table = nullRange;
        }

        // Store a handle to the table if we need it.
        switch (tag) {
            case TT_TAG_CMAP: font->cmap = table; break;
            case TT_TAG_GLYF: font->glyf = table; break;
            case TT_TAG_HEAD: font->head = table; break;
            case TT_TAG_HHEA: font->hhea = table; break;
            case TT_TAG_HMTX: font->hmtx = table; break;
            case TT_TAG_KERN: font->kern = table; break;
            case TT_TAG_LOCA: font->loca = table; break;
            case TT_TAG_MAXP: font->maxp = table; break;
            default: break;
        }
    }

    // Return a nonzero value if all required tables were found. Note that the
    // "kern" table is purposely omitted here because it's optional.
    return font->cmap.data
        && font->glyf.data
        && font->head.data
        && font->hhea.data
        && font->hmtx.data
        && font->loca.data
        && font->maxp.data;
}


// Get the glyph count for a font.

int ttGetGlyphCount(const TTFont* font)
{
    return font ? ttGetUshort(&font->maxp, 2) : 0;
}


// Get the bounding box for a font.

int ttGetFontBox(const TTFont* font, int* xMin, int* yMin, int* xMax, int* yMax)
{
    if (font && font->head.size >= 44) {
        TTRange boxData = ttSubrange(&font->head, 36, 8);
        if (xMin) *xMin = ttGetShort(&boxData, 0);
        if (yMin) *yMin = ttGetShort(&boxData, 1);
        if (xMax) *xMax = ttGetShort(&boxData, 2);
        if (yMax) *yMax = ttGetShort(&boxData, 3);
        return 1;
    }
    return 0;
}


// Get the vertical metrics for a font.

int ttGetFontMetrics(const TTFont* font, int* ascent, int* descent,
                     int* lineGap)
{
    if (font) {
        if (ascent)  *ascent  = ttGetShort(&font->hhea, 2);
        if (descent) *descent = ttGetShort(&font->hhea, 3);
        if (lineGap) *lineGap = ttGetShort(&font->hhea, 4);
        return 1;
    }
    return 0;
}


// Get a pixel height-based scale factor for a font.

float ttGetPixelScale(const TTFont* font, float pixelHeight)
{
    int ascent, descent;
    if (ttGetFontMetrics(font, &ascent, &descent, 0)) {
        int unscaledHeight = ascent - descent;
        if (unscaledHeight != 0) {
            return pixelHeight / unscaledHeight;
        }
    }
    return 0.0f;
}


// Get a point-based scale factor for a font.

float ttGetPointScale(const TTFont* font, float points, float dpi)
{
    if (font) {
        uint16_t unitsPerEm = ttGetUshort(&font->head, 9);
        if (unitsPerEm != 0) {
            float pixelSize = points * dpi / 72.0f;
            return pixelSize / unitsPerEm;
        }
    }
    return 0.0f;
}


// Get a glyph ID from a format 0 mapping subtable.

static int ttGetGlyphID0(TTRange map, uint32_t codePoint)
{
    ttSkipShort(&map); // Skip over format.
    uint16_t length = ttReadUshort(&map);
    ttSkipShort(&map); // Skip over language.
    return codePoint < length ? ttGetUbyte(&map, codePoint) : 0;
}


// Get a glyph ID from a format 4 mapping subtable.

static int ttGetGlyphID4(TTRange map, uint32_t codePoint)
{
    // This format only stores code points up to U+FFFF.
    if (codePoint > 0xffff) {
        return 0;
    }

    // Read the segment count and associated arrays.
    ttSkipShorts(&map, 3); // Skip over format, length, language.
    uint16_t segCount = ttReadUshort(&map) / 2;
    ttSkipShorts(&map, 3); // Skip over searchRange, entrySelector, rangeShift.
    TTRange endCode = ttReadShorts(&map, segCount);
    ttSkipShort(&map); // Skip over unused padding.
    TTRange startCode = ttReadShorts(&map, segCount);
    TTRange idDelta = ttReadShorts(&map, segCount);
    TTRange idRangeOffset = map;

    // Bisect the segment array to find the one the code point is in.
    int segment = -1;
    for (int low = 0, high = segCount - 1; low < high;) {
        int mid = (low + high) >> 1;
        if (ttGetUshort(&startCode, mid) > codePoint) {
            high = mid;
        } else if (ttGetUshort(&endCode, mid) < codePoint) {
            low = mid + 1;
        } else {
            segment = mid;
            break;
        }
    }

    // Translate the located glyph ID.
    if (segment != -1) {
        uint16_t start = ttGetUshort(&startCode, segment);
        uint16_t end = ttGetUshort(&endCode, segment);
        if (start <= codePoint && codePoint <= end) {
            uint16_t offset = ttGetUshort(&idRangeOffset, segment);
            uint16_t rangeIndex = segment + offset / 2 + (codePoint - start);
            uint16_t index = offset == 0
                ? codePoint
                : ttGetUshort(&idRangeOffset, rangeIndex);
            return (index + ttGetUshort(&idDelta, segment)) & 0xffff;
        }
    }
    return 0; // Glyph not found.
}


// Get a glyph ID from a format 6 mapping subtable.

static int ttGetGlyphID6(TTRange map, uint32_t codePoint)
{
    if (codePoint <= 0x1ffff) {
        ttSkipShorts(&map, 3); // Skip format, length, language.
        uint16_t firstCode = ttReadUshort(&map);
        uint16_t entryCount = ttReadUshort(&map);
        TTRange glyphIndexArray = ttReadShorts(&map, entryCount);
        if (firstCode <= codePoint && codePoint < firstCode + entryCount) {
            return ttGetUshort(&glyphIndexArray, codePoint - firstCode);
        }
    }
    return 0; // Glyph not found.
}


// Get a glyph ID from a format 12 or 13 mapping subtable.

static int ttGetGlyphID12Or13(TTRange map, uint32_t codePoint)
{
    // Read the subtable header.
    uint16_t format = ttReadUshort(&map);
    ttSkipShort(&map); // Skip reserved field.
    ttSkipInts(&map, 2); // Skip length, language.
    uint32_t nGroups = ttReadUint(&map);

    // Bisect to find the right group.
    uint32_t low = 0, high = nGroups;
    while (low < high) {

        // Get the group at the midpoint of the search range.
        uint32_t mid = low + ((high - low) >> 1);
        const uint32_t groupSize = sizeof(uint32_t[3]);
        TTRange group = ttSubrange(&map, mid * groupSize, groupSize);
        uint32_t startCharCode = ttGetUint(&group, 0);
        uint32_t endCharCode = ttGetUint(&group, 1);
        uint32_t startGlyphCode = ttGetUint(&group, 2);

        // Recurse unless the correct group was found.
        if (codePoint < startCharCode) {
            high = mid;
        } else if (codePoint > endCharCode) {
            low = mid + 1;
        } else {
            uint32_t offset = format == 12 ? codePoint - startCharCode : 0;
            return startGlyphCode + offset;
        }
    }
    return 0; // Not found.
}


// Get a glyph ID from a Unicode code point.

int ttGetGlyphID(const TTFont* font, int codePoint)
{
    if (!font || codePoint < 0) {
        return 0;
    }

    // Get the mapping subtables.
    TTRange encodings = font->cmap;
    ttSkipShort(&encodings); // Skip over version.
    uint16_t numEncodings = ttReadUshort(&encodings);

    // Search the mapping subtables until a valid glyph ID is found.
    int glyphID = 0;
    for (uint16_t i = 0; i < numEncodings && glyphID == 0; i++) {
        uint16_t type = ttReadUshort(&encodings);
        uint16_t subtype = ttReadUshort(&encodings);
        uint32_t offset = ttReadUint(&encodings);

        // Check for Unicode encodings and Windows encodings of the UCS-2 and
        // UCS-4 subtypes.
        if (type == 0 || (type == 3 && (subtype == 1 || subtype == 10))) {
            TTRange map = font->cmap;
            ttSkipBytes(&map, offset);
            switch (ttGetUshort(&map, 0)) {
                case 0: glyphID = ttGetGlyphID0(map, codePoint); break;
                case 4: glyphID = ttGetGlyphID4(map, codePoint); break;
                case 6: glyphID = ttGetGlyphID6(map, codePoint); break;
                case 12: glyphID = ttGetGlyphID12Or13(map, codePoint); break;
                case 13: glyphID = ttGetGlyphID12Or13(map, codePoint); break;
                default: break;
            }
        }
    }
    return glyphID;
}


// Helper type for bundling state used during contour extraction.

typedef struct {
    TTRange* flagsData; // "glyf" table flags array.
    TTRange* coordData; // "glyf" table coordinate data.
    int axis;           // Current axis (x = 0, y = 1).
    int16_t coord;      // Current coordinate value.
    uint8_t flags;      // Current flags value.
    uint16_t repeat;    // Repeat counter for flags.
} TTCoords;


// Get the points for a single glyph contour.

static void ttGetContour(TTCoords* c, TTPoint* points, size_t arrayLength,
                         size_t contourLength)
{
    for (size_t i = 0; i < contourLength; i++) {

        // Get the flags for the next point, handling repeats.
        if ((c->repeat -= c->repeat > 0) == 0) {
            c->flags = ttReadUbyte(c->flagsData);
            c->repeat = c->flags & 0x08 ? ttReadUbyte(c->flagsData) + 1 : 0;
        }

        // Get the coordinate value of the next point.
        if (c->flags & (0x02 << c->axis)) {
            int16_t value = ttReadUbyte(c->coordData);
            c->coord += (c->flags & (0x10 << c->axis)) ? value : -value;
        } else if (!(c->flags & (0x10 << c->axis))) {
            c->coord += ttReadShort(c->coordData);
        }

        // Write point data (unless we're at the end of the array).
        if (i < arrayLength) {
            points[i].flags = c->flags & 0x01 ? TT_POINT_ON_CURVE : 0;
            *(c->axis ? &points[i].y : &points[i].x) = c->coord;
        }
    }

    // Set first/last flags for the contour.
    if (arrayLength > 0) {
        points[0].flags |= TT_POINT_FIRST;
        if (contourLength <= arrayLength) {
            points[contourLength - 1].flags |= TT_POINT_LAST;
        }
    }
}


// Check if a glyph ID is within a font's valid range of IDs.

static int ttIsValidGlyphID(const TTFont* font, int glyphID)
{
    return 0 <= glyphID && glyphID < ttGetGlyphCount(font);
}


// Get the raw left side bearing value for a glyph.

static int ttGetGlyphLeftSideBearing(const TTFont* font, int glyphID)
{
    ttAssert(ttIsValidGlyphID(font, glyphID));
    int numLongMetrics = ttGetUshort(&font->hhea, 17);
    int index = glyphID >= numLongMetrics
        ? numLongMetrics * 2 + (glyphID - numLongMetrics)
        : glyphID * 2 + 1;
    return ttGetShort(&font->hmtx, index);
}


// Locate the subsection of the "glyf" table that contains the glyph data for a
// given glyph ID.

static TTRange ttGetGlyphData(const TTFont* font, int glyphID)
{
    // Long-form location table with 32-bit offsets.
    if (ttGetUshort(&font->head, 25) != 0) {
        uint32_t offset = ttGetUint(&font->loca, glyphID);
        uint32_t length = ttGetUint(&font->loca, glyphID + 1) - offset;
        return ttSubrange(&font->glyf, offset, length);

    // Short-form location table with 16-bit offsets (stored divided by two).
    } else {
        uint32_t offset = 2 * ttGetUshort(&font->loca, glyphID);
        uint32_t length = 2 * ttGetUshort(&font->loca, glyphID + 1) - offset;
        return ttSubrange(&font->glyf, offset, length);
    }
}


// Get a glyph's outline. Recursive function with a depth limit to prevent
// infinite recursion.

static size_t ttGetOutline(const TTFont* font, int glyphID, TTPoint* points,
                           size_t maxPoints, int depth)
{
    ttAssert(ttIsValidGlyphID(font, glyphID));

    // Prevent infinite recursion.
    if (depth > 16) {
        return 0;
    }

    // Get the glyph data; quit right away if it's too short to contain a glyph.
    TTRange glyf = ttGetGlyphData(font, glyphID);
    if (glyf.size < 10) {
        return 0;
    }

    // Get the contour count. The sign of this value also determines if it's a
    // simple or compound glyph.
    int16_t numContours = ttReadShort(&glyf);

    // Simple glyphs are translated horizontally so that xMin and the left side
    // bearing match. Since point coordinates are relative, this can be done for
    // free by just choosing a different origin.
    int16_t xOrigin = 0;
    if (numContours >= 0) {
        int16_t lsb = ttGetGlyphLeftSideBearing(font, glyphID);
        int16_t xMin = ttGetShort(&glyf, 0);
        xOrigin = lsb - xMin;
    }
    ttSkipShorts(&glyf, 4); // Skip over the bounding box.

    // If the contour count is positive, it's a simple glyph.
    if (numContours > 0) {

        // Read the endpoint array. The last endpoint index tells us how many
        // points there are in the whole outline.
        TTRange endpoints = ttReadShorts(&glyf, numContours);
        size_t numPoints = ttGetUshort(&endpoints, numContours - 1) + 1;

        // If `points` is null, we just return the total number of points.
        if (!points) {
            return numPoints;
        }

        // Skip the instruction array.
        uint16_t numInstructions = ttReadUshort(&glyf);
        ttSkipBytes(&glyf, numInstructions);

        // Scan through the flags array to find the coordinate data.
        TTRange flags = glyf;
        for (size_t i = 0; i < numPoints; i++) {
            i += (ttReadUbyte(&glyf) & 0x08) ? ttReadUbyte(&glyf) : 0;
        }

        // Read the coordinates for the x-axis and then for the y-axis.
        for (int axis = 0; axis < 2; axis++) {
            int16_t origin = axis == 0 ? xOrigin : 0;
            TTRange axisFlags = flags;
            TTPoint* axisPoints = points;
            TTCoords coords = {&axisFlags, &glyf, axis, origin, 0, 0};
            size_t arrayLength = ttMin(numPoints, maxPoints);
            for (int i = 0, start = 0; i < numContours; i++) {
                size_t contourLength = ttGetUshort(&endpoints, i) - start + 1;
                size_t stride = ttMin(contourLength, arrayLength);
                ttGetContour(&coords, axisPoints, stride, contourLength);
                arrayLength -= stride;
                axisPoints += stride;
                start += contourLength;
            }
        }
        return ttMin(numPoints, maxPoints);

    // If the contour count is negative, it's a compound glyph.
    } else if (numContours < 0) {

        // Read each component glyph.
        size_t numPoints = 0;
        uint16_t flags;
        TTPoint* basePoints = points;
        do {

            // Get the component glyph's flags and points.
            flags = ttReadUshort(&glyf);
            uint16_t componentID = ttReadUshort(&glyf);
            size_t componentPoints = ttGetOutline(font, componentID, points,
                                                  maxPoints, depth + 1);

            // 2x3 transformation matrix for the component glyph.
            float a = 1.0f, c = 0.0f, e = 0.0f;
            float b = 0.0f, d = 1.0f, f = 0.0f;

            // Read the translation part of the matrix (if applicable).
            if (flags & 0x0002) {
                e = flags & 1 ? ttReadShort(&glyf) : ttReadByte(&glyf);
                f = flags & 1 ? ttReadShort(&glyf) : ttReadByte(&glyf);

            // Otherwise, translate by matching two points.
            } else {
                size_t i = flags & 1 ? ttReadUshort(&glyf) : ttReadUbyte(&glyf);
                size_t j = flags & 1 ? ttReadUshort(&glyf) : ttReadUbyte(&glyf);
                if (points && i < numPoints && j < componentPoints) {
                    e = basePoints[i].x - points[j].x;
                    f = basePoints[i].y - points[j].y;
                }
            }

            // Read the scale/rotation/shear portion of the matrix.
            const float scale = 1.0 / 16384;
            if (flags & 0x08) { // Uniform scale.
                a = d = ttReadShort(&glyf) * scale;
            } else if (flags & 0x40) { // Separate x and y scale.
                a = ttReadShort(&glyf) * scale;
                d = ttReadShort(&glyf) * scale;
            } else if (flags & 0x80) { // Full 2x2 transformation matrix.
                a = ttReadShort(&glyf) * scale;
                b = ttReadShort(&glyf) * scale;
                c = ttReadShort(&glyf) * scale;
                d = ttReadShort(&glyf) * scale;
            }

            // Handle the SCALED_COMPONENT_OFFSET bit.
            if (flags & 0x0800) {
                e *= sqrtf(a * a + b * b);
                f *= sqrtf(c * c + d * d);
            }

            // If we're writing output, transform the component glyph.
            if (points) {
                for (size_t i = 0; i < componentPoints; i++) {
                    points[i].x = roundf(a * points[i].x + c * points[i].x + e);
                    points[i].y = roundf(b * points[i].y + d * points[i].y + f);
                }

                // Advance the position in the output array.
                maxPoints -= componentPoints;
                points += componentPoints;
            }
            numPoints += componentPoints;

        // Continue until we have all components, or the output array runs out.
        } while ((flags & 0x0020) && (maxPoints > 0 || !points));
        return numPoints;
    }
    return 0;
}


// Get a glyph's outline data.

size_t ttGetGlyphOutline(const TTFont* font, int glyphID, TTPoint* points,
                         size_t maxPoints)
{
    if (font && ttIsValidGlyphID(font, glyphID)) {
        return ttGetOutline(font, glyphID, points, maxPoints, 0);
    }
    return 0;
}


// Calculate the bounding box for a glyph outline.

int ttGetGlyphBox(const TTPoint* points, size_t numPoints,
                  int* xMin, int* yMin, int* xMax, int* yMax)
{
    if (points) {
        int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
        if (numPoints > 0) {
            x0 = y0 = +32767;
            x1 = y1 = -32768;
            for (size_t i = 0; i < numPoints; i++) {
                x0 = ttMin(x0, points[i].x);
                y0 = ttMin(y0, points[i].y);
                x1 = ttMax(x1, points[i].x);
                y1 = ttMax(y1, points[i].y);
            }
        }
        if (xMin) *xMin = x0;
        if (yMin) *yMin = y0;
        if (xMax) *xMax = x1;
        if (yMax) *yMax = y1;
        return 1;
    }
    return 0;
}


// Get the advance metric for a single glyph.

int ttGetGlyphAdvance(const TTFont* font, int glyphID)
{
    if (font && ttIsValidGlyphID(font, glyphID)) {
        int numLongMetrics = ttGetUshort(&font->hhea, 17);
        TTRange metrics = ttSubrange(&font->hmtx, 0, 4 * numLongMetrics);
        return ttGetUshort(&metrics, 2 * ttMin(glyphID, numLongMetrics - 1));
    }
    return 0;
}


// Get the kerning advance for a pair of glyphs.

int ttGetGlyphKerning(const TTFont* font, int leftGlyphID, int rightGlyphID)
{
    // Check that arguments look good and that the font has a "kern" table.
    if (font && font->kern.data
     && ttIsValidGlyphID(font, leftGlyphID)
     && ttIsValidGlyphID(font, rightGlyphID)) {

        // Check that the "kern" table version number is zero.
        TTRange kern = font->kern;
        if (ttReadUshort(&kern) == 0) {

            // Iterate over "kern" subtables.
            uint16_t numSubtables = ttReadUshort(&kern);
            for (uint16_t i = 0; i < numSubtables; i++) {
                uint16_t length = ttGetUshort(&kern, 1);
                uint16_t coverage = ttGetUshort(&kern, 2);
                TTRange subtable = ttSubrange(&kern, 6, kern.size - 6);
                ttSkipBytes(&kern, length); // Skip to the next subtable.

                // Check for a format 0 subtable with horizontal kerning data.
                if ((coverage >> 8) == 0 && (coverage & 3) == 1) {
                    uint32_t needlePair = (leftGlyphID << 16) | rightGlyphID;
                    uint16_t subtablePairs = ttReadUshort(&subtable);
                    ttSkipShorts(&subtable, 3); // Skip bisection fields.
                    subtablePairs = ttMin(subtablePairs, subtable.size / 6);

                    // Bisect to find the correct entry.
                    for (int low = 0, high = subtablePairs - 1; low <= high;) {
                        int mid = (low + high) >> 1;
                        uint16_t pair1 = ttGetUshort(&subtable, mid * 3 + 0);
                        uint16_t pair2 = ttGetUshort(&subtable, mid * 3 + 1);
                        uint32_t haystackPair = (pair1 << 16) | pair2;
                        if (needlePair < haystackPair) {
                            high = mid - 1;
                        } else if (needlePair > haystackPair) {
                            low = mid + 1;
                        } else {
                            return ttGetShort(&subtable, mid * 3 + 2);
                        }
                    }
                }
            }
        }
    }
    return 0;
}


// Handle a decomposition event, where one of the callbacks may be called.

static int ttDecomposeEvent(const TTDecomposeFuncs* f, void* userPtr,
                            TTPoint* prevOn, TTPoint* prev, TTPoint* curr)
{
    int result = 1;

    // Consecutive on-curve points make line segments.
    if (curr->flags & TT_POINT_ON_CURVE) {
        if (prev->flags & TT_POINT_ON_CURVE) {
            result = f->lineTo(userPtr, curr->x, curr->y);

        // On-curve points following off-curve points make curves.
        } else {
            result = f->curveTo(userPtr, prev->x, prev->y, curr->x, curr->y);
        }

    // Consecutive off-curve points make curves with implicit starting points.
    } else if (!(prev->flags & TT_POINT_ON_CURVE)) {
        int x = (prev->x + curr->x) >> 1;
        int y = (prev->y + curr->y) >> 1;
        result = f->curveTo(userPtr, prev->x, prev->y, x, y);
        prevOn->x = x;
        prevOn->y = y;
    }
    return result;
}


// Decompose a glyph outline into a series of Bézier curves.

int ttDecomposeOutline(const TTPoint* points, size_t numPoints,
                       const TTDecomposeFuncs* funcs, void* userPtr)
{
    // Check that the arguments look good.
    if (!points || !funcs) {
        return 0;

    // Check that we got all callback functions.
    } else if (!funcs->moveTo || !funcs->lineTo || !funcs->curveTo) {
        return 0;
    }

    // Process each point in the outline.
    TTPoint prev, prevOn; // The previous point and previous on-curve point.
    TTPoint init, initOn; // The initial point and initial on-curve point.
    for (size_t i = 0; i < numPoints; i++) {
        TTPoint curr = points[i];

        // Handle the initial point in a contour.
        if (curr.flags & TT_POINT_FIRST) {
            prev = prevOn = init = initOn = curr;

            // If we started on an off-curve point, we treat the next point
            // as the starting point instead.
            if (!(curr.flags & TT_POINT_ON_CURVE)) {
                if (++i < numPoints) {
                    initOn = curr = points[i];

                    // If the next point is also an off-curve point, we must use
                    // an implicit on-curve point.
                    if (!(curr.flags & TT_POINT_ON_CURVE)) {
                        initOn.x = prevOn.x = (init.x + curr.x) >> 1;
                        initOn.y = prevOn.y = (init.y + curr.y) >> 1;
                    }
                }
            }

            // The initial point generates a move event.
            if (!funcs->moveTo(userPtr, initOn.x, initOn.y)) {
                return 0;
            }

        // Handle all subsequent points.
        } else if (!ttDecomposeEvent(funcs, userPtr, &prevOn, &prev, &curr)) {
            return 0;
        }

        // Close the contour when the last point is reached.
        if (curr.flags & TT_POINT_LAST) {
            if (!ttDecomposeEvent(funcs, userPtr, &prevOn, &curr, &init)) {
                return 0;
            }

            // If the contour started with an off-curve point, an additional
            // segment is needed to close the contour.
            if (!(init.flags & TT_POINT_ON_CURVE)
             && !ttDecomposeEvent(funcs, userPtr, &curr, &init, &initOn)) {
                return 0;
            }
        }

        // Keep track of the previous point.
        if (curr.flags & TT_POINT_ON_CURVE) {
            prevOn = curr;
        }
        prev = curr;
    }
    return 1;
}


#endif // #ifdef TT_IMPLEMENTATION

#ifdef __cplusplus
}
#endif

#endif // #ifndef TRUETYPE_H_INCLUDED
