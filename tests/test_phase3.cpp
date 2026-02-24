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
#include <utility>
#include <tuple>
#include <optional>
#include <variant>

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

    // --- 3.1 : Containers séquentiels → [1, 2, 3] ---
    {
        // vector
        std::vector<int> v = {1, 2, 3};
        assert(to_string_value(v) == "[1, 2, 3]");

        // vector vide
        std::vector<int> empty_v;
        assert(to_string_value(empty_v) == "[]");

        // array
        std::array<int, 3> a = {10, 20, 30};
        assert(to_string_value(a) == "[10, 20, 30]");

        // list
        std::list<std::string> l = {"a", "b", "c"};
        assert(to_string_value(l) == "[a, b, c]");

        // deque
        std::deque<double> d = {1.5, 2.5};
        assert(to_string_value(d) == "[1.5, 2.5]");

        // forward_list
        std::forward_list<int> fl = {7, 8, 9};
        assert(to_string_value(fl) == "[7, 8, 9]");

        // vector imbriqué
        std::vector<std::vector<int>> nested = {{1, 2}, {3, 4}};
        assert(to_string_value(nested) == "[[1, 2], [3, 4]]");
    }

    // --- 3.2 : Containers associatifs → {a: 1, b: 2} ---
    {
        // map (ordonné → ordre prévisible)
        std::map<std::string, int> m = {{"a", 1}, {"b", 2}, {"c", 3}};
        assert(to_string_value(m) == "{a: 1, b: 2, c: 3}");

        // map vide
        std::map<int, int> empty_m;
        assert(to_string_value(empty_m) == "{}");

        // unordered_map — vérifier juste le format (ordre non garanti)
        std::unordered_map<std::string, int> um = {{"x", 42}};
        std::string um_str = to_string_value(um);
        assert(um_str == "{x: 42}");

        // map avec valeurs bool
        std::map<std::string, bool> mb = {{"ok", true}, {"fail", false}};
        assert(to_string_value(mb) == "{fail: false, ok: true}");
    }

    // --- 3.3 : Sets → {1, 2, 3} ---
    {
        // set (ordonné)
        std::set<int> s = {3, 1, 2};
        assert(to_string_value(s) == "{1, 2, 3}");

        // set vide
        std::set<int> empty_s;
        assert(to_string_value(empty_s) == "{}");

        // unordered_set — un seul élément pour ordre prévisible
        std::unordered_set<int> us = {42};
        assert(to_string_value(us) == "{42}");
    }

    // --- 3.4 : pair et tuple ---
    {
        // pair
        auto p = std::make_pair(1, std::string("hello"));
        assert(to_string_value(p) == "(1, hello)");

        // pair de bools
        auto pb = std::make_pair(true, false);
        assert(to_string_value(pb) == "(true, false)");

        // tuple
        auto t = std::make_tuple(1, std::string("hello"), 3.14);
        assert(to_string_value(t) == "(1, hello, 3.14)");

        // tuple avec un seul élément
        auto t1 = std::make_tuple(42);
        assert(to_string_value(t1) == "(42)");

        // tuple vide
        auto t0 = std::make_tuple();
        assert(to_string_value(t0) == "()");
    }

    // --- 3.5 : optional ---
    {
        std::optional<int> some = 42;
        assert(to_string_value(some) == "Some(42)");

        std::optional<int> none;
        assert(to_string_value(none) == "None");

        std::optional<std::string> some_str = "hello";
        assert(to_string_value(some_str) == "Some(hello)");
    }

    // --- 3.6 : variant ---
    {
        std::variant<int, std::string, double> v1 = 42;
        assert(to_string_value(v1) == "42");

        std::variant<int, std::string, double> v2 = std::string("hello");
        assert(to_string_value(v2) == "hello");

        std::variant<int, std::string, double> v3 = 3.14;
        assert(to_string_value(v3) == "3.14");
    }

    // --- Intégration avec print/println ---
    {
        CaptureStdout cap;

        std::vector<int> v = {1, 2, 3};
        println("Vec:", v);
        assert(cap.str() == "Vec: [1, 2, 3]\n");
        cap.reset();

        std::map<std::string, int> m = {{"x", 10}};
        println("Map:", m);
        assert(cap.str() == "Map: {x: 10}\n");
        cap.reset();

        // Format string avec container
        println("data = {}", v);
        assert(cap.str() == "data = [1, 2, 3]\n");
        cap.reset();

        // Combinaisons complexes
        auto t = std::make_tuple(std::vector<int>{1, 2}, std::optional<int>(99));
        println(t);
        assert(cap.str() == "([1, 2], Some(99))\n");
        cap.reset();
    }

    std::cout << "All phase 3 tests passed!" << std::endl;
    return 0;
}
