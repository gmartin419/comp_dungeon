// COMP 2450 — Warden of the Foundations (Midterm 1)
// battle/Battle.cpp — YOU build the body.
//
// =====================================================================
// REQUIRED MECHANICS
// =====================================================================
// Re-read the public midterm page if anything below is unclear.
//
// MENU (at minimum, four options):
//   1. Attack          — damage the warden; warden retaliates that turn
//   2. Use item        — pick from inventory; effect; turn ends
//   3. Inspect Warden  — print warden's state; FREE action — turn does
//                        NOT end
//   4. Flee            — leave; gate stays closed
//
// END:
//   Victory  when wardenHP <= 0
//   Defeat   when playerHP <= 0
//   Fled     when the player chose Flee
//
// =====================================================================
// FLOOR-TIED REQUIREMENTS — ALL FOUR MUST APPEAR IN YOUR CODE
// =====================================================================
//
//   F0 (ADT)         — the available actions on a turn must live in a
//                      container of your choice. ABOVE the declaration,
//                      in a comment, name the ADT and defend it in one
//                      sentence (the same kind of defence Trial I Q1
//                      practiced — but this is a different collection
//                      than Q1's items menu; the right ADT may differ).
//
//   F1 (search)      — the Use-item branch MUST call findByName<Item>
//                      against hero.inventory to look up the item the
//                      player typed.
//
//   F2 (sort)        — when the items menu is displayed, sort the
//                      inventory at display time with a comparator
//                      (std::sort or your Floor 2 sortInventory). Pick
//                      a criterion (value — the healing-power stand-in
//                      — weight, or name) and document it in a comment.
//
//   F3 (templates +  — invalid menu input must `throw BattleException`
//      exceptions)     (ready skeleton in Battle.h) — or BagException
//                      where a genuinely bad index is the fault —
//                      caught INSIDE the battle loop so the player gets
//                      another prompt — not a crash, not an exit.
//
// =====================================================================
// WHAT THE GRADER WILL DO
// =====================================================================
//   1. cmake --build the project. If it does not compile, the Warden
//      has won by default.
//   2. Type `battle warden`, play through to BOTH a victory and a
//      defeat (or attempt to — items + RNG permitting).
//   3. Type a deliberately invalid menu choice (e.g., "9" for a
//      4-option menu). The game must NOT crash; it must re-prompt.
//   4. Open this file and find each of the four Floor ties. They must
//      be REAL — i.e., the menu actually runs through your container,
//      Use-item actually goes through findByName, the items menu is
//      actually sorted, the throw actually fires on bad input.

#include "Battle.h"

#include <iostream>
#include <string>

#include "../hero/Bag.h"
#include "../hero/BagException.h"
#include "../hero/Item.h"
#include "../bestiary/Search.h"
#include "../hero/Sort.h"

namespace dungeon {

namespace {

    // constexpr means thse values are compile time cinstants
    // k prefix --> means constant
    constexpr int kPlayerStartHP   = 30;
    constexpr int kWardenStartHP   = 50;
    constexpr int kPlayerAttackDmg = 6;   // damage per Attack action
    constexpr int kWardenAttackDmg = 4;   // warden's retaliation damage

    // enum creates a set of named choices 
    // enum class, keeps ou names scoped
    enum class MenuAction {
        Attack,
        UseItem,
        Inspect,
        FLee
    };
    // {1, "Attack", MenuAtction::Attack}

    struct MenuOption {
        int number;        
        std::string label;
        MenuAction action; 
    };
    void printMenu(
        Bag<MenuOption>& menu,
        int playerHP,
        int wardenHP
    ) {
        std::cout << "\n --your turn-- your hp" << playerHP
            << "     warden hp " << wardenHP << "\n";
        for (std::size_t i = 0; i < menu.size(); ++i) {
            std::cout << "    " << menu[i].number << ". "
                << menu[i].label << "\n";
        }
        std::cout << " > ";
    }
    // read the users input and convert it into a menu action
    MenuAction readMenuChoice(const Bag<MenuOption>& menu) {
        std::string line;
        // if problem, kicks you out
        if (!std::getline(std::cin, line)) {
            return MenuAction::FLee;
        }

        int n = -1;
        try {
            // stoi= string or int; "2" --> 2
            // if the strign cant be converted..
            // stoi throw an excp

            n = std::stoi(line);
        }
        catch (...) {
            //catch any exception type
            // we will replace our low level stoi
            // exception with a dimain-specific
            // battleExcepion
            throw BattleException(
                "'" + line + "' is not a menu number (enter 1 to " + std::to_string(menu.size()) + ")"
                );
        }
        // seach the menu for an option whose displayed number matches the number enterd by the player
        for (std::size_t i = 0; i < menu.size(); ++i) {
            if (menu[i].number == n) {
                // return associated matching option
                return menu[i].action;
            }
        }
        //the input was numeric but it did not match a menu option
        throw BagException(
            static_cast<std::size_t>(n),
            menu.size()
        );
    }

