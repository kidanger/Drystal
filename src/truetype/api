#include "module.hpp"

BEGIN_MODULE(truetype)
	DECLARE_FUNCTION(load_font)

	BEGIN_CLASS(font)
		ADD_METHOD(font, draw)
		ADD_METHOD(font, draw_plain)
		ADD_METHOD(font, sizeof)
		ADD_METHOD(font, sizeof_plain)
		ADD_GC(free_font)
		END_CLASS();
	REGISTER_CLASS(font, "Font");
END_MODULE()

