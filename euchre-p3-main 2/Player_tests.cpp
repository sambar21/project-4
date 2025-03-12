#include "Player.hpp"
#include "unit_test_framework.hpp"
#include <iostream>

using namespace std;

// Test player names
TEST(test_player_get_name) {
    Player * alice = Player_factory("Alice", "Simple");
    Player * bob = Player_factory("Bob", "Human");
    
    ASSERT_EQUAL("Alice", alice->get_name());
    ASSERT_EQUAL("Bob", bob->get_name());
    
    delete alice;
    delete bob;
}

// Test player factory
TEST(test_player_factory) {
    Player * simple = Player_factory("Simple Player", "Simple");
    Player * human = Player_factory("Human Player", "Human");
    
    ASSERT_NOT_EQUAL(nullptr, simple);
    ASSERT_NOT_EQUAL(nullptr, human);
    
    delete simple;
    delete human;
}

// Test adding cards
TEST(test_add_card) {
    Player * player = Player_factory("Test", "Simple");
    Card c1(ACE, HEARTS);
    Card c2(KING, SPADES);
    
    player->add_card(c1);
    player->add_card(c2);
    
    // We can't directly test hand contents since it's private
    // but we can test that the player can still accept cards
    Card c3(QUEEN, DIAMONDS);
    player->add_card(c3);
    
    delete player;
}

// Test make trump round 1 for Simple player
TEST(test_simple_make_trump_round1) {
    Player * player = Player_factory("Simple", "Simple");
    
    // Add strong trump cards
    player->add_card(Card(JACK, HEARTS));  // Right bower
    player->add_card(Card(JACK, DIAMONDS)); // Left bower
    player->add_card(Card(ACE, HEARTS));   // Strong trump
    
    Suit trump = SPADES;  // Initial value
    bool order_up = player->make_trump(
        Card(NINE, HEARTS),  // Upcard
        false,              // Is dealer
        1,                  // Round
        trump               // Order up suit
    );
    
    ASSERT_TRUE(order_up);
    ASSERT_EQUAL(HEARTS, trump);
    
    delete player;
}

// Test make trump round 2 for Simple player
TEST(test_simple_make_trump_round2) {
    Player * player = Player_factory("Simple", "Simple");
    
    // Add weak cards in next suit
    player->add_card(Card(NINE, DIAMONDS));
    player->add_card(Card(TEN, DIAMONDS));
    player->add_card(Card(QUEEN, SPADES));
    
    Suit trump = SPADES;  // Initial value
    bool order_up = player->make_trump(
        Card(NINE, HEARTS),  // Upcard
        true,               // Is dealer
        2,                  // Round
        trump               // Order up suit
    );
    
    ASSERT_TRUE(order_up);  // Should order up as dealer
    ASSERT_EQUAL(DIAMONDS, trump);  // Next suit after Hearts
    
    delete player;
}

// Test add and discard

TEST(test_add_and_discard_with_proper_comparison) {
    Player *player = Player_factory("Simple", "Simple");
    
   
    player->add_card(Card(NINE, HEARTS));       // non trump, low
    player->add_card(Card(JACK, CLUBS));        // left bower in spades
    player->add_card(Card(ACE, DIAMONDS));      // non trump high
    
    // upcard
    Card upcard(QUEEN, SPADES);
    player->add_and_discard(upcard);
    
    
    Card played1 = player->lead_card(SPADES);
    ASSERT_EQUAL(ACE, played1.get_rank());
    ASSERT_EQUAL(DIAMONDS, played1.get_suit());
    
    Card played2 = player->lead_card(SPADES);
    ASSERT_EQUAL(JACK, played2.get_rank());
    ASSERT_EQUAL(CLUBS, played2.get_suit());
    
    Card played3 = player->lead_card(SPADES);
    ASSERT_EQUAL(QUEEN, played3.get_rank());
    ASSERT_EQUAL(SPADES, played3.get_suit());
    
    delete player;
}

