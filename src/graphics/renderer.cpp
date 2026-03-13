#include "frost/graphics/renderer.hpp"

namespace frost {

bool is_renderer_backend_available(RendererBackend backend) {
	switch (backend) {
		case RendererBackend::Software:
			return true;
		case RendererBackend::Gpu:
#ifdef FROST_NO_VULKAN
			return false;
#else
			return true;
#endif
	}

	return false;
}

StringView renderer_backend_name(RendererBackend backend) {
	switch (backend) {
		case RendererBackend::Software:
			return "software";
		case RendererBackend::Gpu:
			return "gpu";
	}

	return "unknown";
}

} // namespace frost
