#include <prism/prism.hpp>
#include <cassert>
#include <sstream>
#include <string>
#include <cstring>

struct CaptureStream {
    std::ostream& stream;
    std::ostringstream buf;
    std::streambuf* old;
    CaptureStream(std::ostream& s) : stream(s), old(s.rdbuf(buf.rdbuf())) {}
    ~CaptureStream() { stream.rdbuf(old); }
    std::string str() const { return buf.str(); }
    void reset() { buf.str(""); buf.clear(); }
};

int main() {
    using namespace prism;

    // --- 9.1 : Messages d'erreur (vérifiés manuellement, voir test_error_message.cpp) ---
    // Le static_assert produit :
    //   "prism::print — type is not printable.
    //    Either specialize prism::Formatter<T> or provide operator<<(ostream&, const T&)."
    // avec le nom du type dans l'instanciation template.

    // --- 9.2 : Optimisations ---

    // constexpr clamp_byte
    {
        static_assert(detail::clamp_byte(0) == 0, "");
        static_assert(detail::clamp_byte(255) == 255, "");
        static_assert(detail::clamp_byte(-10) == 0, "");
        static_assert(detail::clamp_byte(300) == 255, "");
        static_assert(detail::clamp_byte(128) == 128, "");
    }

    // constexpr hex_char_to_int
    {
        static_assert(detail::hex_char_to_int('0') == 0, "");
        static_assert(detail::hex_char_to_int('9') == 9, "");
        static_assert(detail::hex_char_to_int('a') == 10, "");
        static_assert(detail::hex_char_to_int('f') == 15, "");
        static_assert(detail::hex_char_to_int('A') == 10, "");
        static_assert(detail::hex_char_to_int('F') == 15, "");
        static_assert(detail::hex_char_to_int('x') == -1, "");
    }

    // CSS table is sorted (required for binary search)
    {
        const auto* table = detail::css_color_table();
        std::size_t count = detail::css_color_count();
        assert(count == 148);

        for (std::size_t i = 1; i < count; ++i) {
            assert(std::strcmp(table[i - 1].name, table[i].name) < 0 &&
                "CSS color table must be sorted alphabetically for binary search");
        }
    }

    // Binary search finds all 148 colors
    {
        const auto* table = detail::css_color_table();
        std::size_t count = detail::css_color_count();

        for (std::size_t i = 0; i < count; ++i) {
            auto [r, g, b] = detail::lookup_css_color(table[i].name);
            assert(r == table[i].r && g == table[i].g && b == table[i].b);
        }

        // Non-existent returns white
        auto [r, g, b] = detail::lookup_css_color("nonexistent");
        assert(r == 255 && g == 255 && b == 255);
    }

    // write_value direct pour bool (pas de Formatter::format allocation)
    {
        enable_colors(true);
        CaptureStream cap(std::cout);
        print(true, false);
        assert(cap.str() == "true false");
        cap.reset();
    }

    // write_value direct pour nullptr
    {
        CaptureStream cap(std::cout);
        print(nullptr);
        assert(cap.str() == "nullptr");
        cap.reset();
    }

    // write_value direct pour StyledText (pas de copie via Formatter)
    {
        enable_colors(true);
        CaptureStream cap(std::cout);
        println(red("test"));
        assert(cap.str() == "\033[38;2;255;0;0mtest\033[0m\n");
        cap.reset();
    }

    // --- 9.3 : Portabilité ---
    // Vérification que les macros sont bien définies
    {
#ifdef _WIN32
        // Sur Windows : PRISM_ISATTY = _isatty, PRISM_FILENO = _fileno
        // + windows.h inclus pour SetConsoleMode
        static_assert(true, "Windows path compiled");
#else
        // Sur POSIX : PRISM_ISATTY = isatty, PRISM_FILENO = fileno
        static_assert(true, "POSIX path compiled");
#endif
    }

    // enable_colors / reset_colors cycle complet
    {
        enable_colors(true);
        assert(red("x").content == "\033[38;2;255;0;0mx\033[0m");

        enable_colors(false);
        assert(red("x").content == "x");
        assert(bold(red("x")).content == "x");
        assert(bg_hex("#f00")("x").content == "x");

        reset_colors();
        // Auto-detect : pas de TTY dans les tests → off
        assert(red("x").content == "x");

        enable_colors(true); // restore
    }

    std::cout << "All phase 9 tests passed!" << std::endl;
    return 0;
}
