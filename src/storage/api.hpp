#include "module.hpp"
#include "storage_bind.hpp"

BEGIN_MODULE(storage)
	DECLARE_FUNCTION(store)
	DECLARE_FUNCTION(fetch)
END_MODULE()