TEST(test_add_and_discard_with_multiple_trumps_fixed) {
    Player *player = Player_factory("Simple", "Simple");
    
    player->add_card(Card(NINE, SPADES));  // Weak trump
    player->add_card(Card(TEN, SPADES));   // Weak trump
    player->add_card(Card(KING, HEARTS));  // Strong non-trump
    
    Card upcard(ACE, SPADES);
    player->add_and_discard(upcard);
   
    Card card1 = player->lead_card(SPADES);
    Card card2 = player->lead_card(SPADES);
    Card card3 = player->lead_card(SPADES);
    
    // Modified assertions to expect all trump cards (no non-trump)
    bool has_all_trump = 
        (card1.is_trump(SPADES)) && 
        (card2.is_trump(SPADES)) && 
        (card3.is_trump(SPADES));
    
    ASSERT_TRUE(has_all_trump);
    
    bool has_ace_spades = 
        (card1.get_rank() == ACE && card1.get_suit() == SPADES) || 
        (card2.get_rank() == ACE && card2.get_suit() == SPADES) || 
        (card3.get_rank() == ACE && card3.get_suit() == SPADES);
    
    ASSERT_TRUE(has_ace_spades);
    
    int spades_count = 0;
    if (card1.get_suit() == SPADES) spades_count++;
    if (card2.get_suit() == SPADES) spades_count++;
    if (card3.get_suit() == SPADES) spades_count++;
    
    ASSERT_EQUAL(3, spades_count);  // Changed from 2 to 3
    
    delete player;
}


// Test lead card
TEST(test_lead_card) {
    Player * player = Player_factory("Simple", "Simple");
    
    // Add mix of trump and non-trump cards
    player->add_card(Card(ACE, HEARTS));   // Non-trump
    player->add_card(Card(KING, SPADES));  // Trump
    player->add_card(Card(QUEEN, HEARTS)); // Non-trump
    
    Card led = player->lead_card(SPADES);  // Spades is trump
    
    // Should lead highest non-trump if possible
    ASSERT_EQUAL(ACE, led.get_rank());
    ASSERT_EQUAL(HEARTS, led.get_suit());
    
    delete player;
}

// Test play card following suit
TEST(test_play_card_follow_suit) {
    Player * player = Player_factory("Simple", "Simple");
    
    // Add cards of led suit
    player->add_card(Card(KING, HEARTS));
    player->add_card(Card(QUEEN, HEARTS));
    player->add_card(Card(JACK, SPADES));
    
    Card played = player->play_card(
        Card(NINE, HEARTS),  // Led card
        SPADES               // Trump suit
    );
    
    // Should play highest card of led suit
    ASSERT_EQUAL(KING, played.get_rank());
    ASSERT_EQUAL(HEARTS, played.get_suit());
    
    delete player;
}

// Test play card when can't follow suit
TEST(test_play_card_cant_follow) {
    Player * player = Player_factory("Simple", "Simple");
    
    // Add cards not of led suit
    player->add_card(Card(ACE, SPADES));   // Trump
    player->add_card(Card(KING, SPADES));  // Trump
    player->add_card(Card(QUEEN, DIAMONDS));
    
    Card played = player->play_card(
        Card(NINE, HEARTS),  // Led card
        SPADES               // Trump suit
    );
    
    // Should play lowest card when can't follow suit
    ASSERT_EQUAL(QUEEN, played.get_rank());
    ASSERT_EQUAL(DIAMONDS, played.get_suit());
    
    delete player;
}

// Test edge case with max hand size
TEST(test_max_hand_size) {
    Player * player = Player_factory("Simple", "Simple");
    
    // Add maximum allowed cards
    for(int i = 0; i < Player::MAX_HAND_SIZE; i++) {
        player->add_card(Card(NINE, SPADES));
    }
    
    delete player;
}

