#include <prism/prism.hpp>
#include <string>
#include <sstream>
#include <cassert>

// Helper : capture stdout dans un string
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

    // --- 2.1 : détection placeholder ---
    {
        CaptureStdout cap;

        // String avec {} → mode format
        print("{} + {} = {}", 1, 2, 3);
        assert(cap.str() == "1 + 2 = 3");
        cap.reset();

        // String sans {} → mode variadic (séparateur espace)
        print("hello", 42);
        assert(cap.str() == "hello 42");
        cap.reset();
    }

    // --- 2.2 : format string basique ---
    {
        CaptureStdout cap;

        // Un seul placeholder
        println("Value: {}", 42);
        assert(cap.str() == "Value: 42\n");
        cap.reset();

        // Plusieurs types mixtes
        println("{} is {} years old", "Alice", 30);
        assert(cap.str() == "Alice is 30 years old\n");
        cap.reset();

        // Bool dans format
        print("{} == {}", true, false);
        assert(cap.str() == "true == false");
        cap.reset();

        // String vide comme format (pas de {})
        print("no placeholders here");
        assert(cap.str() == "no placeholders here");
        cap.reset();
    }

    // --- 2.2 : échappement {{ et }} ---
    {
        CaptureStdout cap;

        // {{ → { littéral
        print("{{}}");
        assert(cap.str() == "{}");
        cap.reset();

        // Mixte : {{ avec placeholder
        print("{{{}}}", "inner");
        assert(cap.str() == "{inner}");
        cap.reset();

        // }} seul
        print("a }}");
        assert(cap.str() == "a }");
        cap.reset();

        // {{ seul
        print("a {{");
        assert(cap.str() == "a {");
        cap.reset();
    }

    // --- 2.3 : gestion d'erreur ---
    {
        CaptureStdout cap;

        // Plus de {} que d'arguments → {?} pour les manquants
        print("{} {} {}", 1);
        assert(cap.str() == "1 {?} {?}");
        cap.reset();

        // Plus d'arguments que de {} → arguments excédentaires ignorés
        print("{}", 1, 2, 3);
        assert(cap.str() == "1");
        cap.reset();

        // Zéro {} dans format mais extra args → variadic mode
        print("no placeholder", 42);
        assert(cap.str() == "no placeholder 42");
        cap.reset();
    }

    // --- Cas limites ---
    {
        CaptureStdout cap;

        // println format
        println("{} {}", "a", "b");
        assert(cap.str() == "a b\n");
        cap.reset();

        // print un seul string (pas de format, juste un string)
        print("just a string");
        assert(cap.str() == "just a string");
        cap.reset();

        // Premier arg non-string avec format-like string après
        print(42, "{}", "x");
        assert(cap.str() == "42 {} x");
        cap.reset();

        // std::string comme format
        std::string fmt_str = "Hello {}!";
        print(fmt_str, "World");
        assert(cap.str() == "Hello World!");
        cap.reset();

        // Un seul argument string avec {} → format mode, pas d'arg → {?}
        println("{}");
        assert(cap.str() == "{?}\n");
        cap.reset();
    }

    std::cout << "All phase 2 tests passed!" << std::endl;
    return 0;
}
