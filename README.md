# Prism

C++17 header-only library that replaces verbose `std::cout` with a universal `print()` featuring built-in colors.

```cpp
#include <prism/prism.hpp>
using namespace prism;

int main() {
    println("Hello,", "World!");                          // Hello, World!
    println("{} scored {}/{}", "Alice", 42, 100);         // Alice scored 42/100
    println(red("ERROR"), "something broke");             // ERROR (in red) something broke
    println("data:", std::vector<int>{1, 2, 3});          // data: [1, 2, 3]
}
```

## Installation

Header-only, zero dependencies. Pick your method:

### CMake FetchContent (recommended)

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

### Manual copy

```bash
cp -r include/prism/ your_project/include/
g++ -std=c++17 -I include main.cpp -o main
```

## API

### print / println

```cpp
// Variadic — automatic space separator
print("a", 42, true);          // a 42 true
println("line", 1);            // line 1\n

// Format string — auto-detected when first arg contains {}
println("{} = {}", "x", 10);   // x = 10
println("{{escaped}}");        // {escaped}

// Custom separator
println_sep(" | ", 1, 2, 3);   // 1 | 2 | 3
println_sep(", ", "a", "b");   // a, b

// Stderr
eprintln("error:", msg);
eprint("{} failed", cmd);
```

### Supported types

All types print natively with no configuration:

```cpp
// Primitives
println(42, 3.14, true, 'A', nullptr);   // 42 3.14 true A nullptr

// STL containers
println(std::vector<int>{1, 2, 3});                      // [1, 2, 3]
println(std::map<std::string, int>{{"a", 1}, {"b", 2}}); // {a: 1, b: 2}
println(std::set<int>{3, 1, 2});                         // {1, 2, 3}
println(std::make_pair(1, "hello"));                      // (1, hello)
println(std::make_tuple(1, "x", 3.14));                   // (1, x, 3.14)
println(std::optional<int>(42));                          // Some(42)
println(std::optional<int>());                            // None

// Nested
println(std::vector<std::vector<int>>{{1, 2}, {3, 4}});  // [[1, 2], [3, 4]]
```

### Custom types

Two ways to make a type printable:

```cpp
struct Point { int x, y; };

// Option A: operator<< (works with existing code)
std::ostream& operator<<(std::ostream& os, const Point& p) {
    return os << "(" << p.x << ", " << p.y << ")";
}

// Option B: specialize Formatter (takes priority)
template<>
struct prism::Formatter<Point> {
    static std::string format(const Point& p) {
        return "(" + std::to_string(p.x) + ", " + std::to_string(p.y) + ")";
    }
};

println(Point{3, 4});  // (3, 4)
```

A type with neither `operator<<` nor `Formatter` produces a clear compile-time error.

## Colors

### 4 color spaces

```cpp
rgb(255, 0, 0)("text")           // RGB direct
hex("#ff6347")("text")            // Hexadecimal (#RRGGBB, #RGB, without #)
hsl(270, 100, 50)("text")        // HSL (h:0-360, s:0-100, l:0-100)
oklch(0.7, 0.15, 150)("text")    // OKLCH (L:0-1, C:0-0.4, H:0-360)
```

### 148 named CSS colors

Every CSS color is a direct function:

```cpp
println(red("stop"), green("go"), yellow("caution"));
println(coral("warm"), dodgerblue("cool"), orchid("fancy"));
println(crimson("alert"), gold("star"), tomato("hot"));
```

<details>
<summary>Full list of 148 colors</summary>

