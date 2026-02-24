#include <prism/prism.hpp>
#include <cassert>
#include <sstream>
#include <string>
#include <cmath>

struct CaptureStdout {
    std::ostringstream buf;
    std::streambuf* old;
    CaptureStdout() : old(std::cout.rdbuf(buf.rdbuf())) {}
    ~CaptureStdout() { std::cout.rdbuf(old); }
    std::string str() const { return buf.str(); }
    void reset() { buf.str(""); buf.clear(); }
};

int main() {
    using namespace prism;
    enable_colors(true); // Forcer pour les tests (stdout redirigé)

    // --- 4.1 : ColorWrapper et StyledText ---
    {
        // StyledText via to_string_value
        StyledText st{"hello"};
        assert(to_string_value(st) == "hello");

        // ColorWrapper produit un StyledText avec ANSI codes
        ColorWrapper cw("\033[38;2;255;0;0m", "\033[0m");
        StyledText result = cw("test");
        assert(result.content == "\033[38;2;255;0;0mtest\033[0m");

        // ColorWrapper avec StyledText en entrée (emboîtement)
        StyledText inner = cw("inner");
        ColorWrapper cw2("\033[1m", "\033[0m"); // bold-like
        StyledText nested = cw2(inner);
        assert(nested.content == "\033[1m\033[38;2;255;0;0minner\033[0m\033[0m");
    }

    // --- 4.2 : rgb() ---
    {
        auto r = rgb(255, 0, 0);
        StyledText st = r("Error");
        assert(st.content == "\033[38;2;255;0;0mError\033[0m");

        // Inline
        StyledText st2 = rgb(0, 255, 0)("OK");
        assert(st2.content == "\033[38;2;0;255;0mOK\033[0m");

        // Clamp
        StyledText st3 = rgb(-10, 300, 128)("clamped");
        assert(st3.content == "\033[38;2;0;255;128mclamped\033[0m");

        // Réutilisation
        auto blue = rgb(0, 0, 255);
        assert(blue("a").content == "\033[38;2;0;0;255ma\033[0m");
        assert(blue("b").content == "\033[38;2;0;0;255mb\033[0m");
    }

    // --- 4.3 : hex() ---
    {
        // #RRGGBB
        StyledText st = hex("#ff0000")("red");
        assert(st.content == "\033[38;2;255;0;0mred\033[0m");

        // Sans #
        StyledText st2 = hex("00ff00")("green");
        assert(st2.content == "\033[38;2;0;255;0mgreen\033[0m");

        // #RGB raccourci
        StyledText st3 = hex("#f00")("r");
        assert(st3.content == "\033[38;2;255;0;0mr\033[0m");

        // #RGB → chaque digit * 17
        StyledText st4 = hex("#abc")("x");
        // a=10*17=170, b=11*17=187, c=12*17=204
        assert(st4.content == "\033[38;2;170;187;204mx\033[0m");

        // Majuscules
        StyledText st5 = hex("#FF6347")("tomato");
        assert(st5.content == "\033[38;2;255;99;71mtomato\033[0m");

        // Invalide → fallback blanc
        StyledText st6 = hex("xyz")("bad");
        assert(st6.content == "\033[38;2;255;255;255mbad\033[0m");
    }

    // --- 4.4 : hsl() ---
    {
        // Rouge pur : hsl(0, 100, 50) → rgb(255, 0, 0)
        StyledText st = hsl(0, 100, 50)("red");
        assert(st.content == "\033[38;2;255;0;0mred\033[0m");

        // Vert : hsl(120, 100, 50) → rgb(0, 255, 0)
        StyledText st2 = hsl(120, 100, 50)("green");
        assert(st2.content == "\033[38;2;0;255;0mgreen\033[0m");

        // Bleu : hsl(240, 100, 50) → rgb(0, 0, 255)
        StyledText st3 = hsl(240, 100, 50)("blue");
        assert(st3.content == "\033[38;2;0;0;255mblue\033[0m");

        // Blanc : hsl(0, 0, 100) → rgb(255, 255, 255)
        StyledText st4 = hsl(0, 0, 100)("white");
        assert(st4.content == "\033[38;2;255;255;255mwhite\033[0m");

        // Noir : hsl(0, 0, 0) → rgb(0, 0, 0)
        StyledText st5 = hsl(0, 0, 0)("black");
        assert(st5.content == "\033[38;2;0;0;0mblack\033[0m");

        // Gris : hsl(0, 0, 50) → rgb(128, 128, 128)
        StyledText st6 = hsl(0, 0, 50)("gray");
        assert(st6.content == "\033[38;2;128;128;128mgray\033[0m");
    }

    // --- 4.5 : oklch() ---
    {
        // Noir : oklch(0, 0, 0) → rgb(0, 0, 0)
        StyledText st = oklch(0, 0, 0)("black");
        assert(st.content == "\033[38;2;0;0;0mblack\033[0m");

        // Blanc : oklch(1, 0, 0) → rgb(255, 255, 255)
        StyledText st2 = oklch(1, 0, 0)("white");
        assert(st2.content == "\033[38;2;255;255;255mwhite\033[0m");

        // Un rouge-ish : oklch(0.6, 0.25, 30) → doit donner des valeurs raisonnables
        auto [r, g, b] = detail::oklch_to_rgb(0.6, 0.25, 30.0);
        assert(r >= 0 && r <= 255);
        assert(g >= 0 && g <= 255);
        assert(b >= 0 && b <= 255);
        // Le rouge devrait dominer pour H=30
        assert(r > g && r > b);

        // Clamp hors bornes
        auto [r2, g2, b2] = detail::oklch_to_rgb(2.0, 0.5, -30.0);
        assert(r2 >= 0 && r2 <= 255);
        assert(g2 >= 0 && g2 <= 255);
        assert(b2 >= 0 && b2 <= 255);
    }

    // --- 4.6 : Intégration avec print ---
    {
        CaptureStdout cap;

        // print avec couleur
        println(rgb(255, 0, 0)("Error:"), "something broke");
        std::string expected = "\033[38;2;255;0;0mError:\033[0m something broke\n";
        assert(cap.str() == expected);
        cap.reset();

        // Format string avec couleur
        println("{} {}", hex("#0f0")("OK"), "done");
        expected = "\033[38;2;0;255;0mOK\033[0m done\n";
        assert(cap.str() == expected);
        cap.reset();

        // Emboîtement dans print
        auto r = rgb(255, 0, 0);
        print(r("nested"));
        assert(cap.str() == "\033[38;2;255;0;0mnested\033[0m");
        cap.reset();

        // ColorWrapper avec int
        print(rgb(0, 0, 255)(42));
        assert(cap.str() == "\033[38;2;0;0;255m42\033[0m");
        cap.reset();

        // ColorWrapper avec bool
        print(rgb(0, 255, 0)(true));
        assert(cap.str() == "\033[38;2;0;255;0mtrue\033[0m");
        cap.reset();
    }

    std::cout << "All phase 4 tests passed!" << std::endl;
    return 0;
}
