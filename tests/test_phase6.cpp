#include <prism/prism.hpp>
#include <cassert>
#include <sstream>
#include <string>

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
    enable_colors(true);

    // --- 6.2 : Fonctions CSS foreground ---
    {
        // Couleurs primaires
        assert(red("x").content == "\033[38;2;255;0;0mx\033[0m");
        assert(green("x").content == "\033[38;2;0;128;0mx\033[0m");
        assert(blue("x").content == "\033[38;2;0;0;255mx\033[0m");

        // Couleurs secondaires
        assert(yellow("x").content == "\033[38;2;255;255;0mx\033[0m");
        assert(cyan("x").content == "\033[38;2;0;255;255mx\033[0m");
        assert(magenta("x").content == "\033[38;2;255;0;255mx\033[0m");

        // Couleurs nommées spécifiques
        assert(coral("hot").content == "\033[38;2;255;127;80mhot\033[0m");
        assert(tomato("t").content == "\033[38;2;255;99;71mt\033[0m");
        assert(dodgerblue("d").content == "\033[38;2;30;144;255md\033[0m");
        assert(gold("g").content == "\033[38;2;255;215;0mg\033[0m");
        assert(orchid("o").content == "\033[38;2;218;112;214mo\033[0m");

        // Noir et blanc
        assert(black("b").content == "\033[38;2;0;0;0mb\033[0m");
        assert(white("w").content == "\033[38;2;255;255;255mw\033[0m");

        // Couleurs longues
        assert(lightgoldenrodyellow("l").content == "\033[38;2;250;250;210ml\033[0m");
        assert(mediumspringgreen("m").content == "\033[38;2;0;250;154mm\033[0m");
        assert(rebeccapurple("r").content == "\033[38;2;102;51;153mr\033[0m");

        // Avec types non-string
        assert(red(42).content == "\033[38;2;255;0;0m42\033[0m");
        assert(blue(true).content == "\033[38;2;0;0;255mtrue\033[0m");
    }

    // --- 6.3 : Fonctions CSS background ---
    {
        assert(bg_red("x").content == "\033[48;2;255;0;0mx\033[0m");
        assert(bg_blue("x").content == "\033[48;2;0;0;255mx\033[0m");
        assert(bg_yellow("x").content == "\033[48;2;255;255;0mx\033[0m");
        assert(bg_coral("x").content == "\033[48;2;255;127;80mx\033[0m");
        assert(bg_black("x").content == "\033[48;2;0;0;0mx\033[0m");
        assert(bg_white("x").content == "\033[48;2;255;255;255mx\033[0m");
    }

    // --- 6.4 : css() lookup dynamique ---
    {
        // Lookup par nom
        assert(css("red")("x").content == "\033[38;2;255;0;0mx\033[0m");
        assert(css("dodgerblue")("x").content == "\033[38;2;30;144;255mx\033[0m");
        assert(css("coral")("x").content == "\033[38;2;255;127;80mx\033[0m");
        assert(css("rebeccapurple")("x").content == "\033[38;2;102;51;153mx\033[0m");

        // bg_css()
        assert(bg_css("red")("x").content == "\033[48;2;255;0;0mx\033[0m");
        assert(bg_css("gold")("x").content == "\033[48;2;255;215;0mx\033[0m");

        // Nom invalide → fallback blanc
        assert(css("nonexistent")("x").content == "\033[38;2;255;255;255mx\033[0m");

        // Réutilisation du ColorWrapper retourné par css()
        auto c = css("tomato");
        assert(c("a").content == "\033[38;2;255;99;71ma\033[0m");
        assert(c("b").content == "\033[38;2;255;99;71mb\033[0m");
    }

    // --- Composition CSS + styles ---
    {
        // bold(red("text"))
        StyledText st = bold(red("Error"));
        assert(st.content == "\033[1m\033[38;2;255;0;0mError\033[0m\033[22m");

        // bg_gold(bold(navy("text")))
        StyledText st2 = bg_gold(bold(navy("Admiral")));
        assert(st2.content ==
            "\033[48;2;255;215;0m"     // bg gold
            "\033[1m"                  // bold
            "\033[38;2;0;0;128m"       // fg navy
            "Admiral"
            "\033[0m"                 // reset fg
            "\033[22m"               // reset bold
            "\033[0m"                // reset bg
        );
    }

    // --- Intégration print ---
    {
        CaptureStdout cap;

        println(red("stop"), green("go"), yellow("caution"));
        std::string expected =
            "\033[38;2;255;0;0mstop\033[0m"
            " "
            "\033[38;2;0;128;0mgo\033[0m"
            " "
            "\033[38;2;255;255;0mcaution\033[0m"
            "\n";
        assert(cap.str() == expected);
        cap.reset();

        // Format string avec CSS colors
        println("{} is {}", coral("warm"), dodgerblue("cool"));
        expected =
            "\033[38;2;255;127;80mwarm\033[0m"
            " is "
            "\033[38;2;30;144;255mcool\033[0m"
            "\n";
        assert(cap.str() == expected);
        cap.reset();
    }

    std::cout << "All phase 6 tests passed!" << std::endl;
    return 0;
}
