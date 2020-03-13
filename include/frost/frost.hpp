#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// FrostUI - Industry-Grade GUI Framework
// ─────────────────────────────────────────────────────────────────────────────
//
// Master header that includes all public API headers.
// Include this for convenience, or include individual headers for faster builds.
//

// Core
#include "frost/core/types.hpp"
#include "frost/core/result.hpp"
#include "frost/core/math.hpp"
#include "frost/core/signals.hpp"

// Platform
#include "frost/platform/input.hpp"
#include "frost/platform/window.hpp"

// Graphics
#include "frost/graphics/color.hpp"
#include "frost/graphics/draw_list.hpp"

// UI
#include "frost/ui/widget.hpp"
#include "frost/ui/layout.hpp"
#include "frost/ui/application.hpp"
#include "frost/ui/widgets/label.hpp"
#include "frost/ui/widgets/button.hpp"
#include "frost/ui/widgets/text_input.hpp"
