#pragma once

struct Surface;
struct stbtt_bakedchar;

class Font {
	private:
		Surface* surface;
		float font_size;
		int first_char;
		int num_chars;
		stbtt_bakedchar* char_data;
	public:
		~Font();
		void draw(const char* text, float x, float y);
		void draw_plain(const char* text, float x, float y);
		void get_textsize(const char* text, float* w, float* h);
		void get_textsize_plain(const char* text, float* w, float* h);

		static Font* load(const char* filename, float size, int first_char=32, int num_chars=96);
};

