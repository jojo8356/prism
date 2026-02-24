#include <prism/prism.hpp>
#include <cassert>
#include <sstream>
#include <string>

struct CaptureStream {
    std::ostream& stream;
    std::ostringstream buf;
    std::streambuf* old;

    CaptureStream(std::ostream& s) : stream(s), old(s.rdbuf(buf.rdbuf())) {}
    ~CaptureStream() { stream.rdbuf(old); }
    std::string str() const { return buf.str(); }
    void reset() { buf.str(""); buf.clear(); }
};

int main() {
    using namespace prism;

    // --- 7.1 : print_sep / println_sep ---
    {
        CaptureStream cap(std::cout);

        // Séparateur virgule
        print_sep(",", "a", "b", "c");
        assert(cap.str() == "a,b,c");
        cap.reset();

        // Séparateur pipe avec newline
        println_sep(" | ", 1, 2, 3);
        assert(cap.str() == "1 | 2 | 3\n");
        cap.reset();

        // Séparateur avec un seul arg
        print_sep("-", "solo");
        assert(cap.str() == "solo");
        cap.reset();

        // Séparateur vide
        println_sep("", "a", "b", "c");
        assert(cap.str() == "abc\n");
        cap.reset();

        // Séparateur multi-char
        print_sep(" -> ", 1, 2, 3);
        assert(cap.str() == "1 -> 2 -> 3");
        cap.reset();

        // Séparateur avec types mixtes
        println_sep(", ", "hello", 42, true, 3.14);
        assert(cap.str() == "hello, 42, true, 3.14\n");
        cap.reset();

        // Séparateur avec couleurs
        enable_colors(true);
        println_sep(" / ", red("a"), blue("b"));
        std::string expected =
            "\033[38;2;255;0;0ma\033[0m"
            " / "
            "\033[38;2;0;0;255mb\033[0m"
            "\n";
        assert(cap.str() == expected);
        cap.reset();
    }

    // --- 7.2 : eprint / eprintln ---
    {
        CaptureStream cap(std::cerr);

        // eprint basique
        eprint("error");
        assert(cap.str() == "error");
        cap.reset();

        // eprintln
        eprintln("fatal error");
        assert(cap.str() == "fatal error\n");
        cap.reset();

        // eprint variadic
        eprint("code:", 42);
        assert(cap.str() == "code: 42");
        cap.reset();

        // eprintln variadic
        eprintln("status:", true);
        assert(cap.str() == "status: true\n");
        cap.reset();

        // eprint format string
        eprint("{} = {}", "x", 10);
        assert(cap.str() == "x = 10");
        cap.reset();

        // eprintln format string
        eprintln("{} failed with code {}", "process", 1);
        assert(cap.str() == "process failed with code 1\n");
        cap.reset();

        // eprint() / eprintln() zéro args
        eprint();
        assert(cap.str() == "");
        cap.reset();

        eprintln();
        assert(cap.str() == "\n");
        cap.reset();

        // eprint un seul non-string
        eprint(42);
        assert(cap.str() == "42");
        cap.reset();

        // eprintln un seul non-string
        eprintln(42);
        assert(cap.str() == "42\n");
        cap.reset();

        // eprint avec couleurs
        enable_colors(true);
        eprintln(red("Error:"), "boom");
        std::string expected = "\033[38;2;255;0;0mError:\033[0m boom\n";
        assert(cap.str() == expected);
        cap.reset();

        // eprint multi args, premier non-string
        eprint(42, "is", "the answer");
        assert(cap.str() == "42 is the answer");
        cap.reset();

        // eprintln multi args, premier non-string
        eprintln(42, "is", "the answer");
        assert(cap.str() == "42 is the answer\n");
        cap.reset();
    }

    // --- 7.3 : Détection auto couleurs ---
    {
        // Quand stdout est capturé (pas un TTY), auto-detect doit désactiver
        reset_colors(); // revenir en auto
        // On ne peut pas vraiment tester la détection TTY ici (stdout est redirigé)
        // mais on peut vérifier que le mode auto fonctionne
        StyledText st = red("test");
        // En auto, pas de TTY → pas de couleurs
        assert(st.content == "test");
    }

    // --- 7.4 : enable_colors / reset_colors ---
    {
        // Forcer ON
        enable_colors(true);
        assert(red("x").content == "\033[38;2;255;0;0mx\033[0m");

        // Forcer OFF
        enable_colors(false);
        assert(red("x").content == "x");
        assert(bold("y").content == "y");
        assert(bg_blue("z").content == "z");

        // Reset → auto (pas de TTY dans les tests → désactivé)
        reset_colors();
        assert(red("x").content == "x");

        // Re-forcer pour la suite
        enable_colors(true);
        assert(red("x").content == "\033[38;2;255;0;0mx\033[0m");

        // OFF puis composition : doit retourner le texte brut
        enable_colors(false);
        assert(bold(red("Error")).content == "Error");
        assert(bg_hex("#ff0")("text").content == "text");

        // Nettoyage
        enable_colors(true);
    }

    // Vérifier que stdout n'a pas reçu les outputs stderr
    {
        CaptureStream cap_out(std::cout);
        CaptureStream cap_err(std::cerr);

        eprintln("stderr only");
        println("stdout only");

        assert(cap_out.str() == "stdout only\n");
        assert(cap_err.str() == "stderr only\n");
    }

    std::cout << "All phase 7 tests passed!" << std::endl;
    return 0;
}
