# Prism

Biblioth&egrave;que C++17 header-only qui remplace `std::cout` par un `print()` universel avec couleurs.

```cpp
#include <prism/prism.hpp>
using namespace prism;

int main() {
    println("Hello,", "World!");                          // Hello, World!
    println("{} scored {}/{}", "Alice", 42, 100);         // Alice scored 42/100
    println(red("ERROR"), "something broke");             // ERROR (en rouge) something broke
    println("data:", std::vector<int>{1, 2, 3});          // data: [1, 2, 3]
}
```

## Installation

Header-only, zero dependance. Plusieurs options :

### CMake FetchContent (recommande)

```cmake
include(FetchContent)
FetchContent_Declare(
    prism
    GIT_REPOSITORY https://github.com/jojo8356/prism.git
    GIT_TAG        v0.1.0
)
FetchContent_MakeAvailable(prism)

target_link_libraries(your_target PRIVATE prism::prism)
```

### vcpkg

```bash
vcpkg install prism
```

```cmake
find_package(prism CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE prism::prism)
```

### Copie manuelle

```bash
cp -r include/prism/ your_project/include/
g++ -std=c++17 -I include main.cpp -o main
```

## API

### print / println

```cpp
// Variadic — separateur espace automatique
print("a", 42, true);          // a 42 true
println("line", 1);            // line 1\n

// Format string — detection auto quand le premier arg contient {}
println("{} = {}", "x", 10);   // x = 10
println("{{escaped}}");        // {escaped}

// Separateur custom
println_sep(" | ", 1, 2, 3);   // 1 | 2 | 3
println_sep(", ", "a", "b");   // a, b

// Stderr
eprintln("error:", msg);
eprint("{} failed", cmd);
```

### Types supportes

Tous les types sont affiches nativement sans configuration :

```cpp
// Primitifs
println(42, 3.14, true, 'A', nullptr);   // 42 3.14 true A nullptr

// Containers STL
println(std::vector<int>{1, 2, 3});                      // [1, 2, 3]
println(std::map<std::string, int>{{"a", 1}, {"b", 2}}); // {a: 1, b: 2}
println(std::set<int>{3, 1, 2});                         // {1, 2, 3}
println(std::make_pair(1, "hello"));                      // (1, hello)
println(std::make_tuple(1, "x", 3.14));                   // (1, x, 3.14)
println(std::optional<int>(42));                          // Some(42)
println(std::optional<int>());                            // None

// Imbrique
println(std::vector<std::vector<int>>{{1, 2}, {3, 4}});  // [[1, 2], [3, 4]]
```

### Types custom

Deux options pour rendre un type printable :

```cpp
struct Point { int x, y; };

// Option A : operator<< (compatible existant)
std::ostream& operator<<(std::ostream& os, const Point& p) {
    return os << "(" << p.x << ", " << p.y << ")";
}

// Option B : specialiser Formatter (prioritaire)
template<>
struct prism::Formatter<Point> {
    static std::string format(const Point& p) {
        return "(" + std::to_string(p.x) + ", " + std::to_string(p.y) + ")";
    }
};

println(Point{3, 4});  // (3, 4)
```

Un type sans `operator<<` ni `Formatter` produit une erreur de compilation claire.

## Couleurs

### 4 espaces colorimetriques

```cpp
rgb(255, 0, 0)("texte")           // RGB direct
hex("#ff6347")("texte")            // Hexadecimal (#RRGGBB, #RGB, sans #)
hsl(270, 100, 50)("texte")        // HSL (h:0-360, s:0-100, l:0-100)
oklch(0.7, 0.15, 150)("texte")    // OKLCH (L:0-1, C:0-0.4, H:0-360)
```

### 148 couleurs CSS nommees

Chaque couleur CSS est une fonction directe :

```cpp
println(red("stop"), green("go"), yellow("caution"));
println(coral("warm"), dodgerblue("cool"), orchid("fancy"));
println(crimson("alert"), gold("star"), tomato("hot"));
```

<details>
<summary>Liste complete des 148 couleurs</summary>

