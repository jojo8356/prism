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

    // --- 5.1 : Background colors ---
    {
        // bg_rgb
        StyledText st = bg_rgb(255, 255, 0)("highlight");
        assert(st.content == "\033[48;2;255;255;0mhighlight\033[0m");

        // bg_rgb clamp
        StyledText st2 = bg_rgb(-1, 256, 128)("clamped");
        assert(st2.content == "\033[48;2;0;255;128mclamped\033[0m");

        // bg_hex #RRGGBB
        StyledText st3 = bg_hex("#ff0000")("red bg");
        assert(st3.content == "\033[48;2;255;0;0mred bg\033[0m");

        // bg_hex #RGB
        StyledText st4 = bg_hex("#0f0")("green bg");
        assert(st4.content == "\033[48;2;0;255;0mgreen bg\033[0m");

        // bg_hsl — bleu pur
        StyledText st5 = bg_hsl(240, 100, 50)("blue bg");
        assert(st5.content == "\033[48;2;0;0;255mblue bg\033[0m");

        // bg_oklch — noir
        StyledText st6 = bg_oklch(0, 0, 0)("black bg");
        assert(st6.content == "\033[48;2;0;0;0mblack bg\033[0m");
    }

    // --- 5.2 : Styles de texte ---
    {
        // bold
        StyledText st = bold("gras");
        assert(st.content == "\033[1mgras\033[22m");

        // dim
        StyledText st2 = dim("faible");
        assert(st2.content == "\033[2mfaible\033[22m");

        // italic
        StyledText st3 = italic("italique");
        assert(st3.content == "\033[3mitalique\033[23m");

        // underline
        StyledText st4 = underline("souligné");
        assert(st4.content == "\033[4msouligné\033[24m");

        // strikethrough
        StyledText st5 = strikethrough("barré");
        assert(st5.content == "\033[9mbarré\033[29m");

        // Styles avec types non-string
        StyledText st6 = bold(42);
        assert(st6.content == "\033[1m42\033[22m");

        StyledText st7 = italic(true);
        assert(st7.content == "\033[3mtrue\033[23m");
    }

    // --- 5.3 : Composition de styles ---
    {
        // bold(rgb(...)("text")) — style wrappant couleur
        StyledText st = bold(rgb(255, 0, 0)("Error"));
        assert(st.content == "\033[1m\033[38;2;255;0;0mError\033[0m\033[22m");

        // rgb(...)(bold("text")) — couleur wrappant style
        StyledText st2 = rgb(0, 255, 0)(bold("OK"));
        assert(st2.content == "\033[38;2;0;255;0m\033[1mOK\033[22m\033[0m");

        // Triple emboîtement
        StyledText st3 = bg_rgb(0, 0, 0)(bold(rgb(0, 255, 0)("Matrix")));
        assert(st3.content ==
            "\033[48;2;0;0;0m"     // bg noir
            "\033[1m"              // bold
            "\033[38;2;0;255;0m"   // fg vert
            "Matrix"
            "\033[0m"              // reset fg
            "\033[22m"             // reset bold
            "\033[0m"              // reset bg
        );

        // italic(underline("text"))
        StyledText st4 = italic(underline("fancy"));
        assert(st4.content == "\033[3m\033[4mfancy\033[24m\033[23m");

        // bold + dim (même catégorie de reset)
        StyledText st5 = bold(dim("both"));
        assert(st5.content == "\033[1m\033[2mboth\033[22m\033[22m");
    }

    // --- Intégration avec print ---
    {
        CaptureStdout cap;

        // print avec style
        println(bold("Title:"), "content");
        assert(cap.str() == "\033[1mTitle:\033[22m content\n");
        cap.reset();

        // Format string avec styles
        println("{} is {}", bold("Alice"), italic("cool"));
        assert(cap.str() == "\033[1mAlice\033[22m is \033[3mcool\033[23m\n");
        cap.reset();

        // Combinaison couleur + style + bg dans print
        print(bg_hex("#333")(bold(hex("#0ff")("cyber"))));
        std::string expected =
            "\033[48;2;51;51;51m"    // bg #333
            "\033[1m"                // bold
            "\033[38;2;0;255;255m"   // fg cyan
            "cyber"
            "\033[0m"               // reset fg
            "\033[22m"              // reset bold
            "\033[0m";              // reset bg
        assert(cap.str() == expected);
        cap.reset();

        // fg + bg côte à côte
        println(rgb(255, 0, 0)("red"), bg_rgb(255, 255, 0)("yellow bg"));
        expected =
            "\033[38;2;255;0;0mred\033[0m"
            " "
            "\033[48;2;255;255;0myellow bg\033[0m"
            "\n";
        assert(cap.str() == expected);
        cap.reset();
    }

    std::cout << "All phase 5 tests passed!" << std::endl;
    return 0;
}
