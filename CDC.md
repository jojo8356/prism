# Prism — Cahier des Charges

> Bibliothèque C++17 header-only de print universel avec couleurs.

---

## 1. Objectif

Remplacer l'usage verbeux de `std::cout` / `std::cerr` par une API minimaliste et expressive :

```cpp
// Avant (std::cout)
std::cout << "x = " << x << ", y = " << y << std::endl;

// Après (prism)
println("x =", x, ", y =", y);
println(red("Error:"), msg);
println(hex("#ff6347")("Status:"), rgb(0, 200, 100)("OK"));
```

---

## 2. Contraintes techniques

| Contrainte        | Valeur                            |
| ----------------- | --------------------------------- |
| Standard          | C++17 minimum                     |
| Format            | Header-only (un seul `.hpp`)      |
| Dépendances       | Zéro (STL uniquement)             |
| Couleurs          | ANSI escape codes (truecolor 24-bit) |
| Thread-safety     | Non requis (v1)                   |
| Plateformes       | Linux, macOS, Windows (ANSI activé) |

---

## 3. API Print

### 3.1 Mode variadic (séparateur automatique par espace)

```cpp
print(args...);        // sans retour à la ligne
println(args...);      // avec retour à la ligne
```

```cpp
print("Hello", 42, 3.14);       // -> Hello 42 3.14
println("Name:", name);          // -> Name: John\n
print();                         // -> (rien)
println();                       // -> \n
```

### 3.2 Mode format string (placeholders `{}`)

Quand le premier argument est un string contenant `{}`, le mode format s'active :

```cpp
print("x = {}, y = {}", x, y);      // -> x = 10, y = 20
println("{} + {} = {}", 1, 2, 3);    // -> 1 + 2 = 3\n
```

Règles :
- `{}` est remplacé séquentiellement par chaque argument suivant
- Nombre de `{}` doit correspondre au nombre d'arguments restants (sinon erreur à la compilation si possible, sinon runtime)
- `{{` et `}}` pour échapper les accolades littérales : `print("{{}}");` -> `{}`

### 3.3 Séparateur configurable

```cpp
print_sep(",", "a", "b", "c");      // -> a,b,c
println_sep(" | ", 1, 2, 3);        // -> 1 | 2 | 3\n
```

### 3.4 Sortie d'erreur

```cpp
eprint(args...);       // print vers stderr
eprintln(args...);     // println vers stderr
```

---

## 4. Types supportés nativement

### 4.1 Primitifs

| Type                  | Affichage           |
| --------------------- | ------------------- |
| `int`, `long`, etc.   | `42`                |
| `float`, `double`     | `3.14`              |
| `bool`                | `true` / `false`    |
| `char`                | `A`                 |
| `const char*`         | `hello`             |
| `std::string`         | `hello`             |
| `std::string_view`    | `hello`             |
| `nullptr_t`           | `nullptr`           |
| Pointeurs             | `0x7fff5fbff8ac`    |

### 4.2 Containers STL

| Type                          | Affichage                    |
| ----------------------------- | ---------------------------- |
| `std::vector<T>`              | `[1, 2, 3]`                 |
| `std::array<T, N>`            | `[1, 2, 3]`                 |
| `std::list<T>`                | `[1, 2, 3]`                 |
| `std::set<T>`                 | `{1, 2, 3}`                 |
| `std::unordered_set<T>`       | `{1, 2, 3}`                 |
| `std::map<K, V>`              | `{a: 1, b: 2}`              |
| `std::unordered_map<K, V>`    | `{a: 1, b: 2}`              |
| `std::pair<A, B>`             | `(1, "hello")`              |
| `std::tuple<Ts...>`           | `(1, "hello", 3.14)`        |
| `std::optional<T>`            | `Some(42)` ou `None`        |
| `std::variant<Ts...>`         | La valeur active             |

### 4.3 Trait custom : `Printable`

Pour rendre un type custom printable, deux options :

**Option A** — Surcharger `operator<<` (compatible existant) :
```cpp
struct Point { int x, y; };

std::ostream& operator<<(std::ostream& os, const Point& p) {
    return os << "(" << p.x << ", " << p.y << ")";
}

println(Point{3, 4});  // -> (3, 4)
```

**Option B** — Spécialiser le trait `prism::Formatter` :
```cpp
template<>
struct prism::Formatter<Point> {
    static std::string format(const Point& p) {
        return "(" + std::to_string(p.x) + ", " + std::to_string(p.y) + ")";
    }
};

println(Point{3, 4});  // -> (3, 4)
```

Priorité : `Formatter<T>` > `operator<<` > erreur de compilation.

---

## 5. Système de couleurs

### 5.1 Architecture

Une couleur produit un **wrapper callable** qui colore le texte passé en argument :

```cpp
auto r = rgb(255, 0, 0);
println(r("Error!"));           // "Error!" en rouge

println(hex("#ff6347")("Hot")); // inline
```

