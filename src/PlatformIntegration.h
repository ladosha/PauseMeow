#pragma once

namespace PlatformIntegration {

[[nodiscard]] bool setAutostartEnabled(bool enabled);
[[nodiscard]] bool reconcileAutostartEnabled(bool savedEnabled);
void resetUserState();

}
