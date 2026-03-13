#include "frost/graphics/font.hpp"
#include "frost/graphics/bitmap_font.hpp"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <cctype>

#ifdef FROST_HAS_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

namespace frost {

namespace {

template<typename T>
bool read_value(std::ifstream& file, T& value) {
    file.read(reinterpret_cast<char*>(&value), static_cast<std::streamsize>(sizeof(T)));
    return static_cast<bool>(file);
}

u32 read_u32_le(const u8* ptr) {
    return static_cast<u32>(ptr[0])
        | (static_cast<u32>(ptr[1]) << 8)
        | (static_cast<u32>(ptr[2]) << 16)
        | (static_cast<u32>(ptr[3]) << 24);
}

} // namespace

Font Font::make_default() {
    Font font;
    font.glyph_width_ = BitmapFont::GLYPH_WIDTH;
    font.glyph_height_ = BitmapFont::GLYPH_HEIGHT;
    font.first_char_ = BitmapFont::FIRST_CHAR;
    font.last_char_ = BitmapFont::LAST_CHAR;
    font.glyph_count_ = BitmapFont::CHAR_COUNT;
    font.bytes_per_glyph_ = BitmapFont::GLYPH_HEIGHT;
    font.glyph_data_.assign(
        BitmapFont::FONT_DATA,
        BitmapFont::FONT_DATA + (BitmapFont::CHAR_COUNT * BitmapFont::GLYPH_HEIGHT)
    );
    return font;
}

Result<Font> Font::load_from_file(StringView file_path, i32 pixel_height) {
    auto is_ttf_or_otf = [](StringView path) {
        auto dot = path.find_last_of('.');
        if (dot == StringView::npos) {
            return false;
        }

        String ext(path.substr(dot + 1));
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        return ext == "ttf" || ext == "otf";
    };

    if (is_ttf_or_otf(file_path)) {
#ifdef FROST_HAS_FREETYPE
        FT_Library library = nullptr;
        if (FT_Init_FreeType(&library) != 0) {
            return Error{ErrorCode::FileReadError, "Failed to initialize FreeType"};
        }

        FT_Face face = nullptr;
        if (FT_New_Face(library, String(file_path).c_str(), 0, &face) != 0) {
            FT_Done_FreeType(library);
            return Error{ErrorCode::FileReadError, "Failed to load TTF/OTF font"};
        }

        const i32 target_pixel_height = std::max(1, pixel_height);
        if (FT_Set_Pixel_Sizes(face, 0, static_cast<u32>(target_pixel_height)) != 0) {
            FT_Done_Face(face);
            FT_Done_FreeType(library);
            return Error{ErrorCode::FileReadError, "Failed to configure TTF/OTF font size"};
        }

        // Derive cell dimensions from the face metrics (in 26.6 fractional pixels).
        const i32 ascender_px  = static_cast<i32>(face->size->metrics.ascender  >> 6);
        const i32 descender_px = static_cast<i32>(-(face->size->metrics.descender >> 6));
        const i32 cell_h = std::max(ascender_px + descender_px, target_pixel_height);

        // Maximum advance width across printable ASCII – used as the cell width so
        // the existing fixed-advance renderer path still works correctly.
        i32 max_advance = 1;
        for (i32 code = 32; code <= 126; ++code) {
            if (FT_Load_Char(face, static_cast<FT_ULong>(code), FT_LOAD_DEFAULT) != 0) {
                continue;
            }
            max_advance = std::max(max_advance, static_cast<i32>(face->glyph->advance.x >> 6));
        }

        Font font;
        font.first_char_  = 32;
        font.last_char_   = 126;
        font.glyph_count_ = font.last_char_ - font.first_char_ + 1;
        font.glyph_width_  = max_advance;
        font.glyph_height_ = cell_h;
        font.ascender_     = ascender_px;
        // bytes_per_glyph_ is not used for alpha-data fonts but keep it consistent.
        font.bytes_per_glyph_ = cell_h;

        // 8-bit alpha cell buffer: glyph_width * cell_h bytes per glyph.
        const usize cell_pixels = static_cast<usize>(max_advance) * static_cast<usize>(cell_h);
        font.glyph_alpha_data_.assign(static_cast<usize>(font.glyph_count_) * cell_pixels, 0);
        font.glyph_advances_x_.assign(static_cast<usize>(font.glyph_count_), max_advance);

        for (i32 code = font.first_char_; code <= font.last_char_; ++code) {
            if (FT_Load_Char(face, static_cast<FT_ULong>(code), FT_LOAD_RENDER) != 0) {
                continue;
            }

            const i32 glyph_index = code - font.first_char_;
            font.glyph_advances_x_[static_cast<usize>(glyph_index)] =
                static_cast<i32>(face->glyph->advance.x >> 6);

            const i32 src_w   = static_cast<i32>(face->glyph->bitmap.width);
            const i32 src_h   = static_cast<i32>(face->glyph->bitmap.rows);
            const i32 src_pitch = std::abs(static_cast<i32>(face->glyph->bitmap.pitch));

            // Place the bitmap using the glyph's bearing so all characters
            // share the same baseline (ascender_px rows from the top of the cell).
            const i32 dst_x = static_cast<i32>(face->glyph->bitmap_left);
            const i32 dst_y = ascender_px - static_cast<i32>(face->glyph->bitmap_top);

            const usize glyph_offset = static_cast<usize>(glyph_index) * cell_pixels;

            for (i32 y = 0; y < src_h; ++y) {
                for (i32 x = 0; x < src_w; ++x) {
                    const u8 alpha = face->glyph->bitmap.buffer[y * src_pitch + x];
                    if (alpha == 0) {
                        continue;
                    }

                    const i32 px = dst_x + x;
                    const i32 py = dst_y + y;
                    if (px < 0 || px >= max_advance || py < 0 || py >= cell_h) {
                        continue;
                    }

                    font.glyph_alpha_data_[
                        glyph_offset +
                        static_cast<usize>(py) * static_cast<usize>(max_advance) +
                        static_cast<usize>(px)
                    ] = alpha;
                }
            }
        }

        FT_Done_Face(face);
        FT_Done_FreeType(library);

        return font;
#else
        return Error{ErrorCode::NotSupported, "TTF/OTF loading requires FreeType (build with FROST_USE_FREETYPE=ON)"};
#endif
    }

    std::ifstream file(String(file_path), std::ios::binary);
    if (!file) {
        return Error{ErrorCode::FileNotFound, "Failed to open font file"};
    }

    u8 magic[4]{};
    file.read(reinterpret_cast<char*>(magic), 4);
    if (!file) {
        return Error{ErrorCode::FileReadError, "Failed to read font header"};
    }

    file.seekg(0, std::ios::beg);

    Font font;

    // PSF1
    if (magic[0] == 0x36 && magic[1] == 0x04) {
        u8 psf1_magic0 = 0;
        u8 psf1_magic1 = 0;
        u8 mode = 0;
        u8 char_size = 0;

        if (!read_value(file, psf1_magic0)
            || !read_value(file, psf1_magic1)
            || !read_value(file, mode)
            || !read_value(file, char_size)) {
            return Error{ErrorCode::FileReadError, "Invalid PSF1 header"};
        }

        const i32 glyph_count = (mode & 0x01) ? 512 : 256;
        if (char_size <= 0) {
            return Error{ErrorCode::FileReadError, "Invalid PSF1 character size"};
        }

        font.glyph_width_ = 8;
        font.glyph_height_ = static_cast<i32>(char_size);
        font.first_char_ = 0;
        font.last_char_ = glyph_count - 1;
        font.glyph_count_ = glyph_count;
        font.bytes_per_glyph_ = static_cast<i32>(char_size);
        font.glyph_data_.resize(static_cast<usize>(glyph_count) * static_cast<usize>(char_size));

        file.read(reinterpret_cast<char*>(font.glyph_data_.data()), static_cast<std::streamsize>(font.glyph_data_.size()));
        if (!file) {
            return Error{ErrorCode::FileReadError, "Failed to read PSF1 glyph table"};
        }

        return font;
    }

    // PSF2
    if (read_u32_le(magic) == 0x864ab572u) {
        u8 header_bytes[32]{};
        file.read(reinterpret_cast<char*>(header_bytes), 32);
        if (!file) {
            return Error{ErrorCode::FileReadError, "Invalid PSF2 header"};
        }

        const u32 version = read_u32_le(&header_bytes[4]);
        const u32 header_size = read_u32_le(&header_bytes[8]);
        const u32 glyph_count = read_u32_le(&header_bytes[16]);
        const u32 bytes_per_glyph = read_u32_le(&header_bytes[20]);
        const u32 glyph_height = read_u32_le(&header_bytes[24]);
        const u32 glyph_width = read_u32_le(&header_bytes[28]);

        if (version != 0 || header_size < 32 || glyph_count == 0 || bytes_per_glyph == 0 || glyph_height == 0 || glyph_width == 0) {
            return Error{ErrorCode::FileReadError, "Unsupported PSF2 header values"};
        }

        file.seekg(static_cast<std::streamoff>(header_size), std::ios::beg);

        font.glyph_width_ = static_cast<i32>(glyph_width);
        font.glyph_height_ = static_cast<i32>(glyph_height);
        font.first_char_ = 0;
        font.last_char_ = static_cast<i32>(glyph_count - 1);
        font.glyph_count_ = static_cast<i32>(glyph_count);
        font.bytes_per_glyph_ = static_cast<i32>(bytes_per_glyph);
        font.glyph_data_.resize(static_cast<usize>(glyph_count) * static_cast<usize>(bytes_per_glyph));

        file.read(reinterpret_cast<char*>(font.glyph_data_.data()), static_cast<std::streamsize>(font.glyph_data_.size()));
        if (!file) {
            return Error{ErrorCode::FileReadError, "Failed to read PSF2 glyph table"};
        }

        return font;
    }

    return Error{ErrorCode::NotSupported, "Unsupported font format. Expected PSF1 or PSF2."};
}

f32 Font::char_width_for_size(f32 size) const {
    return size * static_cast<f32>(glyph_width_) / static_cast<f32>(glyph_height_);
}

f32 Font::line_height_for_size(f32 size) const {
    return size;
}

bool Font::get_pixel(char c, i32 x, i32 y) const {
    if (!valid()) {
        return false;
    }

    if (x < 0 || x >= glyph_width_ || y < 0 || y >= glyph_height_) {
        return false;
    }

    const u8 uc = static_cast<u8>(c);
    i32 glyph_index = static_cast<i32>(uc);

    if (glyph_index < first_char_ || glyph_index > last_char_) {
        glyph_index = static_cast<i32>('?');
        if (glyph_index < first_char_ || glyph_index > last_char_) {
            glyph_index = first_char_;
        }
    }

    const usize byte_index_base = static_cast<usize>(glyph_index - first_char_) * static_cast<usize>(bytes_per_glyph_);
    const i32 row_stride = (glyph_width_ + 7) / 8;
    const usize byte_index = byte_index_base + static_cast<usize>(y * row_stride + (x / 8));
    if (byte_index >= glyph_data_.size()) {
        return false;
    }

    const u8 row_byte = glyph_data_[byte_index];
    return ((row_byte >> (7 - (x % 8))) & 0x1u) != 0;
}

bool Font::valid() const {
    const bool has_data = !glyph_data_.empty() || !glyph_alpha_data_.empty();
    return glyph_width_ > 0 && glyph_height_ > 0 && glyph_count_ > 0 && has_data;
}

u8 Font::get_alpha(char c, i32 x, i32 y) const {
    if (x < 0 || x >= glyph_width_ || y < 0 || y >= glyph_height_) {
        return 0;
    }

    const u8 uc = static_cast<u8>(c);
    i32 glyph_index = static_cast<i32>(uc);
    if (glyph_index < first_char_ || glyph_index > last_char_) {
        glyph_index = static_cast<i32>('?');
        if (glyph_index < first_char_ || glyph_index > last_char_) {
            return 0;
        }
    }

    if (!glyph_alpha_data_.empty()) {
        const usize cell_pixels = static_cast<usize>(glyph_width_) * static_cast<usize>(glyph_height_);
        const usize idx =
            static_cast<usize>(glyph_index - first_char_) * cell_pixels +
            static_cast<usize>(y) * static_cast<usize>(glyph_width_) +
            static_cast<usize>(x);
        return (idx < glyph_alpha_data_.size()) ? glyph_alpha_data_[idx] : 0;
    }

    // PSF/fixed data: return binary alpha
    return get_pixel(c, x, y) ? 255 : 0;
}

i32 Font::glyph_advance_x(char c) const {
    if (glyph_advances_x_.empty()) {
        return glyph_width_;
    }

    const u8 uc = static_cast<u8>(c);
    const i32 glyph_index = static_cast<i32>(uc);
    if (glyph_index < first_char_ || glyph_index > last_char_) {
        return glyph_width_;
    }

    return glyph_advances_x_[static_cast<usize>(glyph_index - first_char_)];
}

} // namespace frost
