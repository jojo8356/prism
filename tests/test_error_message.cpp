// Ce fichier NE DOIT PAS compiler.
// Il sert à vérifier la qualité du message d'erreur static_assert.
// Compiler avec : g++ -std=c++17 -I include tests/test_error_message.cpp
// Résultat attendu : message clair indiquant comment rendre le type printable.

#include <prism/prism.hpp>

struct Unprintable {
    int secret;
    // Pas d'operator<< ni de Formatter
};

int main() {
    using namespace prism;
    Unprintable u{42};
    println(u);  // Doit produire un static_assert clair
    return 0;
}