Le wrapper ajoute `\033[38;2;R;G;Bm` avant et `\033[0m` après le texte.

### 5.2 Espaces colorimétriques supportés

#### RGB (direct)
```cpp
rgb(255, 0, 0)("text")        // r, g, b : 0-255
```

#### Hexadécimal
```cpp
hex("#ff0000")("text")        // #RRGGBB
hex("#f00")("text")           // #RGB (raccourci)
hex("ff0000")("text")         // sans #
```

#### HSL
```cpp
hsl(0, 100, 50)("text")      // h: 0-360, s: 0-100, l: 0-100
```

#### OKLCH
```cpp
oklch(0.7, 0.15, 30)("text") // L: 0-1, C: 0-0.4, H: 0-360
```

### 5.3 Background (couleur de fond)

```cpp
bg_rgb(255, 255, 0)("text")       // fond jaune
bg_hex("#ffff00")("text")
bg_hsl(60, 100, 50)("text")
```

Utilise `\033[48;2;R;G;Bm` au lieu de `38`.

### 5.4 Styles de texte

```cpp
bold("text")
dim("text")
italic("text")
underline("text")
strikethrough("text")
```

### 5.5 Combinaison de styles

```cpp
println(bold(red("Error:")), "something broke");
println(bg_rgb(0,0,0)(bold(hex("#0f0")("Matrix"))));
```

### 5.6 Les 148 couleurs CSS nommées

Chaque nom CSS est disponible comme fonction directe dans le namespace `prism` :

```cpp
println(red("stop"), green("go"), yellow("caution"));
println(coral("warm"), dodgerblue("cool"), orchid("fancy"));
```

Liste complète des 148 couleurs (noms en snake_case pour ceux avec espaces) :

| Nom | Hex | Nom | Hex |
|-----|-----|-----|-----|
| `aliceblue` | #F0F8FF | `antiquewhite` | #FAEBD7 |
| `aqua` | #00FFFF | `aquamarine` | #7FFFD4 |
| `azure` | #F0FFFF | `beige` | #F5F5DC |
| `bisque` | #FFE4C4 | `black` | #000000 |
| `blanchedalmond` | #FFEBCD | `blue` | #0000FF |
| `blueviolet` | #8A2BE2 | `brown` | #A52A2A |
| `burlywood` | #DEB887 | `cadetblue` | #5F9EA0 |
| `chartreuse` | #7FFF00 | `chocolate` | #D2691E |
| `coral` | #FF7F50 | `cornflowerblue` | #6495ED |
| `cornsilk` | #FFF8DC | `crimson` | #DC143C |
| `cyan` | #00FFFF | `darkblue` | #00008B |
| `darkcyan` | #008B8B | `darkgoldenrod` | #B8860B |
| `darkgray` | #A9A9A9 | `darkgreen` | #006400 |
| `darkkhaki` | #BDB76B | `darkmagenta` | #8B008B |
| `darkolivegreen` | #556B2F | `darkorange` | #FF8C00 |
| `darkorchid` | #9932CC | `darkred` | #8B0000 |
| `darksalmon` | #E9967A | `darkseagreen` | #8FBC8F |
| `darkslateblue` | #483D8B | `darkslategray` | #2F4F4F |
| `darkturquoise` | #00CED1 | `darkviolet` | #9400D3 |
| `deeppink` | #FF1493 | `deepskyblue` | #00BFFF |
| `dimgray` | #696969 | `dodgerblue` | #1E90FF |
| `firebrick` | #B22222 | `floralwhite` | #FFFAF0 |
| `forestgreen` | #228B22 | `fuchsia` | #FF00FF |
| `gainsboro` | #DCDCDC | `ghostwhite` | #F8F8FF |
| `gold` | #FFD700 | `goldenrod` | #DAA520 |
| `gray` | #808080 | `green` | #008000 |
| `greenyellow` | #ADFF2F | `honeydew` | #F0FFF0 |
| `hotpink` | #FF69B4 | `indianred` | #CD5C5C |
| `indigo` | #4B0082 | `ivory` | #FFFFF0 |
| `khaki` | #F0E68C | `lavender` | #E6E6FA |
| `lavenderblush` | #FFF0F5 | `lawngreen` | #7CFC00 |
| `lemonchiffon` | #FFFACD | `lightblue` | #ADD8E6 |
| `lightcoral` | #F08080 | `lightcyan` | #E0FFFF |
| `lightgoldenrodyellow` | #FAFAD2 | `lightgray` | #D3D3D3 |
| `lightgreen` | #90EE90 | `lightpink` | #FFB6C1 |
| `lightsalmon` | #FFA07A | `lightseagreen` | #20B2AA |
| `lightskyblue` | #87CEFA | `lightslategray` | #778899 |
| `lightsteelblue` | #B0C4DE | `lightyellow` | #FFFFE0 |
| `lime` | #00FF00 | `limegreen` | #32CD32 |
| `linen` | #FAF0E6 | `magenta` | #FF00FF |
| `maroon` | #800000 | `mediumaquamarine` | #66CDAA |
| `mediumblue` | #0000CD | `mediumorchid` | #BA55D3 |
| `mediumpurple` | #9370DB | `mediumseagreen` | #3CB371 |
| `mediumslateblue` | #7B68EE | `mediumspringgreen` | #00FA9A |
| `mediumturquoise` | #48D1CC | `mediumvioletred` | #C71585 |
| `midnightblue` | #191970 | `mintcream` | #F5FFFA |
| `mistyrose` | #FFE4E1 | `moccasin` | #FFE4B5 |
| `navajowhite` | #FFDEAD | `navy` | #000080 |
| `oldlace` | #FDF5E6 | `olive` | #808000 |
| `olivedrab` | #6B8E23 | `orange` | #FFA500 |
| `orangered` | #FF4500 | `orchid` | #DA70D6 |
| `palegoldenrod` | #EEE8AA | `palegreen` | #98FB98 |
| `paleturquoise` | #AFEEEE | `palevioletred` | #DB7093 |
| `papayawhip` | #FFEFD5 | `peachpuff` | #FFDAB9 |
| `peru` | #CD853F | `pink` | #FFC0CB |
| `plum` | #DDA0DD | `powderblue` | #B0E0E6 |
| `purple` | #800080 | `rebeccapurple` | #663399 |
| `red` | #FF0000 | `rosybrown` | #BC8F8F |
| `royalblue` | #4169E1 | `saddlebrown` | #8B4513 |
| `salmon` | #FA8072 | `sandybrown` | #F4A460 |
| `seagreen` | #2E8B57 | `seashell` | #FFF5EE |
| `sienna` | #A0522D | `silver` | #C0C0C0 |
| `skyblue` | #87CEEB | `slateblue` | #6A5ACD |
| `slategray` | #708090 | `snow` | #FFFAFA |
| `springgreen` | #00FF7F | `steelblue` | #4682B4 |
| `tan` | #D2B48C | `teal` | #008080 |
| `thistle` | #D8BFD8 | `tomato` | #FF6347 |
| `turquoise` | #40E0D0 | `violet` | #EE82EE |
| `wheat` | #F5DEB3 | `white` | #FFFFFF |
| `whitesmoke` | #F5F5F5 | `yellow` | #FFFF00 |
| `yellowgreen` | #9ACD32 | | |

