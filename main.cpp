#include <cstdint>

#include "frame_buffer_config.hpp"

struct PixelColor
{
    uint8_t r, g, b;
};

class PixelWriter
{
    public:
        PixelWriter(const FrameBufferConfig& config) : config_{config}
        {

        }
        virtual ~PixelWriter() = default;
        virtual void Write(int x, int y, const PixelColor& c) = 0;
    
    protected:
        uint8_t* PixelAt(int x, int y)
        {
            return config_.frame_buffer + 4 * (config_.pixels_per_scan_line * y + x);
        }
    
    private:
        const FrameBufferConfig& config_;
};

class RGBResv8BitColorPixelWriter : public PixelWriter 
{
    public:
        using PixelWriter::PixelWriter;
        virtual void Write(int x, int y, const PixelColor& c) override
        {
            auto p = PixelAt(x, y);
            p[0] = c.r;
            p[1] = c.g;
            p[2] = c.b;
        }
};

class BGRResv8BitColorPixelWriter : public PixelWriter 
{
    public:
        using PixelWriter::PixelWriter;
        virtual void Write(int x, int y, const PixelColor& c) override
        {
            auto p = PixelAt(x, y);
            p[0] = c.b;
            p[1] = c.g;
            p[2] = c.r;
        }
};

void* operator new(size_t size, void* buf)
{
    return buf;
}

void operator delete(void* obj) noexcept
{

}

char pixel_writer_buf[sizeof(RGBResv8BitColorPixelWriter)];
PixelWriter* pixel_writer;


// 言語リンケージの宣言(C言語形式で記述)
extern "C" void KernelMain(
    // ブートローダから情報を受け取る
    const FrameBufferConfig& frame_buffer_config
)
{
    switch (frame_buffer_config.pixel_format)
    {
    case kPixelRGBResv8BitPerColor:
        pixel_writer = new(pixel_writer_buf)
            RGBResv8BitColorPixelWriter{frame_buffer_config};
        break;

    case kPixelBGRResv8BitPerColor:
        pixel_writer = new(pixel_writer_buf)
            BGRResv8BitColorPixelWriter{frame_buffer_config};
        break;
    }
    // PC画面いっぱいに色を書く
    for (int x = 0; x < frame_buffer_config.horizontal_resolution; ++x)
    {
        for (int y = 0; y < frame_buffer_config.vertical_resolution; ++y)
        {
            // ピクセルに色を書き込む
            pixel_writer->Write(x, y, {255, 255, 0});
        }
    }

    // 色を塗る範囲を指定する
    for (int x = 0; x < 200; ++x)
    {
        for (int y = 100; y < 200; ++y)
        {
                        pixel_writer->Write(x, y, {0, 255, 0});
        }
    }
    // __asm__: インラインアセンブラ
    while (1) __asm__("hlt");
}
