#include "frost/graphics/font.hpp"
#include "frost/graphics/bitmap_font.hpp"
#include <fstream>
#include <cstring>

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

Result<Font> Font::load_from_file(StringView file_path) {
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
    return glyph_width_ > 0 && glyph_height_ > 0 && glyph_count_ > 0 && bytes_per_glyph_ > 0 && !glyph_data_.empty();
}

} // namespace frost