---

## 6. Détection automatique du support couleur

```cpp
// Désactivation auto si :
// - stdout n'est pas un TTY (pipe, redirection)
// - Variable NO_COLOR est définie (https://no-color.org/)
// - Variable TERM=dumb

// Forçage manuel :
prism::enable_colors(true);   // forcer ON
prism::enable_colors(false);  // forcer OFF
```

---

## 7. Namespace et inclusion

```cpp
#include <prism/prism.hpp>

// Tout est dans le namespace prism
prism::println("Hello");
prism::println(prism::red("Error"));

// Ou avec using
using namespace prism;
println(red("Error:"), "file not found");
```

---

## 8. Gestion des erreurs

| Situation | Comportement |
|-----------|-------------|
| Format string avec mauvais nombre de `{}` | `static_assert` si possible, sinon affiche `{?}` pour les manquants |
| Type non-printable | Erreur de compilation claire via `static_assert` avec message |
| Couleur hex invalide | Pas de couleur appliquée (fallback silencieux) |
| HSL/OKLCH hors bornes | Clamp aux valeurs valides |

---

## 9. Performance

- Zéro allocation pour les types primitifs (écriture directe dans le stream)
- Les wrappers couleur sont légers (stockent juste R, G, B)
- Pas de virtual dispatch
- `constexpr` quand possible pour les conversions couleur

---

## 10. Exemples d'usage visés

```cpp
#include <prism/prism.hpp>
using namespace prism;

int main() {
    // Print simple
    println("Hello, World!");

    // Variadic avec types mixtes
    println("Score:", 42, "sur", 100, "->", 0.42);

    // Format string
    println("{} scored {}/{}", "Alice", 42, 100);

    // Couleurs
    println(red("ERROR"), "Something went wrong");
    println(green("OK"), "All tests passed");
    println(hex("#ff6347")("WARNING"), "Disk almost full");

    // Combinaisons
    println(bold(cyan(">>> ")), "Processing...");

    // Containers
    std::vector<int> v = {1, 2, 3, 4, 5};
    println("Vector:", v);  // -> Vector: [1, 2, 3, 4, 5]

    std::map<std::string, int> m = {{"a", 1}, {"b", 2}};
    println("Map:", m);  // -> Map: {a: 1, b: 2}

    // Séparateur custom
    println_sep(" | ", 1, 2, 3);  // -> 1 | 2 | 3

    // Stderr
    eprintln(red("Fatal:"), "cannot open file");

    return 0;
}
```
