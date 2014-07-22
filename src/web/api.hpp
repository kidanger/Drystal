#include "module.hpp"
#include "web_bind.hpp"

BEGIN_MODULE(web)
	DECLARE_FUNCTION(is_web)
	DECLARE_FUNCTION(wget)
	DECLARE_FUNCTION(run_js)
END_MODULE()