`aliceblue` `antiquewhite` `aqua` `aquamarine` `azure` `beige` `bisque` `black` `blanchedalmond` `blue` `blueviolet` `brown` `burlywood` `cadetblue` `chartreuse` `chocolate` `coral` `cornflowerblue` `cornsilk` `crimson` `cyan` `darkblue` `darkcyan` `darkgoldenrod` `darkgray` `darkgreen` `darkgrey` `darkkhaki` `darkmagenta` `darkolivegreen` `darkorange` `darkorchid` `darkred` `darksalmon` `darkseagreen` `darkslateblue` `darkslategray` `darkslategrey` `darkturquoise` `darkviolet` `deeppink` `deepskyblue` `dimgray` `dimgrey` `dodgerblue` `firebrick` `floralwhite` `forestgreen` `fuchsia` `gainsboro` `ghostwhite` `gold` `goldenrod` `gray` `green` `greenyellow` `grey` `honeydew` `hotpink` `indianred` `indigo` `ivory` `khaki` `lavender` `lavenderblush` `lawngreen` `lemonchiffon` `lightblue` `lightcoral` `lightcyan` `lightgoldenrodyellow` `lightgray` `lightgreen` `lightgrey` `lightpink` `lightsalmon` `lightseagreen` `lightskyblue` `lightslategray` `lightslategrey` `lightsteelblue` `lightyellow` `lime` `limegreen` `linen` `magenta` `maroon` `mediumaquamarine` `mediumblue` `mediumorchid` `mediumpurple` `mediumseagreen` `mediumslateblue` `mediumspringgreen` `mediumturquoise` `mediumvioletred` `midnightblue` `mintcream` `mistyrose` `moccasin` `navajowhite` `navy` `oldlace` `olive` `olivedrab` `orange` `orangered` `orchid` `palegoldenrod` `palegreen` `paleturquoise` `palevioletred` `papayawhip` `peachpuff` `peru` `pink` `plum` `powderblue` `purple` `rebeccapurple` `red` `rosybrown` `royalblue` `saddlebrown` `salmon` `sandybrown` `seagreen` `seashell` `sienna` `silver` `skyblue` `slateblue` `slategray` `slategrey` `snow` `springgreen` `steelblue` `tan` `teal` `thistle` `tomato` `turquoise` `violet` `wheat` `white` `whitesmoke` `yellow` `yellowgreen`

</details>

### Dynamic lookup

```cpp
auto theme = css("steelblue");     // by name string
println(theme("styled text"));

auto bg = bg_css("papayawhip");    // background by name
println(bg("warm background"));
```

### Background colors

```cpp
bg_rgb(255, 255, 0)("highlight")
bg_hex("#333")(white("dark bg"))
bg_hsl(120, 100, 50)("green bg")

// CSS functions: bg_red(), bg_blue(), bg_gold(), ...
println(bg_red(white(" ALERT ")), bg_green(black(" OK ")));
```

### Text styles

```cpp
bold("bold text")
dim("dimmed")
italic("italic")
underline("underlined")
strikethrough("strikethrough")
```

### Free composition

Styles and colors compose by nesting:

```cpp
println(bold(red("ERROR:")), "file not found");
println(italic(dodgerblue("Note:")), "this is info");
println(bg_black(bold(green(">>> "))), "command executed");

// Inside format strings
println("{} got {}", bold("Alice"), gold("95/100"));
```

## Automatic color detection

ANSI codes are automatically disabled when:
- stdout is not a TTY (pipe, file redirect)
- The `NO_COLOR` environment variable is set ([no-color.org](https://no-color.org))
- `TERM=dumb`

Manual override:

```cpp
prism::enable_colors(true);    // always ON
prism::enable_colors(false);   // always OFF
prism::reset_colors();         // back to auto-detection
```

## Specs

| | |
|---|---|
| Standard | C++17 |
| Format | Header-only (single `.hpp`) |
| Dependencies | Zero (STL only) |
| Colors | ANSI truecolor 24-bit |
| Platforms | Linux, macOS, Windows 10+ |

## Building tests

```bash
# With CMake
cmake -B build -DPRISM_BUILD_TESTS=ON -DPRISM_BUILD_EXAMPLES=ON
cmake --build build
ctest --test-dir build

# Or directly
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -I include tests/test_prism.cpp -o test_prism
./test_prism
```

## License

[MIT](LICENSE)
