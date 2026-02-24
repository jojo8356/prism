#ifndef PRISM_HPP
#define PRISM_HPP

#include <iostream>
#include <string>
#include <string_view>
#include <sstream>
#include <type_traits>
#include <cstddef>
#include <cmath>
#include <algorithm>
#include <cstdlib>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define PRISM_ISATTY _isatty
#define PRISM_FILENO _fileno
#else
#include <unistd.h>
#define PRISM_ISATTY isatty
#define PRISM_FILENO fileno
#endif
#include <utility>
#include <tuple>
#include <optional>
#include <variant>
#include <vector>
#include <array>
#include <list>
#include <deque>
#include <forward_list>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

namespace prism {

// ============================================================================
// Phase 1 : Fondations
// ============================================================================

// ---------------------------------------------------------------------------
// 1.2 — Trait Formatter<T>
// ---------------------------------------------------------------------------
// Priorité : Formatter<T> spécialisé > operator<< > static_assert
//
// Pour spécialiser :
//   template<> struct prism::Formatter<MyType> {
//       static std::string format(const MyType& v) { return "..."; }
//   };

// Tag par défaut — indique que le Formatter n'est PAS spécialisé
struct formatter_not_specialized {};

template <typename T, typename Enable = void>
struct Formatter : formatter_not_specialized {};

// Détection : Formatter<T> est-il spécialisé ?
template <typename T, typename = void>
struct has_formatter : std::false_type {};

template <typename T>
struct has_formatter<T, std::void_t<
    decltype(Formatter<std::decay_t<T>>::format(std::declval<const std::decay_t<T>&>()))
>> : std::negation<std::is_base_of<formatter_not_specialized, Formatter<std::decay_t<T>>>> {};

template <typename T>
inline constexpr bool has_formatter_v = has_formatter<T>::value;

// Détection : operator<<(ostream, T) existe-t-il ?
template <typename T, typename = void>
struct has_ostream_op : std::false_type {};

template <typename T>
struct has_ostream_op<T, std::void_t<
    decltype(std::declval<std::ostream&>() << std::declval<const T&>())
>> : std::true_type {};

template <typename T>
inline constexpr bool has_ostream_op_v = has_ostream_op<T>::value;

// ---------------------------------------------------------------------------
// 1.3 — to_string_value(T) — conversion universelle
// ---------------------------------------------------------------------------

// Cas 1 : Formatter<T> spécialisé
template <typename T>
inline auto to_string_value(const T& val)
    -> std::enable_if_t<has_formatter_v<std::decay_t<T>>, std::string>
{
    return Formatter<std::decay_t<T>>::format(val);
}

// Cas 2 : operator<< disponible (et pas de Formatter spécialisé)
template <typename T>
inline auto to_string_value(const T& val)
    -> std::enable_if_t<!has_formatter_v<std::decay_t<T>> && has_ostream_op_v<T>, std::string>
{
    std::ostringstream oss;
    oss << val;
    return oss.str();
}

// Cas 3 : Ni Formatter ni operator<< — erreur de compilation
template <typename T>
inline auto to_string_value(const T&)
    -> std::enable_if_t<!has_formatter_v<std::decay_t<T>> && !has_ostream_op_v<T>, std::string>
{
    static_assert(has_formatter_v<std::decay_t<T>> || has_ostream_op_v<T>,
        "prism::print — type is not printable. "
        "Either specialize prism::Formatter<T> or provide operator<<(ostream&, const T&).");
    return {};
}

// ---------------------------------------------------------------------------
// Écriture directe dans un ostream (évite allocation quand possible)
// ---------------------------------------------------------------------------

// Dispatch : Formatter spécialisé → format() puis write
template <typename T>
inline auto write_value(std::ostream& os, const T& val)
    -> std::enable_if_t<has_formatter_v<std::decay_t<T>>>
{
    os << Formatter<std::decay_t<T>>::format(val);
}

// Dispatch : operator<< → écriture directe (zéro allocation)
template <typename T>
inline auto write_value(std::ostream& os, const T& val)
    -> std::enable_if_t<!has_formatter_v<std::decay_t<T>> && has_ostream_op_v<T>>
{
    os << val;
}

// Dispatch : rien → static_assert
template <typename T>
inline auto write_value(std::ostream&, const T&)
    -> std::enable_if_t<!has_formatter_v<std::decay_t<T>> && !has_ostream_op_v<T>>
{
    static_assert(has_formatter_v<std::decay_t<T>> || has_ostream_op_v<T>,
        "prism::print — type is not printable. "
        "Either specialize prism::Formatter<T> or provide operator<<(ostream&, const T&).");
}

// ---------------------------------------------------------------------------
// Spécialisations Formatter pour les types de base
// ---------------------------------------------------------------------------

// bool → "true" / "false" (au lieu de 1/0)
template <>
struct Formatter<bool> {
    static std::string format(const bool& val) {
        return val ? "true" : "false";
    }
};

// nullptr_t → "nullptr"
template <>
struct Formatter<std::nullptr_t> {
    static std::string format(const std::nullptr_t&) {
        return "nullptr";
    }
};

// Écriture directe pour les types légers (évite Formatter::format → string → os)
// bool : écriture directe sans allocation
inline void write_value(std::ostream& os, const bool& val) {
    os << (val ? "true" : "false");
}

// nullptr_t : écriture directe sans allocation
inline void write_value(std::ostream& os, const std::nullptr_t&) {
    os << "nullptr";
}

// StyledText : forward-declaration de write_value (défini après StyledText)
struct StyledText;
inline void write_value(std::ostream& os, const StyledText& st);

// ============================================================================
// Phase 3 : Types STL
// ============================================================================

// ---------------------------------------------------------------------------
// 3.0 — Traits de détection pour containers
// ---------------------------------------------------------------------------

namespace detail {

// Détecte si T est itérable (a begin/end)
template <typename T, typename = void>
struct is_iterable : std::false_type {};

template <typename T>
struct is_iterable<T, std::void_t<
    decltype(std::begin(std::declval<const T&>())),
    decltype(std::end(std::declval<const T&>()))
>> : std::true_type {};

template <typename T>
inline constexpr bool is_iterable_v = is_iterable<T>::value;

// Détecte un type map-like (a mapped_type)
template <typename T, typename = void>
struct is_map_like : std::false_type {};

template <typename T>
struct is_map_like<T, std::void_t<typename T::mapped_type>> : std::true_type {};

template <typename T>
inline constexpr bool is_map_like_v = is_map_like<T>::value;

// Détecte un type set-like (a key_type mais pas mapped_type)
template <typename T, typename = void>
struct is_set_like : std::false_type {};

template <typename T>
struct is_set_like<T, std::enable_if_t<
    !is_map_like_v<T> &&
    !std::is_same_v<std::decay_t<T>, std::string> &&
    !std::is_same_v<std::decay_t<T>, std::string_view>,
    std::void_t<typename T::key_type>
>> : std::true_type {};

template <typename T>
inline constexpr bool is_set_like_v = is_set_like<T>::value;

// Container séquentiel : itérable, pas map, pas set, pas string
template <typename T>
inline constexpr bool is_sequence_v =
    is_iterable_v<T> &&
    !is_map_like_v<T> &&
    !is_set_like_v<T> &&
    !std::is_same_v<std::decay_t<T>, std::string> &&
    !std::is_same_v<std::decay_t<T>, std::string_view>;

// Détecte std::pair
template <typename T>
struct is_pair : std::false_type {};

template <typename A, typename B>
struct is_pair<std::pair<A, B>> : std::true_type {};

template <typename T>
inline constexpr bool is_pair_v = is_pair<std::decay_t<T>>::value;

// Détecte std::tuple
template <typename T>
struct is_tuple : std::false_type {};

template <typename... Ts>
struct is_tuple<std::tuple<Ts...>> : std::true_type {};

template <typename T>
inline constexpr bool is_tuple_v = is_tuple<std::decay_t<T>>::value;

// Détecte std::optional
template <typename T>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<std::optional<T>> : std::true_type {};

template <typename T>
inline constexpr bool is_optional_v = is_optional<std::decay_t<T>>::value;

// Détecte std::variant
template <typename T>
struct is_variant : std::false_type {};

template <typename... Ts>
struct is_variant<std::variant<Ts...>> : std::true_type {};

template <typename T>
inline constexpr bool is_variant_v = is_variant<std::decay_t<T>>::value;

} // namespace detail

// ---------------------------------------------------------------------------
// 3.1 — Containers séquentiels (vector, array, list, deque...) → [1, 2, 3]
// ---------------------------------------------------------------------------

template <typename T>
struct Formatter<T, std::enable_if_t<detail::is_sequence_v<T>>> {
    static std::string format(const T& container) {
        std::ostringstream oss;
        oss << '[';
        bool first = true;
        for (const auto& elem : container) {
            if (!first) oss << ", ";
            first = false;
            oss << to_string_value(elem);
        }
        oss << ']';
        return oss.str();
    }
};

// ---------------------------------------------------------------------------
// 3.2 — Containers associatifs (map, unordered_map) → {a: 1, b: 2}
// ---------------------------------------------------------------------------

template <typename T>
struct Formatter<T, std::enable_if_t<detail::is_map_like_v<T>>> {
    static std::string format(const T& container) {
        std::ostringstream oss;
        oss << '{';
        bool first = true;
        for (const auto& [key, val] : container) {
            if (!first) oss << ", ";
            first = false;
            oss << to_string_value(key) << ": " << to_string_value(val);
        }
        oss << '}';
        return oss.str();
    }
};

// ---------------------------------------------------------------------------
// 3.3 — Sets (set, unordered_set) → {1, 2, 3}
// ---------------------------------------------------------------------------

template <typename T>
struct Formatter<T, std::enable_if_t<detail::is_set_like_v<T>>> {
    static std::string format(const T& container) {
        std::ostringstream oss;
        oss << '{';
        bool first = true;
        for (const auto& elem : container) {
            if (!first) oss << ", ";
            first = false;
            oss << to_string_value(elem);
        }
        oss << '}';
        return oss.str();
    }
};

// ---------------------------------------------------------------------------
// 3.4 — pair → (1, "hello") et tuple → (1, "hello", 3.14)
// ---------------------------------------------------------------------------

template <typename A, typename B>
struct Formatter<std::pair<A, B>> {
    static std::string format(const std::pair<A, B>& p) {
        std::ostringstream oss;
        oss << '(' << to_string_value(p.first) << ", " << to_string_value(p.second) << ')';
        return oss.str();
    }
};

template <typename... Ts>
struct Formatter<std::tuple<Ts...>> {
    static std::string format(const std::tuple<Ts...>& t) {
        std::ostringstream oss;
        oss << '(';
        format_impl(oss, t, std::index_sequence_for<Ts...>{});
        oss << ')';
        return oss.str();
    }

private:
    template <std::size_t... Is>
    static void format_impl(std::ostringstream& oss, const std::tuple<Ts...>& t,
                             std::index_sequence<Is...>) {
        ((oss << (Is == 0 ? "" : ", ") << to_string_value(std::get<Is>(t))), ...);
    }
};

// Tuple vide
template <>
struct Formatter<std::tuple<>> {
    static std::string format(const std::tuple<>&) {
        return "()";
    }
};

// ---------------------------------------------------------------------------
// 3.5 — optional → Some(42) / None
// ---------------------------------------------------------------------------

template <typename T>
struct Formatter<std::optional<T>> {
    static std::string format(const std::optional<T>& opt) {
        if (opt.has_value()) {
            return "Some(" + to_string_value(*opt) + ")";
        }
        return "None";
    }
};

// ---------------------------------------------------------------------------
// 3.6 — variant → valeur active
// ---------------------------------------------------------------------------

template <typename... Ts>
struct Formatter<std::variant<Ts...>> {
    static std::string format(const std::variant<Ts...>& v) {
        return std::visit([](const auto& val) -> std::string {
            return to_string_value(val);
        }, v);
    }
};

// ============================================================================
// Phase 4 : Système de couleurs — Core
// ============================================================================

// ---------------------------------------------------------------------------
// 4.1 — StyledText et ColorWrapper
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// 7.3 / 7.4 — Détection auto du support couleur + forçage manuel
// ---------------------------------------------------------------------------

namespace detail {

enum class ColorOverride { automatic, force_on, force_off };

inline ColorOverride& color_override() {
    static ColorOverride state = ColorOverride::automatic;
    return state;
}

inline bool detect_color_support() {
    // NO_COLOR (https://no-color.org/)
    if (std::getenv("NO_COLOR") != nullptr) return false;

    // TERM=dumb
    const char* term = std::getenv("TERM");
    if (term != nullptr && std::string_view(term) == "dumb") return false;

    // stdout est un TTY ?
    if (PRISM_ISATTY(PRISM_FILENO(stdout)) == 0) return false;

#ifdef _WIN32
    // Windows 10+ : activer le mode ANSI (Virtual Terminal Processing)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode)) {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, mode);
        }
    }
#endif