`aliceblue` `antiquewhite` `aqua` `aquamarine` `azure` `beige` `bisque` `black` `blanchedalmond` `blue` `blueviolet` `brown` `burlywood` `cadetblue` `chartreuse` `chocolate` `coral` `cornflowerblue` `cornsilk` `crimson` `cyan` `darkblue` `darkcyan` `darkgoldenrod` `darkgray` `darkgreen` `darkgrey` `darkkhaki` `darkmagenta` `darkolivegreen` `darkorange` `darkorchid` `darkred` `darksalmon` `darkseagreen` `darkslateblue` `darkslategray` `darkslategrey` `darkturquoise` `darkviolet` `deeppink` `deepskyblue` `dimgray` `dimgrey` `dodgerblue` `firebrick` `floralwhite` `forestgreen` `fuchsia` `gainsboro` `ghostwhite` `gold` `goldenrod` `gray` `green` `greenyellow` `grey` `honeydew` `hotpink` `indianred` `indigo` `ivory` `khaki` `lavender` `lavenderblush` `lawngreen` `lemonchiffon` `lightblue` `lightcoral` `lightcyan` `lightgoldenrodyellow` `lightgray` `lightgreen` `lightgrey` `lightpink` `lightsalmon` `lightseagreen` `lightskyblue` `lightslategray` `lightslategrey` `lightsteelblue` `lightyellow` `lime` `limegreen` `linen` `magenta` `maroon` `mediumaquamarine` `mediumblue` `mediumorchid` `mediumpurple` `mediumseagreen` `mediumslateblue` `mediumspringgreen` `mediumturquoise` `mediumvioletred` `midnightblue` `mintcream` `mistyrose` `moccasin` `navajowhite` `navy` `oldlace` `olive` `olivedrab` `orange` `orangered` `orchid` `palegoldenrod` `palegreen` `paleturquoise` `palevioletred` `papayawhip` `peachpuff` `peru` `pink` `plum` `powderblue` `purple` `rebeccapurple` `red` `rosybrown` `royalblue` `saddlebrown` `salmon` `sandybrown` `seagreen` `seashell` `sienna` `silver` `skyblue` `slateblue` `slategray` `slategrey` `snow` `springgreen` `steelblue` `tan` `teal` `thistle` `tomato` `turquoise` `violet` `wheat` `white` `whitesmoke` `yellow` `yellowgreen`

</details>

### Lookup dynamique

```cpp
auto theme = css("steelblue");     // par nom string
println(theme("styled text"));

auto bg = bg_css("papayawhip");    // background par nom
println(bg("warm background"));
```

### Background

```cpp
bg_rgb(255, 255, 0)("highlight")
bg_hex("#333")(white("dark bg"))
bg_hsl(120, 100, 50)("green bg")

// Fonctions CSS : bg_red(), bg_blue(), bg_gold(), ...
println(bg_red(white(" ALERT ")), bg_green(black(" OK ")));
```

### Styles de texte

```cpp
bold("gras")
dim("attenue")
italic("italique")
underline("souligne")
strikethrough("barre")
```

### Composition libre

Les styles et couleurs se composent par emboitement :

```cpp
println(bold(red("ERREUR:")), "fichier introuvable");
println(italic(dodgerblue("Note:")), "ceci est une info");
println(bg_black(bold(green(">>> "))), "commande executee");

// Dans un format string
println("{} a obtenu {}", bold("Alice"), gold("95/100"));
```

## Detection automatique des couleurs

Les codes ANSI sont desactives automatiquement quand :
- stdout n'est pas un TTY (pipe, redirection fichier)
- La variable `NO_COLOR` est definie ([no-color.org](https://no-color.org))
- `TERM=dumb`

Forcer manuellement :

```cpp
prism::enable_colors(true);    // toujours ON
prism::enable_colors(false);   // toujours OFF
prism::reset_colors();         // revenir en auto-detection
```

## Specifications

| | |
|---|---|
| Standard | C++17 |
| Format | Header-only (un seul `.hpp`) |
| Dependances | Zero (STL uniquement) |
| Couleurs | ANSI truecolor 24-bit |
| Plateformes | Linux, macOS, Windows 10+ |

## Compiler les tests

```bash
# Tests unitaires (73 tests)
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -I include tests/test_prism.cpp -o test_prism
./test_prism

# Demo interactive
g++ -std=c++17 -I include examples/demo.cpp -o demo
./demo
```

## Licence

MIT
