#include <prism/prism.hpp>
#include <vector>
#include <map>
#include <set>
#include <optional>
#include <tuple>
#include <variant>

using namespace prism;

int main() {
    enable_colors(true);

    // =========================================================================
    // 1. Print basique — remplacer std::cout
    // =========================================================================

    println("=== Print basique ===");
    println("Hello, World!");
    println("Score:", 42, "sur", 100, "->", 0.42);
    println("Bool:", true, "/ Null:", nullptr);
    println();

    // =========================================================================
    // 2. Format strings — style Python / fmt
    // =========================================================================

    println("=== Format strings ===");
    println("{} scored {}/{}", "Alice", 42, 100);
    println("{} + {} = {}", 1, 2, 3);
    println("Escaped: {{braces}}");
    println("Missing args: {} {} {}", "only one");
    println();

    // =========================================================================
    // 3. Séparateur configurable
    // =========================================================================

    println("=== Séparateurs ===");
    println_sep(", ", "apple", "banana", "cherry");
    println_sep(" | ", 10, 20, 30, 40);
    println_sep(" -> ", "start", "middle", "end");
    println();

    // =========================================================================
    // 4. Types STL
    // =========================================================================

    println("=== Types STL ===");

    std::vector<int> vec = {1, 2, 3, 4, 5};
    println("vector:", vec);

    std::map<std::string, int> scores = {{"Alice", 95}, {"Bob", 87}};
    println("map:", scores);

    std::set<int> unique = {3, 1, 4, 1, 5};
    println("set:", unique);

    auto pair = std::make_pair("key", 42);
    println("pair:", pair);

    auto tup = std::make_tuple(1, "hello", 3.14, true);
    println("tuple:", tup);

    std::optional<int> some = 42;
    std::optional<int> none;
    println("optional:", some, "/", none);

    std::variant<int, std::string> var = std::string("variant!");
    println("variant:", var);

    // Imbriqué
    std::vector<std::vector<int>> matrix = {{1, 2}, {3, 4}, {5, 6}};
    println("nested:", matrix);
    println();

    // =========================================================================
    // 5. Couleurs RGB / Hex / HSL / OKLCH
    // =========================================================================

    println("=== Couleurs ===");
    println(rgb(255, 0, 0)("Rouge RGB"));
    println(hex("#00ff00")("Vert Hex"));
    println(hex("#00f")("Bleu Hex court"));
    println(hsl(270, 100, 50)("Violet HSL"));
    println(oklch(0.7, 0.15, 150)("Vert OKLCH"));
    println();

    // =========================================================================
    // 6. Couleurs CSS nommées
    // =========================================================================

    println("=== 148 Couleurs CSS ===");
    println(red("red"), orange("orange"), yellow("yellow"),
            green("green"), blue("blue"), indigo("indigo"), violet("violet"));
    println(coral("coral"), tomato("tomato"), gold("gold"),
            dodgerblue("dodgerblue"), orchid("orchid"), crimson("crimson"));
    println(lightseagreen("lightseagreen"), mediumpurple("mediumpurple"),
            rebeccapurple("rebeccapurple"), darkturquoise("darkturquoise"));
    println();

    // =========================================================================
    // 7. Background
    // =========================================================================

    println("=== Background ===");
    println(bg_rgb(255, 255, 0)(black(" Highlight jaune ")));
    println(bg_hex("#333")(white(" Dark background ")));
    println(bg_red(white(" ALERTE ")), bg_green(black(" OK ")), bg_blue(white(" INFO ")));
    println();

    // =========================================================================
    // 8. Styles de texte
    // =========================================================================

    println("=== Styles ===");
    println(bold("Gras"), dim("Dim"), italic("Italique"),
            underline("Souligné"), strikethrough("Barré"));
    println();

    // =========================================================================
    // 9. Composition — le vrai pouvoir
    // =========================================================================

    println("=== Compositions ===");
    println(bold(red("ERREUR:")), "Le fichier n'existe pas");
    println(italic(dodgerblue("Note:")), "Ceci est une info");
    println(bg_black(bold(green(">>> "))), "Commande exécutée");
    println(underline(hex("#ff6347")("Lien:")), "https://example.com");

    // Couleur dans un format string
    println("{} a obtenu {} en {}", bold("Alice"), gold("95/100"), italic(green("maths")));
    println();

    // =========================================================================
    // 10. Couleurs dynamiques (css lookup)
    // =========================================================================

    println("=== CSS dynamique ===");
    auto theme = css("steelblue");
    println(theme("Texte en steelblue"));
    println(bg_css("papayawhip")(css("saddlebrown")("Thème chaleureux")));
    println();

    // =========================================================================
    // 11. Stderr
    // =========================================================================

    eprintln(bold(red("[STDERR]")), "Ce message va vers stderr");

    // =========================================================================
    // 12. enable_colors toggle
    // =========================================================================

    println("=== Toggle couleurs ===");
    println(red("Avec couleurs"));
    enable_colors(false);
    println(red("Sans couleurs (enable_colors(false))"));
    enable_colors(true);
    println(red("Couleurs re-activées"));

    println();
    println(bold(green("Demo terminée !")));

    return 0;
}
