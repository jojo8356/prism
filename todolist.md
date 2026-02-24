# Prism — Todolist d'implémentation

## Phase 1 : Fondations

- [x] **1.1** Créer le squelette du header `include/prism/prism.hpp` (include guards, namespace, forward declarations)
- [x] **1.2** Implémenter le trait `Formatter<T>` — mécanisme de dispatch : spécialisation > `operator<<` > `static_assert`
- [x] **1.3** Implémenter `to_string_value(T)` — conversion universelle d'une valeur en string via le trait
- [x] **1.4** Implémenter `print(args...)` et `println(args...)` — mode variadic avec séparateur espace

## Phase 2 : Format strings

- [x] **2.1** Implémenter le parsing de format string — détection `{}`, échappement `{{`/`}}`
- [x] **2.2** Implémenter `print("fmt", args...)` — mode format string avec remplacement séquentiel des `{}`
- [x] **2.3** Gestion d'erreur — `{?}` pour arguments manquants, arguments excédentaires ignorés

## Phase 3 : Types STL

- [x] **3.1** `Formatter` pour containers séquentiels — `vector`, `array`, `list`, `deque` → `[1, 2, 3]`
- [x] **3.2** `Formatter` pour containers associatifs — `map`, `unordered_map` → `{a: 1, b: 2}`
- [x] **3.3** `Formatter` pour sets — `set`, `unordered_set` → `{1, 2, 3}`
- [x] **3.4** `Formatter` pour `pair` et `tuple` → `(1, "hello")`
- [x] **3.5** `Formatter` pour `optional` → `Some(42)` / `None`
- [x] **3.6** `Formatter` pour `variant` → valeur active

## Phase 4 : Système de couleurs — Core

- [x] **4.1** Implémenter `ColorWrapper` — struct qui stocke les codes ANSI et wrappe du texte
- [x] **4.2** Implémenter `rgb(r, g, b)` → retourne un `ColorWrapper` callable
- [x] **4.3** Implémenter `hex("#RRGGBB")` et `hex("#RGB")` — parsing hex → RGB
- [x] **4.4** Implémenter `hsl(h, s, l)` — conversion HSL → RGB
- [x] **4.5** Implémenter `oklch(l, c, h)` — conversion OKLCH → sRGB (via OKLab intermédiaire)
- [x] **4.6** Intégrer les `ColorWrapper` dans le système de print (les rendre printable)

## Phase 5 : Couleurs — Background et styles

- [x] **5.1** Implémenter `bg_rgb()`, `bg_hex()`, `bg_hsl()`, `bg_oklch()` — fond coloré via `\033[48;2;R;G;Bm`
- [x] **5.2** Implémenter les styles texte — `bold()`, `dim()`, `italic()`, `underline()`, `strikethrough()`
- [x] **5.3** Implémenter la composition de styles — `bold(red("text"))`, emboîtement correct des escape codes

## Phase 6 : 148 couleurs CSS nommées

- [x] **6.1** Définir la table des 148 couleurs CSS (nom → RGB)
- [x] **6.2** Générer les 148 fonctions : `red()`, `blue()`, `coral()`, `dodgerblue()`, etc.
- [x] **6.3** Générer les 148 fonctions background : `bg_red()`, `bg_blue()`, etc.
- [x] **6.4** Implémenter `css("colorname")` — lookup dynamique par nom string

## Phase 7 : Fonctionnalités complémentaires

- [x] **7.1** Implémenter `print_sep()` / `println_sep()` — séparateur configurable
- [x] **7.2** Implémenter `eprint()` / `eprintln()` — sortie stderr
- [x] **7.3** Détection auto du support couleur — TTY check, `NO_COLOR`, `TERM=dumb`
- [x] **7.4** Implémenter `prism::enable_colors(bool)` — forçage manuel

## Phase 8 : Tests et exemples

- [x] **8.1** Écrire `examples/demo.cpp` — showcase de toutes les features
- [x] **8.2** Écrire `tests/test_prism.cpp` — tests unitaires (print, format, couleurs, types)
- [x] **8.3** Vérifier la compilation en C++17 strict (`-std=c++17 -Wall -Wextra -Werror -Wpedantic`)

## Phase 9 : Polish

- [x] **9.1** Vérifier la qualité des messages d'erreur de compilation pour types non-printable
- [x] **9.2** Optimisation — minimiser les allocations, utiliser `constexpr` où possible
- [x] **9.3** Tester sur Linux, macOS, Windows (ANSI support)
