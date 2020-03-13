#include "frost/platform/window.hpp"

namespace frost {
namespace platform {

Result<void> initialize() {
    // Platform-specific initialization will be implemented in platform files
    return {};
}

void shutdown() {
    // Platform-specific cleanup will be implemented in platform files
}

f32 get_primary_monitor_dpi_scale() {
    // TODO: Implement platform-specific DPI detection
    return 1.0f;
}

f32 get_primary_monitor_refresh_rate() {
    // TODO: Implement platform-specific refresh rate detection
    return 60.0f;
}

Vector<MonitorInfo> get_monitors() {
    // TODO: Implement platform-specific monitor enumeration
    return {};
}

} // namespace platform
} // namespace frost