// Test left bower behavior
TEST(test_left_bower) {
    Player * player = Player_factory("Simple", "Simple");
    
    // Add left bower and other cards
    player->add_card(Card(JACK, DIAMONDS)); // Left bower when Hearts is trump
    player->add_card(Card(ACE, HEARTS));
    
    Suit trump = SPADES;
    bool order_up = player->make_trump(
        Card(NINE, HEARTS),
        false,
        1,
        trump
    );
    
    ASSERT_TRUE(order_up);
    ASSERT_EQUAL(HEARTS, trump);
    
    delete player;
}
TEST(test_add_and_discard_trump_comparison_complex) {
    Player *player = Player_factory("Simple", "Simple");
    
    player->add_card(Card(NINE, HEARTS));     // Low non-trump
    player->add_card(Card(NINE, CLUBS));      // Low non-trump
    player->add_card(Card(QUEEN, DIAMONDS));  // Medium non-trump
    player->add_card(Card(KING, DIAMONDS));   // High non-trump
    player->add_card(Card(ACE, CLUBS));       // High non-trump
    
    Card upcard(QUEEN, SPADES);
    player->add_and_discard(upcard);
    
   
    
    int hearts_nines = 0;
    int clubs_nines = 0;
    
    for (int i = 0; i < 5; i++) {
        Card played = player->lead_card(SPADES);
        
        if (played.get_rank() == NINE && played.get_suit() == HEARTS) {
            hearts_nines++;
        }
        if (played.get_rank() == NINE && played.get_suit() == CLUBS) {
            clubs_nines++;
        }
    }
    
    ASSERT_EQUAL(1, hearts_nines + clubs_nines);
    
    delete player;
}
TEST(test_add_and_discard_left_bower_is_trump) {
    Player *player = Player_factory("Simple", "Simple");

    player->add_card(Card(JACK, DIAMONDS)); // Left Bower when HEARTS is trump
    player->add_card(Card(ACE, HEARTS));    // Trump
    player->add_card(Card(QUEEN, CLUBS));   // Non-trump

    Card upcard(KING, HEARTS); // Trump suit is HEARTS
    player->add_and_discard(upcard);

    // Ensure QUEEN of CLUBS (non-trump, lowest) is discarded
    Card played1 = player->lead_card(HEARTS);
    Card played2 = player->lead_card(HEARTS);

    ASSERT_NOT_EQUAL(QUEEN, played1.get_rank());
    ASSERT_NOT_EQUAL(CLUBS, played1.get_suit());
    ASSERT_NOT_EQUAL(QUEEN, played2.get_rank());
    ASSERT_NOT_EQUAL(CLUBS, played2.get_suit());

    delete player;
}
TEST(test_add_and_discard_weakest_trump_if_no_non_trump) {
    Player *player = Player_factory("Simple", "Simple");

    player->add_card(Card(NINE, SPADES));   // Weakest trump
    player->add_card(Card(TEN, SPADES));    // Stronger trump
    player->add_card(Card(JACK, CLUBS));    // Left Bower (treated as trump)

    Card upcard(KING, SPADES);  // Trump suit is SPADES
    player->add_and_discard(upcard);

    // Check that the weakest trump (NINE of SPADES) was discarded
    Card played1 = player->lead_card(SPADES);
    Card played2 = player->lead_card(SPADES);

    // Ensure the hand no longer contains NINE of SPADES
    ASSERT_NOT_EQUAL(NINE, played1.get_rank());
    ASSERT_NOT_EQUAL(NINE, played2.get_rank());

    delete player;
}

TEST(test_add_and_discard_prefers_non_trump) {
    Player *player = Player_factory("Simple", "Simple");

    player->add_card(Card(NINE, HEARTS));   // Non-trump, lowest
    player->add_card(Card(TEN, SPADES));    // Trump
    player->add_card(Card(KING, SPADES));   // Trump

    Card upcard(ACE, SPADES); // Trump suit is SPADES
    player->add_and_discard(upcard);

    // Ensure NINE of HEARTS (non-trump, lowest) is discarded
    Card played1 = player->lead_card(SPADES);
    Card played2 = player->lead_card(SPADES);
    
    ASSERT_NOT_EQUAL(NINE, played1.get_rank());
    ASSERT_NOT_EQUAL(HEARTS, played1.get_suit());
    ASSERT_NOT_EQUAL(NINE, played2.get_rank());
    ASSERT_NOT_EQUAL(HEARTS, played2.get_suit());

    delete player;
}

TEST_MAIN()