    return true;
}

inline bool colors_enabled() {
    switch (color_override()) {
        case ColorOverride::force_on:  return true;
        case ColorOverride::force_off: return false;
        default: {
            static bool detected = detect_color_support();
            return detected;
        }
    }
}

} // namespace detail

inline void enable_colors(bool on) {
    detail::color_override() = on ? detail::ColorOverride::force_on
                                  : detail::ColorOverride::force_off;
}

inline void reset_colors() {
    detail::color_override() = detail::ColorOverride::automatic;
}

// ---------------------------------------------------------------------------
// 4.1 — StyledText et ColorWrapper
// ---------------------------------------------------------------------------

struct StyledText {
    std::string content;
};

template <>
struct Formatter<StyledText> {
    static std::string format(const StyledText& st) {
        return st.content;
    }
};

// Écriture directe pour StyledText (évite copie via Formatter::format)
inline void write_value(std::ostream& os, const StyledText& st) {
    os << st.content;
}

struct ColorWrapper {
    std::string prefix;
    std::string suffix;

    ColorWrapper(std::string p, std::string s)
        : prefix(std::move(p)), suffix(std::move(s)) {}

    // Appel avec n'importe quel type printable
    template <typename T>
    StyledText operator()(const T& val) const {
        std::string text = to_string_value(val);
        if (!detail::colors_enabled()) return StyledText{std::move(text)};
        return StyledText{prefix + text + suffix};
    }