    // handle the players use item action
    // hero& will give the function access to the 
    // orignal hero object isntead of a copy
    void useItem(Hero& hero, int& playerHP) {
        // handle missing case
        if (hero.inventory.empty()) {
            std::cout << "your satchel is empty.\n";
            return;
        }
        // sort heros inventory from highest to lowest
        sortInventory(hero, "value desc");
        std::cout << "Choose an item by name:\n";
        printInventory(hero);
        std::cout << " > ";
        std::string name;

        //|| short circut
        // 1. try to read line
        // 2. if that suceeds, then i will check wheter the 
        // line is empty
        // if either is true, the player does nothing
        if (!std::getline(std::cin, name) || name.empty()) {
            std::cout << "you hesitated.\n";
            return;
        }
        // findByName<Item> <-- function-template specialization
        // <Item> will tell the compiler this search will operate
        // on Item objects
        const Item* it = findByName<Item>(hero.inventory, name);

        // a nullptr converts to false
        if (!it) {
            throw BattleException(
                "no item found" + name + " in your satchel"
            );
        }
        // if you say Potion, healing potion
        if (it->name.find("otion") != std::string::npos) {
            // heal 12 hp, but we also dont need our healing 
            // to exced our max health
            playerHP = std::min(
                playerHP + 12,
                kPlayerStartHP
            );
            std::cout << " You drink " <<
                it->name
                << ". HP -> "
                << playerHP
                << ".\n";
        }
        else {
            std::cout << " you ready " << it->name << " - it is not a consumable.\n";
        }
    }
}  // anonymous namespace

BattleOutcome runWardenBattle(Hero& hero) {
    // TODO — write the boss battle. Suggested outline (yours to refactor):
    // create 2 variables for the player and warden health

    int playerHP = kPlayerStartHP;
    int wardenHP = kWardenStartHP;

    //create a bag specialized to store menuOption objects
    Bag<MenuOption> menu;
    menu.push_back({ 1, "Attack", MenuAction::Attack });
    menu.push_back({ 2, "Use Item", MenuAction::UseItem });
    menu.push_back({ 3, "Inspect", MenuAction::Inspect });
    menu.push_back({ 4, "Flee", MenuAction::FLee });

    //continue the battle only while both particapants are alive
    while (playerHP > 0 && wardenHP > 0) {
        try {
            printMenu(menu, playerHP, wardenHP);

            //read menuchouce returens a menuaction
            // switch statement to slect the 
            // corresponding block of code
            switch (readMenuChoice(menu)) {
            case MenuAction::Attack: {
                wardenHP -= kPlayerAttackDmg;
                std::cout << "you tickled the warden for " << kPlayerAttackDmg << ". warden HP -> " << std::max(wardenHP, 0) << ".\n";
            
                // is warden dead
                if (wardenHP > 0) {
                    playerHP -= kWardenAttackDmg;
					std::cout << "The warden spanks back for " << kWardenAttackDmg << ". your HP -> " << std::max(playerHP, 0) << ".\n";
                }
                // break to exit switch case
                break;
            }
            case MenuAction::UseItem: {
                // lets use an item
                useItem(hero, playerHP);
                //using an item consumes our turn
                //the waden will attack, assuming we are both alive
                if (wardenHP > 0 && playerHP > 0) {
                    playerHP -= kWardenAttackDmg;
                    std::cout << "the warden strikes while you fumble. your HP -> "
						<< std::max(playerHP, 0) << ".\n";
                   
                }
                break;
            }
            case MenuAction::Inspect: {
                std::cout << "The warden of the foudnations. HP -> "
                    << wardenHP <<
                    " / "
                    << kPlayerStartHP
                    << ". No visible weakness. (Free action).\n";

                break;
            }
            case MenuAction::FLee: {
                //return to immediatly exit the function
                return BattleOutcome::Fled;
            }
            } // end of switch
        } // try block
        catch (const std::exception& e) {
            //battle and bag exception will inherit this
            std::cout << e.what()
                << " - try again.\n"; 
        }
    }
    // condition ? value_it_true : value_if_false
    return wardenHP <= 0
        ? BattleOutcome::Victory
        : BattleOutcome::Defeat;
    }

}  // namespace dungeon
