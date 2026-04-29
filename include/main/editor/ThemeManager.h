#pragma once

namespace Editor {

enum class EditorTheme {
    ModernDark,
    ModernLight,
    Retro
};

class ThemeManager {
public:
    static void ApplyTheme(EditorTheme theme);
};

} // namespace Editor
