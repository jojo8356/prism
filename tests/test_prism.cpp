// test_prism.cpp — Tests unitaires complets de la bibliothèque prism
// Couvre : print, format, couleurs, types STL, styles, composition, sep, stderr, toggle

#include <prism/prism.hpp>
#include <cassert>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <list>
#include <deque>
#include <forward_list>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <optional>
#include <tuple>
#include <variant>

// --- Helpers ---

struct CaptureStream {
    std::ostream& stream;
    std::ostringstream buf;
    std::streambuf* old;
    CaptureStream(std::ostream& s) : stream(s), old(s.rdbuf(buf.rdbuf())) {}
    ~CaptureStream() { stream.rdbuf(old); }
    std::string str() const { return buf.str(); }
    void reset() { buf.str(""); buf.clear(); }
};

// Custom type via operator<<
struct Point { int x, y; };
std::ostream& operator<<(std::ostream& os, const Point& p) {
    return os << "(" << p.x << ", " << p.y << ")";
}

// Custom type via Formatter
struct Color { int r, g, b; };
template<> struct prism::Formatter<Color> {
    static std::string format(const Color& c) {
        return "rgb(" + std::to_string(c.r) + "," + std::to_string(c.g) + "," + std::to_string(c.b) + ")";
    }
};

// Type with both (Formatter wins)
struct Dual { int v; };
std::ostream& operator<<(std::ostream& os, const Dual& d) { return os << "os:" << d.v; }
template<> struct prism::Formatter<Dual> {
    static std::string format(const Dual& d) { return "fmt:" + std::to_string(d.v); }
};

static int tests_passed = 0;

#define TEST(name) \
    do { \
        std::string test_name = name;

#define END_TEST \
        ++tests_passed; \
    } while(0)

