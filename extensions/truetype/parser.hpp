#pragma once

struct TextState {
	float size;
	float italic;

	int r;
	int g;
	int b;
	int alpha;

	bool outlined;
	int outr;
	int outg;
	int outb;

	bool shadow;
	float shadow_x;
	float shadow_y;

	TextState() :
		size(1.0),
		alpha(255)
	{
	}
};

bool parse(TextState** state, const char*& text, const char*& end);
void reset_parser();