    // Surcharge spécifique pour StyledText (emboîtement)
    StyledText operator()(const StyledText& inner) const {
        if (!detail::colors_enabled()) return inner;
        return StyledText{prefix + inner.content + suffix};
    }
};

// ---------------------------------------------------------------------------
// 4.2 — rgb(r, g, b)
// ---------------------------------------------------------------------------

namespace detail {

inline std::string fg_ansi(int r, int g, int b) {
    return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
}

inline constexpr const char* ansi_reset = "\033[0m";

constexpr int clamp_byte(int v) {
    return v < 0 ? 0 : (v > 255 ? 255 : v);
}

} // namespace detail

inline ColorWrapper rgb(int r, int g, int b) {
    r = detail::clamp_byte(r);
    g = detail::clamp_byte(g);
    b = detail::clamp_byte(b);
    return ColorWrapper(detail::fg_ansi(r, g, b), detail::ansi_reset);
}

// ---------------------------------------------------------------------------
// 4.3 — hex("#RRGGBB") et hex("#RGB")
// ---------------------------------------------------------------------------

namespace detail {

constexpr int hex_char_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

struct RGB {
    int r, g, b;
};

inline RGB parse_hex(std::string_view s) {
    // Retirer le '#' si présent
    if (!s.empty() && s[0] == '#') s.remove_prefix(1);

    if (s.size() == 3) {
        // #RGB → #RRGGBB
        int r = hex_char_to_int(s[0]);
        int g = hex_char_to_int(s[1]);
        int b = hex_char_to_int(s[2]);
        if (r < 0 || g < 0 || b < 0) return {255, 255, 255};
        return {r * 17, g * 17, b * 17};
    }

    if (s.size() == 6) {
        int r = hex_char_to_int(s[0]) * 16 + hex_char_to_int(s[1]);
        int g = hex_char_to_int(s[2]) * 16 + hex_char_to_int(s[3]);
        int b = hex_char_to_int(s[4]) * 16 + hex_char_to_int(s[5]);
        if (r < 0 || g < 0 || b < 0) return {255, 255, 255};
        return {r, g, b};
    }

    // Invalide → fallback blanc
    return {255, 255, 255};
}

} // namespace detail

