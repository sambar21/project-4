#include <iostream>
#include <cassert>
#include "Player.hpp"
#include "Card.hpp"
using namespace std;

// Test specifically focusing on the remaining issue in Hand 7
void test_hand7_herb_selection() {
    cout << "=== HAND 7 SPECIFIC TEST CASE ===\n\n";
    
    // Create Herb player
    Player* herb = Player_factory("Herb", "Simple");
    
    // Add the cards that Herb has in this scenario - before the third trick
    // Based on the correct output, Herb should NOT have Queen of Spades 
    // as it was played in the previous trick
    herb->add_card(Card(NINE, DIAMONDS));
    herb->add_card(Card(NINE, CLUBS));
    herb->add_card(Card(QUEEN, HEARTS)); // trump
    herb->add_card(Card(KING, HEARTS));  // trump
    // Remove Queen of Spades from hand since it was already played
    
    // Set up the scenario where Jack of Spades is led
    Suit trump = HEARTS;
    Card led_card(JACK, SPADES);
    
    cout << "Scenario: Jack of Spades led, Hearts is trump\n";
    cout << "Herb's hand contains:\n";
    cout << "- Nine of Diamonds\n";
    cout << "- Nine of Clubs\n";
    cout << "- Queen of Hearts (trump)\n";
    cout << "- King of Hearts (trump)\n";
    cout << "The Queen of Spades was played in previous trick\n\n";
    
    cout << "Expected behavior: Herb cannot follow Spades (since he has no Spades),\n";
    cout << "                   should play Nine of Diamonds\n\n";
    
    // Test what Herb actually plays
    Card played = herb->play_card(led_card, trump);
    cout << "Herb played: " << played << "\n";
    cout << "Expected: Nine of Diamonds\n\n";
    
    // Test direct card comparison
    Card nine_diamonds(NINE, DIAMONDS);
    Card nine_clubs(NINE, CLUBS);
    
    cout << "Direct Card_less comparison:\n";
    cout << "Nine of Diamonds < Nine of Clubs? " 
         << Card_less(nine_diamonds, nine_clubs, led_card, trump) << "\n";
    cout << "Nine of Clubs < Nine of Diamonds? " 
         << Card_less(nine_clubs, nine_diamonds, led_card, trump) << "\n\n";
    
    // Test with different led cards
    Card led_hearts(ACE, HEARTS);
    cout << "With Hearts led (Ace of Hearts):\n";
    cout << "Nine of Diamonds < Nine of Clubs? " 
         << Card_less(nine_diamonds, nine_clubs, led_hearts, trump) << "\n";
    
    Card led_diamonds(KING, DIAMONDS);
    cout << "With Diamonds led (King of Diamonds):\n";
    cout << "Nine of Diamonds < Nine of Clubs? " 
         << Card_less(nine_diamonds, nine_clubs, led_diamonds, trump) << "\n";
    
    Card led_clubs(QUEEN, CLUBS);
    cout << "With Clubs led (Queen of Clubs):\n";
    cout << "Nine of Diamonds < Nine of Clubs? " 
         << Card_less(nine_diamonds, nine_clubs, led_clubs, trump) << "\n";
}

// Simulate the exact hand 7 scenario with all players
void simulate_hand7_third_trick() {
    cout << "\n=== FULL HAND 7 THIRD TRICK SIMULATION ===\n\n";
    
    // Create all players
    Player* gabriel = Player_factory("Gabriel", "Simple");
    Player* herb = Player_factory("Herb", "Simple");
    Player* edsger = Player_factory("Edsger", "Simple");
    Player* fran = Player_factory("Fran", "Simple");
    
    // Set up the cards each player has at this point in the game
    // Gabriel has played Jack of Spades to lead
    
    // Herb's hand - NO Queen of Spades (already played in the 2nd trick)
    herb->add_card(Card(NINE, DIAMONDS));
    herb->add_card(Card(NINE, CLUBS));
    herb->add_card(Card(QUEEN, HEARTS)); // trump
    herb->add_card(Card(KING, HEARTS));  // trump
    
    // Edsger's hand (approximate based on what we know)
    edsger->add_card(Card(QUEEN, CLUBS));
    edsger->add_card(Card(KING, CLUBS));
    edsger->add_card(Card(NINE, HEARTS)); // trump
    
    // Fran's hand (approximate based on what we know)
    fran->add_card(Card(TEN, CLUBS));
    fran->add_card(Card(JACK, CLUBS));
    fran->add_card(Card(TEN, HEARTS)); // trump
    
    // Set up the trick
    Suit trump = HEARTS;
    Card led_card(JACK, SPADES);
    
    cout << "Jack of Spades led by Gabriel\n";
    
    // Herb plays
    Card herb_card = herb->play_card(led_card, trump);
    cout << herb_card << " played by Herb\n";
    
    // Edsger plays
    Card edsger_card = edsger->play_card(led_card, trump);
    cout << edsger_card << " played by Edsger\n";
    
    // Fran plays
    Card fran_card = fran->play_card(led_card, trump);
    cout << fran_card << " played by Fran\n";
    
    cout << "\nExpected play: Nine of Diamonds played by Herb\n";
}

int main() {
    // Run tests
    test_hand7_herb_selection();
    simulate_hand7_third_trick();
    
    return 0;
}