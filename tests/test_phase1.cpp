#include <prism/prism.hpp>
#include <string>
#include <cassert>
#include <sstream>

// Type custom avec operator<<
struct Point {
    int x, y;
};

std::ostream& operator<<(std::ostream& os, const Point& p) {
    return os << "(" << p.x << ", " << p.y << ")";
}

// Type custom avec Formatter spécialisé
struct Color {
    int r, g, b;
};

template <>
struct prism::Formatter<Color> {
    static std::string format(const Color& c) {
        return "rgb(" + std::to_string(c.r) + ", "
             + std::to_string(c.g) + ", "
             + std::to_string(c.b) + ")";
    }
};

// Type avec les DEUX (Formatter doit gagner)
struct Dual {
    int val;
};

std::ostream& operator<<(std::ostream& os, const Dual& d) {
    return os << "ostream:" << d.val;
}

template <>
struct prism::Formatter<Dual> {
    static std::string format(const Dual& d) {
        return "formatter:" + std::to_string(d.val);
    }
};

int main() {
    using namespace prism;

    // --- Test to_string_value ---
    // Primitifs via operator<<
    assert(to_string_value(42) == "42");
    assert(to_string_value(3.14) == "3.14");
    assert(to_string_value('A') == "A");
    assert(to_string_value(std::string("hello")) == "hello");

    // bool via Formatter
    assert(to_string_value(true) == "true");
    assert(to_string_value(false) == "false");

    // nullptr via Formatter
    assert(to_string_value(nullptr) == "nullptr");

    // Custom operator<<
    assert(to_string_value(Point{3, 4}) == "(3, 4)");

    // Custom Formatter
    assert(to_string_value(Color{255, 0, 128}) == "rgb(255, 0, 128)");

    // Priorité : Formatter > operator<<
    assert(to_string_value(Dual{7}) == "formatter:7");

    // --- Test print / println output ---
    // Capture stdout
    std::ostringstream capture;
    auto* old_buf = std::cout.rdbuf(capture.rdbuf());

    // print zéro args
    print();
    assert(capture.str() == "");

    // println zéro args
    println();
    assert(capture.str() == "\n");

    // Reset
    capture.str("");
    capture.clear();

    // print variadic avec séparateur espace
    print("Hello", 42, 3.14);
    assert(capture.str() == "Hello 42 3.14");

    // Reset
    capture.str("");
    capture.clear();

    // println variadic
    println("Name:", std::string("Alice"), true);
    assert(capture.str() == "Name: Alice true\n");

    // Reset
    capture.str("");
    capture.clear();

    // print avec types custom
    println(Point{1, 2}, Color{0, 255, 0});
    assert(capture.str() == "(1, 2) rgb(0, 255, 0)\n");

    // Reset
    capture.str("");
    capture.clear();

    // print un seul argument
    print(42);
    assert(capture.str() == "42");

    // Reset
    capture.str("");
    capture.clear();

    // println un seul argument
    println("solo");
    assert(capture.str() == "solo\n");

    // Restaurer stdout
    std::cout.rdbuf(old_buf);

    std::cout << "All phase 1 tests passed!\n";
    return 0;
}