inline ColorWrapper hex(std::string_view h) {
    auto [r, g, b] = detail::parse_hex(h);
    return rgb(r, g, b);
}

// ---------------------------------------------------------------------------
// 4.4 — hsl(h, s, l) — conversion HSL → RGB
// ---------------------------------------------------------------------------

namespace detail {

inline RGB hsl_to_rgb(double h, double s, double l) {
    // Clamp
    h = std::fmod(h, 360.0);
    if (h < 0) h += 360.0;
    s = std::clamp(s, 0.0, 100.0) / 100.0;
    l = std::clamp(l, 0.0, 100.0) / 100.0;

    auto hue_to_rgb = [](double p, double q, double t) -> double {
        if (t < 0.0) t += 1.0;
        if (t > 1.0) t -= 1.0;
        if (t < 1.0 / 6.0) return p + (q - p) * 6.0 * t;
        if (t < 1.0 / 2.0) return q;
        if (t < 2.0 / 3.0) return p + (q - p) * (2.0 / 3.0 - t) * 6.0;
        return p;
    };

    double r_f, g_f, b_f;

    if (s == 0.0) {
        r_f = g_f = b_f = l;
    } else {
        double q = l < 0.5 ? l * (1.0 + s) : l + s - l * s;
        double p = 2.0 * l - q;
        double hk = h / 360.0;
        r_f = hue_to_rgb(p, q, hk + 1.0 / 3.0);
        g_f = hue_to_rgb(p, q, hk);
        b_f = hue_to_rgb(p, q, hk - 1.0 / 3.0);
    }

    return {
        static_cast<int>(std::round(r_f * 255.0)),
        static_cast<int>(std::round(g_f * 255.0)),
        static_cast<int>(std::round(b_f * 255.0))
    };
}

} // namespace detail

inline ColorWrapper hsl(double h, double s, double l) {
    auto [r, g, b] = detail::hsl_to_rgb(h, s, l);
    return rgb(r, g, b);
}

// ---------------------------------------------------------------------------
// 4.5 — oklch(L, C, H) — conversion OKLCH → OKLab → linear RGB → sRGB
// ---------------------------------------------------------------------------

namespace detail {

inline RGB oklch_to_rgb(double L, double C, double H) {
    // Clamp
    L = std::clamp(L, 0.0, 1.0);
    C = std::clamp(C, 0.0, 0.4);
    H = std::fmod(H, 360.0);
    if (H < 0) H += 360.0;

    // OKLCH → OKLab
    double h_rad = H * 3.14159265358979323846 / 180.0;
    double a = C * std::cos(h_rad);
    double b = C * std::sin(h_rad);

    // OKLab → LMS (cube root space)
    double l_ = L + 0.3963377774 * a + 0.2158037573 * b;
    double m_ = L - 0.1055613458 * a - 0.0638541728 * b;
    double s_ = L - 0.0894841775 * a - 1.2914855480 * b;

    // LMS cube root → LMS linear
    double l_lin = l_ * l_ * l_;
    double m_lin = m_ * m_ * m_;
    double s_lin = s_ * s_ * s_;

    // LMS → linear sRGB
    double r_lin = +4.0767416621 * l_lin - 3.3077115913 * m_lin + 0.2309699292 * s_lin;
    double g_lin = -1.2684380046 * l_lin + 2.6097574011 * m_lin - 0.3413193965 * s_lin;
    double b_lin = -0.0041960863 * l_lin - 0.7034186147 * m_lin + 1.7076147010 * s_lin;

    // Linear sRGB → sRGB (gamma)
    auto linear_to_srgb = [](double x) -> double {
        if (x <= 0.0) return 0.0;
        if (x >= 1.0) return 1.0;
        return x <= 0.0031308
            ? 12.92 * x
            : 1.055 * std::pow(x, 1.0 / 2.4) - 0.055;
    };

    double r_f = linear_to_srgb(r_lin);
    double g_f = linear_to_srgb(g_lin);
    double b_f = linear_to_srgb(b_lin);

    return {
        static_cast<int>(std::round(std::clamp(r_f, 0.0, 1.0) * 255.0)),
        static_cast<int>(std::round(std::clamp(g_f, 0.0, 1.0) * 255.0)),
        static_cast<int>(std::round(std::clamp(b_f, 0.0, 1.0) * 255.0))
    };
}

} // namespace detail

inline ColorWrapper oklch(double L, double C, double H) {
    auto [r, g, b] = detail::oklch_to_rgb(L, C, H);
    return rgb(r, g, b);
}

// ============================================================================
// Phase 5 : Couleurs — Background et styles
// ============================================================================

// ---------------------------------------------------------------------------
// 5.1 — bg_rgb(), bg_hex(), bg_hsl(), bg_oklch()
// ---------------------------------------------------------------------------

namespace detail {

inline std::string bg_ansi(int r, int g, int b) {
    return "\033[48;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
}

} // namespace detail

inline ColorWrapper bg_rgb(int r, int g, int b) {
    r = detail::clamp_byte(r);
    g = detail::clamp_byte(g);
    b = detail::clamp_byte(b);
    return ColorWrapper(detail::bg_ansi(r, g, b), detail::ansi_reset);
}

inline ColorWrapper bg_hex(std::string_view h) {
    auto [r, g, b] = detail::parse_hex(h);
    return bg_rgb(r, g, b);
}

inline ColorWrapper bg_hsl(double h, double s, double l) {
    auto [r, g, b] = detail::hsl_to_rgb(h, s, l);
    return bg_rgb(r, g, b);
}