int main() {
    using namespace prism;
    enable_colors(true);

    // =====================================================================
    // PRINT BASIQUE
    // =====================================================================

    TEST("print() zero args") {
        CaptureStream cap(std::cout);
        print();
        assert(cap.str().empty());
    } END_TEST;

    TEST("println() zero args") {
        CaptureStream cap(std::cout);
        println();
        assert(cap.str() == "\n");
    } END_TEST;

    TEST("print single string") {
        CaptureStream cap(std::cout);
        print("hello");
        assert(cap.str() == "hello");
    } END_TEST;

    TEST("println single int") {
        CaptureStream cap(std::cout);
        println(42);
        assert(cap.str() == "42\n");
    } END_TEST;

    TEST("print variadic mixed types") {
        CaptureStream cap(std::cout);
        print("a", 1, 3.14, true, 'X');
        assert(cap.str() == "a 1 3.14 true X");
    } END_TEST;

    TEST("println variadic") {
        CaptureStream cap(std::cout);
        println("Name:", std::string("Alice"), 30);
        assert(cap.str() == "Name: Alice 30\n");
    } END_TEST;

    // =====================================================================
    // TYPES PRIMITIFS
    // =====================================================================

    TEST("bool formatting") {
        assert(to_string_value(true) == "true");
        assert(to_string_value(false) == "false");
    } END_TEST;

    TEST("nullptr formatting") {
        assert(to_string_value(nullptr) == "nullptr");
    } END_TEST;

    TEST("string types") {
        assert(to_string_value(std::string("s")) == "s");
        assert(to_string_value("cstr") == "cstr");
    } END_TEST;

    // =====================================================================
    // CUSTOM TYPES
    // =====================================================================

    TEST("custom operator<<") {
        assert(to_string_value(Point{3, 4}) == "(3, 4)");
    } END_TEST;

    TEST("custom Formatter") {
        assert(to_string_value(Color{1, 2, 3}) == "rgb(1,2,3)");
    } END_TEST;

    TEST("Formatter priority over operator<<") {
        assert(to_string_value(Dual{7}) == "fmt:7");
    } END_TEST;

    // =====================================================================
    // FORMAT STRINGS
    // =====================================================================

    TEST("format basic") {
        CaptureStream cap(std::cout);
        print("{} + {} = {}", 1, 2, 3);
        assert(cap.str() == "1 + 2 = 3");
    } END_TEST;

    TEST("format with string arg") {
        CaptureStream cap(std::cout);
        println("{} is {}", "Alice", 30);
        assert(cap.str() == "Alice is 30\n");
    } END_TEST;

    TEST("format escape braces") {
        CaptureStream cap(std::cout);
        print("{{}}");
        assert(cap.str() == "{}");
    } END_TEST;

    TEST("format mixed escape and placeholder") {
        CaptureStream cap(std::cout);
        print("{{{}}}", "inner");
        assert(cap.str() == "{inner}");
    } END_TEST;

    TEST("format missing args -> {?}") {
        CaptureStream cap(std::cout);
        print("{} {} {}", 1);
        assert(cap.str() == "1 {?} {?}");
    } END_TEST;

    TEST("format excess args ignored") {
        CaptureStream cap(std::cout);
        print("{}", 1, 2, 3);
        assert(cap.str() == "1");
    } END_TEST;

    TEST("no placeholder -> variadic mode") {
        CaptureStream cap(std::cout);
        print("hello", 42);
        assert(cap.str() == "hello 42");
    } END_TEST;

    TEST("first arg non-string -> variadic") {
        CaptureStream cap(std::cout);
        print(42, "{}", "x");
        assert(cap.str() == "42 {} x");
    } END_TEST;

    TEST("std::string as format") {
        CaptureStream cap(std::cout);
        std::string fmt = "val={}";
        print(fmt, 10);
        assert(cap.str() == "val=10");
    } END_TEST;

    // =====================================================================
    // CONTAINERS STL
    // =====================================================================

    TEST("vector") {
        assert(to_string_value(std::vector<int>{1, 2, 3}) == "[1, 2, 3]");
        assert(to_string_value(std::vector<int>{}) == "[]");
    } END_TEST;

    TEST("array") {
        assert(to_string_value(std::array<int, 3>{10, 20, 30}) == "[10, 20, 30]");
    } END_TEST;

    TEST("list") {
        assert(to_string_value(std::list<std::string>{"a", "b"}) == "[a, b]");
    } END_TEST;

    TEST("deque") {
        assert(to_string_value(std::deque<int>{7, 8}) == "[7, 8]");
    } END_TEST;

    TEST("forward_list") {
        assert(to_string_value(std::forward_list<int>{1, 2}) == "[1, 2]");
    } END_TEST;

    TEST("nested vector") {
        std::vector<std::vector<int>> v = {{1, 2}, {3, 4}};
        assert(to_string_value(v) == "[[1, 2], [3, 4]]");
    } END_TEST;

    TEST("map") {
        std::map<std::string, int> m = {{"a", 1}, {"b", 2}};
        assert(to_string_value(m) == "{a: 1, b: 2}");
    } END_TEST;

    TEST("map empty") {
        assert(to_string_value(std::map<int, int>{}) == "{}");
    } END_TEST;

    TEST("set") {
        assert(to_string_value(std::set<int>{3, 1, 2}) == "{1, 2, 3}");
        assert(to_string_value(std::set<int>{}) == "{}");
    } END_TEST;

    TEST("pair") {
        assert(to_string_value(std::make_pair(1, "hi")) == "(1, hi)");
    } END_TEST;

    TEST("tuple") {
        assert(to_string_value(std::make_tuple(1, "x", 3.14)) == "(1, x, 3.14)");
        assert(to_string_value(std::make_tuple()) == "()");
        assert(to_string_value(std::make_tuple(42)) == "(42)");
    } END_TEST;

    TEST("optional") {
        assert(to_string_value(std::optional<int>(42)) == "Some(42)");
        assert(to_string_value(std::optional<int>()) == "None");
    } END_TEST;

    TEST("variant") {
        std::variant<int, std::string> v1 = 42;
        assert(to_string_value(v1) == "42");
        std::variant<int, std::string> v2 = std::string("hi");
        assert(to_string_value(v2) == "hi");
    } END_TEST;

    TEST("containers in print") {
        CaptureStream cap(std::cout);
        println("v={}", std::vector<int>{1, 2});
        assert(cap.str() == "v=[1, 2]\n");
    } END_TEST;

    // =====================================================================
    // COULEURS
    // =====================================================================

    TEST("rgb") {
        assert(rgb(255, 0, 0)("x").content == "\033[38;2;255;0;0mx\033[0m");
    } END_TEST;

    TEST("rgb clamp") {
        assert(rgb(-1, 300, 128)("x").content == "\033[38;2;0;255;128mx\033[0m");
    } END_TEST;

    TEST("hex #RRGGBB") {
        assert(hex("#ff6347")("x").content == "\033[38;2;255;99;71mx\033[0m");
    } END_TEST;

    TEST("hex #RGB") {
        assert(hex("#f00")("x").content == "\033[38;2;255;0;0mx\033[0m");
    } END_TEST;

    TEST("hex sans #") {
        assert(hex("00ff00")("x").content == "\033[38;2;0;255;0mx\033[0m");
    } END_TEST;

    TEST("hex invalid -> white") {
        assert(hex("xyz")("x").content == "\033[38;2;255;255;255mx\033[0m");
    } END_TEST;

    TEST("hsl primaries") {
        assert(hsl(0, 100, 50)("x").content == "\033[38;2;255;0;0mx\033[0m");
        assert(hsl(120, 100, 50)("x").content == "\033[38;2;0;255;0mx\033[0m");
        assert(hsl(240, 100, 50)("x").content == "\033[38;2;0;0;255mx\033[0m");
    } END_TEST;

    TEST("hsl black/white") {
        assert(hsl(0, 0, 0)("x").content == "\033[38;2;0;0;0mx\033[0m");
        assert(hsl(0, 0, 100)("x").content == "\033[38;2;255;255;255mx\033[0m");
    } END_TEST;

    TEST("oklch extremes") {
        assert(oklch(0, 0, 0)("x").content == "\033[38;2;0;0;0mx\033[0m");
        assert(oklch(1, 0, 0)("x").content == "\033[38;2;255;255;255mx\033[0m");
    } END_TEST;

    // =====================================================================
    // BACKGROUND
    // =====================================================================

    TEST("bg_rgb") {
        assert(bg_rgb(255, 0, 0)("x").content == "\033[48;2;255;0;0mx\033[0m");
    } END_TEST;

    TEST("bg_hex") {
        assert(bg_hex("#0f0")("x").content == "\033[48;2;0;255;0mx\033[0m");
    } END_TEST;

    TEST("bg_hsl") {
        assert(bg_hsl(240, 100, 50)("x").content == "\033[48;2;0;0;255mx\033[0m");
    } END_TEST;

    TEST("bg_oklch") {
        assert(bg_oklch(0, 0, 0)("x").content == "\033[48;2;0;0;0mx\033[0m");
    } END_TEST;

    // =====================================================================
    // STYLES
    // =====================================================================

    TEST("bold") {
        assert(bold("x").content == "\033[1mx\033[22m");
    } END_TEST;

    TEST("dim") {
        assert(dim("x").content == "\033[2mx\033[22m");
    } END_TEST;

    TEST("italic") {
        assert(italic("x").content == "\033[3mx\033[23m");
    } END_TEST;

    TEST("underline") {
        assert(underline("x").content == "\033[4mx\033[24m");
    } END_TEST;

    TEST("strikethrough") {
        assert(strikethrough("x").content == "\033[9mx\033[29m");
    } END_TEST;

    TEST("style with non-string") {
        assert(bold(42).content == "\033[1m42\033[22m");
        assert(italic(true).content == "\033[3mtrue\033[23m");
    } END_TEST;

    // =====================================================================
    // COMPOSITION
    // =====================================================================

    TEST("bold(red(...))") {
        auto st = bold(red("E"));
        assert(st.content == "\033[1m\033[38;2;255;0;0mE\033[0m\033[22m");
    } END_TEST;

    TEST("rgb()(bold(...))") {
        auto st = rgb(0, 255, 0)(bold("OK"));
        assert(st.content == "\033[38;2;0;255;0m\033[1mOK\033[22m\033[0m");
    } END_TEST;

    TEST("triple nesting") {
        auto st = bg_black(bold(green("M")));
        assert(st.content ==
            "\033[48;2;0;0;0m\033[1m\033[38;2;0;128;0mM\033[0m\033[22m\033[0m");
    } END_TEST;

    // =====================================================================
    // CSS NAMED COLORS
    // =====================================================================

    TEST("css fg functions") {
        assert(red("x").content == "\033[38;2;255;0;0mx\033[0m");
        assert(coral("x").content == "\033[38;2;255;127;80mx\033[0m");
        assert(dodgerblue("x").content == "\033[38;2;30;144;255mx\033[0m");
    } END_TEST;

    TEST("css bg functions") {
        assert(bg_red("x").content == "\033[48;2;255;0;0mx\033[0m");
        assert(bg_gold("x").content == "\033[48;2;255;215;0mx\033[0m");
    } END_TEST;

    TEST("css() dynamic lookup") {
        assert(css("tomato")("x").content == "\033[38;2;255;99;71mx\033[0m");
        assert(css("nonexistent")("x").content == "\033[38;2;255;255;255mx\033[0m");
    } END_TEST;

    TEST("bg_css() dynamic lookup") {
        assert(bg_css("red")("x").content == "\033[48;2;255;0;0mx\033[0m");
    } END_TEST;

    // =====================================================================
    // PRINT_SEP
    // =====================================================================

    TEST("print_sep comma") {
        CaptureStream cap(std::cout);
        print_sep(",", "a", "b", "c");
        assert(cap.str() == "a,b,c");
    } END_TEST;

    TEST("println_sep pipe") {
        CaptureStream cap(std::cout);
        println_sep(" | ", 1, 2, 3);
        assert(cap.str() == "1 | 2 | 3\n");
    } END_TEST;

    TEST("print_sep single arg") {
        CaptureStream cap(std::cout);
        print_sep("-", "solo");
        assert(cap.str() == "solo");
    } END_TEST;

    // =====================================================================
    // EPRINT / EPRINTLN
    // =====================================================================

    TEST("eprint to stderr") {
        CaptureStream cap_out(std::cout);
        CaptureStream cap_err(std::cerr);
        eprint("err");
        println("out");
        assert(cap_out.str() == "out\n");
        assert(cap_err.str() == "err");
    } END_TEST;

    TEST("eprintln format") {
        CaptureStream cap(std::cerr);
        eprintln("{} = {}", "x", 10);
        assert(cap.str() == "x = 10\n");
    } END_TEST;

    TEST("eprintln zero args") {
        CaptureStream cap(std::cerr);
        eprintln();
        assert(cap.str() == "\n");
    } END_TEST;

    // =====================================================================
    // ENABLE/DISABLE COLORS
    // =====================================================================

    TEST("enable_colors(false) strips ANSI") {
        enable_colors(false);
        assert(red("x").content == "x");
        assert(bold("y").content == "y");
        assert(bg_blue("z").content == "z");
        assert(bold(red("E")).content == "E");
        enable_colors(true);
    } END_TEST;

    TEST("enable_colors(true) restores ANSI") {
        enable_colors(true);
        assert(red("x").content == "\033[38;2;255;0;0mx\033[0m");
    } END_TEST;

    TEST("reset_colors auto-detect") {
        reset_colors();
        // stdout is redirected -> no TTY -> colors off
        assert(red("x").content == "x");
        enable_colors(true); // restore for any following test
    } END_TEST;

    // =====================================================================
    // INTEGRATION : couleurs dans print
    // =====================================================================

    TEST("println with colors variadic") {
        CaptureStream cap(std::cout);
        println(red("E:"), "msg");
        assert(cap.str() == "\033[38;2;255;0;0mE:\033[0m msg\n");
    } END_TEST;

    TEST("println with colors format") {
        CaptureStream cap(std::cout);
        println("{} {}", hex("#0f0")("OK"), "done");
        assert(cap.str() == "\033[38;2;0;255;0mOK\033[0m done\n");
    } END_TEST;

    TEST("color wrapping int") {
        CaptureStream cap(std::cout);
        print(rgb(0, 0, 255)(42));
        assert(cap.str() == "\033[38;2;0;0;255m42\033[0m");
    } END_TEST;

    // =====================================================================

    std::cout << "All " << tests_passed << " tests passed!" << std::endl;
    return 0;
}