inline ColorWrapper bg_oklch(double L, double C, double H) {
    auto [r, g, b] = detail::oklch_to_rgb(L, C, H);
    return bg_rgb(r, g, b);
}

// ---------------------------------------------------------------------------
// 5.2 — Styles de texte : bold, dim, italic, underline, strikethrough
// ---------------------------------------------------------------------------

namespace detail {

inline ColorWrapper make_style(const char* on, const char* off) {
    return ColorWrapper(on, off);
}

} // namespace detail

template <typename T>
inline StyledText bold(const T& val) {
    return detail::make_style("\033[1m", "\033[22m")(val);
}

template <typename T>
inline StyledText dim(const T& val) {
    return detail::make_style("\033[2m", "\033[22m")(val);
}

template <typename T>
inline StyledText italic(const T& val) {
    return detail::make_style("\033[3m", "\033[23m")(val);
}

template <typename T>
inline StyledText underline(const T& val) {
    return detail::make_style("\033[4m", "\033[24m")(val);
}

template <typename T>
inline StyledText strikethrough(const T& val) {
    return detail::make_style("\033[9m", "\033[29m")(val);
}

// ============================================================================
// Phase 6 : 148 couleurs CSS nommées
// ============================================================================

// ---------------------------------------------------------------------------
// 6.1 — Table X-macro des 148 couleurs CSS (nom, R, G, B)
// ---------------------------------------------------------------------------

#define PRISM_CSS_COLORS(X) \
    X(aliceblue,            240, 248, 255) \
    X(antiquewhite,         250, 235, 215) \
    X(aqua,                   0, 255, 255) \
    X(aquamarine,           127, 255, 212) \
    X(azure,                240, 255, 255) \
    X(beige,                245, 245, 220) \
    X(bisque,               255, 228, 196) \
    X(black,                  0,   0,   0) \
    X(blanchedalmond,       255, 235, 205) \
    X(blue,                   0,   0, 255) \
    X(blueviolet,           138,  43, 226) \
    X(brown,                165,  42,  42) \
    X(burlywood,            222, 184, 135) \
    X(cadetblue,             95, 158, 160) \
    X(chartreuse,           127, 255,   0) \
    X(chocolate,            210, 105,  30) \
    X(coral,                255, 127,  80) \
    X(cornflowerblue,       100, 149, 237) \
    X(cornsilk,             255, 248, 220) \
    X(crimson,              220,  20,  60) \
    X(cyan,                   0, 255, 255) \
    X(darkblue,               0,   0, 139) \
    X(darkcyan,               0, 139, 139) \
    X(darkgoldenrod,        184, 134,  11) \
    X(darkgray,             169, 169, 169) \
    X(darkgreen,              0, 100,   0) \
    X(darkgrey,             169, 169, 169) \
    X(darkkhaki,            189, 183, 107) \
    X(darkmagenta,          139,   0, 139) \
    X(darkolivegreen,        85, 107,  47) \
    X(darkorange,           255, 140,   0) \
    X(darkorchid,           153,  50, 204) \
    X(darkred,              139,   0,   0) \
    X(darksalmon,           233, 150, 122) \
    X(darkseagreen,         143, 188, 143) \
    X(darkslateblue,         72,  61, 139) \
    X(darkslategray,         47,  79,  79) \
    X(darkslategrey,         47,  79,  79) \
    X(darkturquoise,          0, 206, 209) \
    X(darkviolet,           148,   0, 211) \
    X(deeppink,             255,  20, 147) \
    X(deepskyblue,            0, 191, 255) \
    X(dimgray,              105, 105, 105) \
    X(dimgrey,              105, 105, 105) \
    X(dodgerblue,            30, 144, 255) \
    X(firebrick,            178,  34,  34) \
    X(floralwhite,          255, 250, 240) \
    X(forestgreen,           34, 139,  34) \
    X(fuchsia,              255,   0, 255) \
    X(gainsboro,            220, 220, 220) \
    X(ghostwhite,           248, 248, 255) \
    X(gold,                 255, 215,   0) \
    X(goldenrod,            218, 165,  32) \
    X(gray,                 128, 128, 128) \
    X(green,                  0, 128,   0) \
    X(greenyellow,          173, 255,  47) \
    X(grey,                 128, 128, 128) \
    X(honeydew,             240, 255, 240) \
    X(hotpink,              255, 105, 180) \
    X(indianred,            205,  92,  92) \
    X(indigo,                75,   0, 130) \
    X(ivory,                255, 255, 240) \
    X(khaki,                240, 230, 140) \
    X(lavender,             230, 230, 250) \
    X(lavenderblush,        255, 240, 245) \
    X(lawngreen,            124, 252,   0) \
    X(lemonchiffon,         255, 250, 205) \
    X(lightblue,            173, 216, 230) \
    X(lightcoral,           240, 128, 128) \
    X(lightcyan,            224, 255, 255) \
    X(lightgoldenrodyellow, 250, 250, 210) \
    X(lightgray,            211, 211, 211) \
    X(lightgreen,           144, 238, 144) \
    X(lightgrey,            211, 211, 211) \
    X(lightpink,            255, 182, 193) \
    X(lightsalmon,          255, 160, 122) \
    X(lightseagreen,         32, 178, 170) \
    X(lightskyblue,         135, 206, 250) \
    X(lightslategray,       119, 136, 153) \
    X(lightslategrey,       119, 136, 153) \
    X(lightsteelblue,       176, 196, 222) \
    X(lightyellow,          255, 255, 224) \
    X(lime,                   0, 255,   0) \
    X(limegreen,             50, 205,  50) \
    X(linen,                250, 240, 230) \
    X(magenta,              255,   0, 255) \
    X(maroon,               128,   0,   0) \
    X(mediumaquamarine,     102, 205, 170) \
    X(mediumblue,             0,   0, 205) \
    X(mediumorchid,         186,  85, 211) \
    X(mediumpurple,         147, 112, 219) \
    X(mediumseagreen,        60, 179, 113) \
    X(mediumslateblue,      123, 104, 238) \
    X(mediumspringgreen,      0, 250, 154) \
    X(mediumturquoise,       72, 209, 204) \
    X(mediumvioletred,      199,  21, 133) \
    X(midnightblue,          25,  25, 112) \
    X(mintcream,            245, 255, 250) \
    X(mistyrose,            255, 228, 225) \
    X(moccasin,             255, 228, 181) \
    X(navajowhite,          255, 222, 173) \
    X(navy,                   0,   0, 128) \
    X(oldlace,              253, 245, 230) \
    X(olive,                128, 128,   0) \
    X(olivedrab,            107, 142,  35) \
    X(orange,               255, 165,   0) \
    X(orangered,            255,  69,   0) \
    X(orchid,               218, 112, 214) \
    X(palegoldenrod,        238, 232, 170) \
    X(palegreen,            152, 251, 152) \
    X(paleturquoise,        175, 238, 238) \
    X(palevioletred,        219, 112, 147) \
    X(papayawhip,           255, 239, 213) \
    X(peachpuff,            255, 218, 185) \
    X(peru,                 205, 133,  63) \
    X(pink,                 255, 192, 203) \
    X(plum,                 221, 160, 221) \
    X(powderblue,           176, 224, 230) \
    X(purple,               128,   0, 128) \
    X(rebeccapurple,        102,  51, 153) \
    X(red,                  255,   0,   0) \
    X(rosybrown,            188, 143, 143) \
    X(royalblue,             65, 105, 225) \
    X(saddlebrown,          139,  69,  19) \
    X(salmon,               250, 128, 114) \
    X(sandybrown,           244, 164,  96) \
    X(seagreen,              46, 139,  87) \
    X(seashell,             255, 245, 238) \
    X(sienna,               160,  82,  45) \
    X(silver,               192, 192, 192) \
    X(skyblue,              135, 206, 235) \
    X(slateblue,            106,  90, 205) \
    X(slategray,            112, 128, 144) \
    X(slategrey,            112, 128, 144) \
    X(snow,                 255, 250, 250) \
    X(springgreen,            0, 255, 127) \
    X(steelblue,             70, 130, 180) \
    X(tan,                  210, 180, 140) \
    X(teal,                   0, 128, 128) \
    X(thistle,              216, 191, 216) \
    X(tomato,               255,  99,  71) \
    X(turquoise,             64, 224, 208) \
    X(violet,               238, 130, 238) \
    X(wheat,                245, 222, 179) \
    X(white,                255, 255, 255) \
    X(whitesmoke,           245, 245, 245) \
    X(yellow,               255, 255,   0) \
    X(yellowgreen,          154, 205,  50)

// ---------------------------------------------------------------------------
// 6.2 — Fonctions foreground : red("text"), blue("text"), etc.
// ---------------------------------------------------------------------------

#define PRISM_DEFINE_CSS_COLOR_FG(name, r, g, b) \
    template <typename T>                         \
    inline StyledText name(const T& val) {        \
        return rgb(r, g, b)(val);                 \
    }

PRISM_CSS_COLORS(PRISM_DEFINE_CSS_COLOR_FG)
#undef PRISM_DEFINE_CSS_COLOR_FG

// ---------------------------------------------------------------------------
// 6.3 — Fonctions background : bg_red("text"), bg_blue("text"), etc.
// ---------------------------------------------------------------------------

#define PRISM_DEFINE_CSS_COLOR_BG(name, r, g, b)     \
    template <typename T>                             \
    inline StyledText bg_##name(const T& val) {       \
        return bg_rgb(r, g, b)(val);                  \
    }

PRISM_CSS_COLORS(PRISM_DEFINE_CSS_COLOR_BG)
#undef PRISM_DEFINE_CSS_COLOR_BG

// ---------------------------------------------------------------------------
// 6.4 — css("colorname") — lookup dynamique par nom string
// ---------------------------------------------------------------------------

namespace detail {

struct CSSColorEntry {
    const char* name;
    int r, g, b;
};

inline const CSSColorEntry* css_color_table() {
    static const CSSColorEntry table[] = {
#define PRISM_CSS_TABLE_ENTRY(name, r, g, b) {#name, r, g, b},
        PRISM_CSS_COLORS(PRISM_CSS_TABLE_ENTRY)
#undef PRISM_CSS_TABLE_ENTRY
        {nullptr, 0, 0, 0} // sentinel
    };
    return table;
}

inline std::size_t css_color_count() {
    const auto* table = css_color_table();
    static std::size_t count = [&]() {
        std::size_t n = 0;
        while (table[n].name != nullptr) ++n;
        return n;
    }();
    return count;
}

inline RGB lookup_css_color(std::string_view name) {
    // Table déjà triée alphabétiquement par la X-macro → recherche binaire
    const auto* table = css_color_table();
    const std::size_t count = css_color_count();

    std::size_t lo = 0, hi = count;
    while (lo < hi) {
        std::size_t mid = lo + (hi - lo) / 2;
        int cmp = name.compare(table[mid].name);
        if (cmp == 0) return {table[mid].r, table[mid].g, table[mid].b};
        if (cmp < 0) hi = mid;
        else lo = mid + 1;
    }
    // Pas trouvé → fallback blanc
    return {255, 255, 255};
}

} // namespace detail

inline ColorWrapper css(std::string_view name) {
    auto [r, g, b] = detail::lookup_css_color(name);
    return rgb(r, g, b);
}

inline ColorWrapper bg_css(std::string_view name) {
    auto [r, g, b] = detail::lookup_css_color(name);
    return bg_rgb(r, g, b);
}

// ---------------------------------------------------------------------------
// 1.4 — print(args...) et println(args...) — mode variadic
// ---------------------------------------------------------------------------

namespace detail {

// Écriture d'un seul argument dans un stream
template <typename T>
inline void print_one(std::ostream& os, const T& val) {
    write_value(os, val);
}

// Écriture variadic avec séparateur
inline void print_args(std::ostream&) {
    // Cas de base : zéro arguments → rien
}

template <typename T>
inline void print_args(std::ostream& os, const T& val) {
    print_one(os, val);
}

template <typename T, typename... Rest>
inline void print_args(std::ostream& os, const T& first, const Rest&... rest) {
    print_one(os, first);
    // Fold : ajouter espace + valeur pour chaque argument restant
    ((os << ' ', print_one(os, rest)), ...);
}

} // namespace detail

// ============================================================================
// Phase 2 : Format strings
// ============================================================================

// ---------------------------------------------------------------------------
// 2.1 — Détection de type string + parsing format
// ---------------------------------------------------------------------------

namespace detail {

// Trait : T est-il un type string ? (const char*, string, string_view)
template <typename T>
struct is_string_type : std::false_type {};

template <> struct is_string_type<const char*> : std::true_type {};
template <> struct is_string_type<char*> : std::true_type {};
template <> struct is_string_type<std::string> : std::true_type {};
template <> struct is_string_type<std::string_view> : std::true_type {};

// Matcher pour les char arrays (string literals "hello")
template <std::size_t N>
struct is_string_type<char[N]> : std::true_type {};
template <std::size_t N>
struct is_string_type<const char[N]> : std::true_type {};

template <typename T>
inline constexpr bool is_string_type_v = is_string_type<std::decay_t<T>>::value;

// Vérifie si un string_view contient des séquences format ({}, {{ ou }})
inline bool contains_format_sequences(std::string_view sv) {
    for (std::size_t i = 0; i + 1 < sv.size(); ++i) {
        if (sv[i] == '{' && (sv[i + 1] == '}' || sv[i + 1] == '{')) return true;
        if (sv[i] == '}' && sv[i + 1] == '}') return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// 2.2 — format_to(os, fmt, args...) — remplacement séquentiel des {}
// ---------------------------------------------------------------------------

// Écrire l'argument à l'index `idx` dans le pack
// Cas de base : index hors bornes → {?}
inline void write_arg_at(std::ostream& os, std::size_t) {
    os << "{?}";
}

template <typename T, typename... Rest>
inline void write_arg_at(std::ostream& os, std::size_t idx, const T& first, const Rest&... rest) {
    if (idx == 0) {
        print_one(os, first);
    } else {
        write_arg_at(os, idx - 1, rest...);
    }
}

template <typename... Args>
inline void format_to(std::ostream& os, std::string_view fmt, const Args&... args) {
    std::size_t arg_idx = 0;
    const std::size_t arg_count = sizeof...(Args);

    for (std::size_t i = 0; i < fmt.size(); ++i) {
        char c = fmt[i];

        if (c == '{') {
            if (i + 1 < fmt.size()) {
                if (fmt[i + 1] == '}') {
                    // {} → remplacer par l'argument courant
                    if (arg_idx < arg_count) {
                        write_arg_at(os, arg_idx, args...);
                    } else {
                        os << "{?}";
                    }
                    ++arg_idx;
                    ++i; // sauter le '}'
                    continue;
                }
                if (fmt[i + 1] == '{') {
                    // {{ → accolade littérale
                    os << '{';
                    ++i;
                    continue;
                }
            }
            os << c;
        } else if (c == '}') {
            if (i + 1 < fmt.size() && fmt[i + 1] == '}') {
                // }} → accolade littérale
                os << '}';
                ++i;
                continue;
            }
            os << c;
        } else {
            os << c;
        }
    }
}

} // namespace detail

// ---------------------------------------------------------------------------
// 2.3 — print / println — dispatch variadic vs format
// ---------------------------------------------------------------------------

// --- print / println vers stdout ---

inline void print() {
    // Zéro arguments → rien
}

// Un seul argument string : check pour {{ / }}
template <typename T>
inline auto print(const T& val)
    -> std::enable_if_t<detail::is_string_type_v<T>>
{
    std::string_view sv(val);
    if (detail::contains_format_sequences(sv)) {
        detail::format_to(std::cout, sv);
    } else {
        detail::print_one(std::cout, val);
    }
}

// Un seul argument non-string : variadic
template <typename T>
inline auto print(const T& val)
    -> std::enable_if_t<!detail::is_string_type_v<T>>
{
    detail::print_one(std::cout, val);
}

// Plusieurs arguments, premier est string → check format
template <typename First, typename... Rest>
inline auto print(const First& first, const Rest&... rest)
    -> std::enable_if_t<detail::is_string_type_v<First> && (sizeof...(Rest) > 0)>
{
    std::string_view sv(first);
    if (detail::contains_format_sequences(sv)) {
        detail::format_to(std::cout, sv, rest...);
    } else {
        detail::print_args(std::cout, first, rest...);
    }
}

// Plusieurs arguments, premier n'est PAS string → variadic
template <typename First, typename... Rest>
inline auto print(const First& first, const Rest&... rest)
    -> std::enable_if_t<!detail::is_string_type_v<First> && (sizeof...(Rest) > 0)>
{
    detail::print_args(std::cout, first, rest...);
}

inline void println() {
    std::cout << '\n';
}

// Un seul argument string : check pour {{ / }}
template <typename T>
inline auto println(const T& val)
    -> std::enable_if_t<detail::is_string_type_v<T>>
{
    std::string_view sv(val);
    if (detail::contains_format_sequences(sv)) {
        detail::format_to(std::cout, sv);
    } else {
        detail::print_one(std::cout, val);
    }
    std::cout << '\n';
}

// Un seul argument non-string
template <typename T>
inline auto println(const T& val)
    -> std::enable_if_t<!detail::is_string_type_v<T>>
{
    detail::print_one(std::cout, val);
    std::cout << '\n';
}

// Plusieurs arguments, premier est string → check format
template <typename First, typename... Rest>
inline auto println(const First& first, const Rest&... rest)
    -> std::enable_if_t<detail::is_string_type_v<First> && (sizeof...(Rest) > 0)>
{
    std::string_view sv(first);
    if (detail::contains_format_sequences(sv)) {
        detail::format_to(std::cout, sv, rest...);
    } else {
        detail::print_args(std::cout, first, rest...);
    }
    std::cout << '\n';
}

// Plusieurs arguments, premier n'est PAS string → variadic
template <typename First, typename... Rest>
inline auto println(const First& first, const Rest&... rest)
    -> std::enable_if_t<!detail::is_string_type_v<First> && (sizeof...(Rest) > 0)>
{
    detail::print_args(std::cout, first, rest...);
    std::cout << '\n';
}

// ============================================================================
// Phase 7 : Fonctionnalités complémentaires
// ============================================================================

// ---------------------------------------------------------------------------
// 7.1 — print_sep / println_sep — séparateur configurable
// ---------------------------------------------------------------------------

namespace detail {

template <typename Sep>
inline void print_args_sep(std::ostream&, const Sep&) {
    // Zéro arguments après le séparateur → rien
}

template <typename Sep, typename T>
inline void print_args_sep(std::ostream& os, const Sep&, const T& val) {
    print_one(os, val);
}

template <typename Sep, typename T, typename... Rest>
inline void print_args_sep(std::ostream& os, const Sep& sep, const T& first, const Rest&... rest) {
    print_one(os, first);
    ((os << sep, print_one(os, rest)), ...);
}

} // namespace detail

template <typename Sep, typename... Args>
inline void print_sep(const Sep& sep, const Args&... args) {
    detail::print_args_sep(std::cout, sep, args...);
}

template <typename Sep, typename... Args>
inline void println_sep(const Sep& sep, const Args&... args) {
    detail::print_args_sep(std::cout, sep, args...);
    std::cout << '\n';
}

// ---------------------------------------------------------------------------
// 7.2 — eprint / eprintln — sortie stderr
// ---------------------------------------------------------------------------

inline void eprint() {}

template <typename T>
inline auto eprint(const T& val)
    -> std::enable_if_t<detail::is_string_type_v<T>>
{
    std::string_view sv(val);
    if (detail::contains_format_sequences(sv)) {
        detail::format_to(std::cerr, sv);
    } else {
        detail::print_one(std::cerr, val);
    }
}

template <typename T>
inline auto eprint(const T& val)
    -> std::enable_if_t<!detail::is_string_type_v<T>>
{
    detail::print_one(std::cerr, val);
}

template <typename First, typename... Rest>
inline auto eprint(const First& first, const Rest&... rest)
    -> std::enable_if_t<detail::is_string_type_v<First> && (sizeof...(Rest) > 0)>
{
    std::string_view sv(first);
    if (detail::contains_format_sequences(sv)) {
        detail::format_to(std::cerr, sv, rest...);
    } else {
        detail::print_args(std::cerr, first, rest...);
    }
}

template <typename First, typename... Rest>
inline auto eprint(const First& first, const Rest&... rest)
    -> std::enable_if_t<!detail::is_string_type_v<First> && (sizeof...(Rest) > 0)>
{
    detail::print_args(std::cerr, first, rest...);
}

inline void eprintln() {
    std::cerr << '\n';
}

template <typename T>
inline auto eprintln(const T& val)
    -> std::enable_if_t<detail::is_string_type_v<T>>
{
    std::string_view sv(val);
    if (detail::contains_format_sequences(sv)) {
        detail::format_to(std::cerr, sv);
    } else {
        detail::print_one(std::cerr, val);
    }
    std::cerr << '\n';
}

template <typename T>
inline auto eprintln(const T& val)
    -> std::enable_if_t<!detail::is_string_type_v<T>>
{
    detail::print_one(std::cerr, val);
    std::cerr << '\n';
}

template <typename First, typename... Rest>
inline auto eprintln(const First& first, const Rest&... rest)
    -> std::enable_if_t<detail::is_string_type_v<First> && (sizeof...(Rest) > 0)>
{
    std::string_view sv(first);
    if (detail::contains_format_sequences(sv)) {
        detail::format_to(std::cerr, sv, rest...);
    } else {
        detail::print_args(std::cerr, first, rest...);
    }
    std::cerr << '\n';
}

template <typename First, typename... Rest>
inline auto eprintln(const First& first, const Rest&... rest)
    -> std::enable_if_t<!detail::is_string_type_v<First> && (sizeof...(Rest) > 0)>
{
    detail::print_args(std::cerr, first, rest...);
    std::cerr << '\n';
}

} // namespace prism

#endif // PRISM_HPP
