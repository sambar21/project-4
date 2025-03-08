#include "Card.hpp"
#include "unit_test_framework.hpp"
#include <iostream>

using namespace std;

TEST(test_card_ctor) {
    Card c(ACE, HEARTS);
    ASSERT_EQUAL(ACE, c.get_rank());
    ASSERT_EQUAL(HEARTS, c.get_suit());
}

// Add more test cases here
TEST(test_default_constructor){
    Card c;
    ASSERT_EQUAL(TWO,c.get_rank());
    ASSERT_EQUAL(SPADES,c.get_suit());
}
TEST(test_variable_constructor){
    Card c(JACK,DIAMONDS);
    ASSERT_EQUAL(JACK, c.get_rank());
    ASSERT_EQUAL(DIAMONDS, c.get_suit());
}
TEST(test_is_face){
    Card c(JACK,DIAMONDS);
    Card c2(FIVE,DIAMONDS);
    ASSERT_EQUAL(true, c.is_face_or_ace());
    ASSERT_EQUAL(false, c2.is_face_or_ace());
}
TEST(test_trump_card){
    Card c(JACK,SPADES);
    ASSERT_EQUAL(c.is_trump(SPADES),true);
    ASSERT_EQUAL(c.is_right_bower(SPADES),true);
    ASSERT_EQUAL(c.is_left_bower(CLUBS),true);
}
TEST(test_input_stream){
    Card ace(ACE,HEARTS);
    string str = "Ace of Hearts";
    istringstream i(str);
    Card clone;
    i>>clone;
    ASSERT_EQUAL(clone==ace,true);
}
TEST(test_bool_comparisons){
    Card jack(JACK,SPADES);
    Card num(FIVE,HEARTS);
    ASSERT_EQUAL(jack > num,true);
    ASSERT_EQUAL(jack >= num,true);
    ASSERT_EQUAL(num == jack,false);
    ASSERT_EQUAL(num != jack,true);
    ASSERT_EQUAL(jack < num,false);
    ASSERT_EQUAL(jack <= num,false);
}
TEST(test_Suit_next){
    ASSERT_EQUAL(Suit_next(SPADES)==CLUBS,true);
    ASSERT_EQUAL(Suit_next(HEARTS)==DIAMONDS,true);
}
TEST(test_Card_less){
    Card jack(JACK,SPADES);
    Card num(FIVE,HEARTS);
    ASSERT_EQUAL(Card_less(num,jack,SPADES),true);
    ASSERT_EQUAL(Card_less(num,jack,HEARTS),false);
}
TEST(test_get_suit) {
    // non-trump card should return its own suit
    Card c1(NINE, HEARTS);
    ASSERT_EQUAL(c1.get_suit(SPADES), HEARTS);

    //  Jack of trump suit should return the same trump suit
    Card c2(JACK, SPADES);
    ASSERT_EQUAL(c2.get_suit(SPADES), SPADES);

    // Jack of the other suit of the same color should return trump
    Card c3(JACK, CLUBS);
    ASSERT_EQUAL(c3.get_suit(SPADES), SPADES);  // Should return SPADES, not CLUBS

    // Jack of the other red suit should return trump
    Card c4(JACK, DIAMONDS);
    ASSERT_EQUAL(c4.get_suit(HEARTS), HEARTS);  // Should return HEARTS, not DIAMONDS

    // Non-trump face card  should return its own suit
    Card c5(KING, CLUBS);
    ASSERT_EQUAL(c5.get_suit(SPADES), CLUBS);
}

TEST(test_is_face_or_ace) {
    Card ace(ACE, HEARTS);
    Card king(KING, CLUBS);
    Card queen(QUEEN, DIAMONDS);
    Card jack(JACK, SPADES);
    Card ten(TEN, HEARTS);
    Card five(FIVE, DIAMONDS);
    
    // Ensure all face cards and ACE return true
    ASSERT_EQUAL(ace.is_face_or_ace(), true);
    ASSERT_EQUAL(king.is_face_or_ace(), true);
    ASSERT_EQUAL(queen.is_face_or_ace(), true);
    ASSERT_EQUAL(jack.is_face_or_ace(), true);
    
    // Ensure non-face cards return false
    ASSERT_EQUAL(ten.is_face_or_ace(), false);
    ASSERT_EQUAL(five.is_face_or_ace(), false);
}

TEST(test_Card_less_trump_bower) {
    Card right_bower(JACK, SPADES);
    Card left_bower(JACK, CLUBS);
    Card other_trump(KING, SPADES);
    
    ASSERT_EQUAL(Card_less(other_trump, right_bower, SPADES), true);
    ASSERT_EQUAL(Card_less(left_bower, right_bower, SPADES), true);
    ASSERT_EQUAL(Card_less(right_bower, left_bower, SPADES), false);
}

TEST(test_operator_output) {
    Card c(QUEEN, DIAMONDS);
    ostringstream os;
    os << c;
    ASSERT_EQUAL(os.str(), "Queen of Diamonds");
}


TEST(test_is_trump_comprehensive) {
    // Test right bower
    Card right_bower(JACK, HEARTS);
    ASSERT_EQUAL(right_bower.is_trump(HEARTS), true);
    
    // Test left bower 
    Card left_bower_diff_color(JACK, DIAMONDS);
    ASSERT_EQUAL(left_bower_diff_color.is_trump(HEARTS), true);
    
    // Test left bower 
    Card left_bower_same_color(JACK, CLUBS);
    ASSERT_EQUAL(left_bower_same_color.is_trump(SPADES), true);
    
    // Test regular trump card
    Card regular_trump(QUEEN, HEARTS);
    ASSERT_EQUAL(regular_trump.is_trump(HEARTS), true);
    
    // Test non trump card
    Card non_trump(KING, CLUBS);
    ASSERT_EQUAL(non_trump.is_trump(HEARTS), false);
    
    // Test jack of different suit and color
    Card different_jack(JACK, SPADES);
    ASSERT_EQUAL(different_jack.is_trump(HEARTS), false);
}

TEST(test_Card_less_led_suit_comprehensive) {
    Suit trump = SPADES;
    
    // Test  both cards follow led suit
    Card led_card(NINE, HEARTS);
    Card low_led_suit(SEVEN, HEARTS);
    Card high_led_suit(KING, HEARTS);
    
    ASSERT_EQUAL(Card_less(low_led_suit, high_led_suit, led_card, trump), true);
    ASSERT_EQUAL(Card_less(high_led_suit, low_led_suit, led_card, trump), false);
    
    // Test  one card is trump  other follows led suit
    Card trump_card(TEN, SPADES);
    ASSERT_EQUAL(Card_less(high_led_suit, trump_card, led_card, trump), true);
    ASSERT_EQUAL(Card_less(trump_card, high_led_suit, led_card, trump), false);
    
    // Test  neither card follows led suit and one is trump
    Card non_led_non_trump(QUEEN, DIAMONDS);
    ASSERT_EQUAL(Card_less(non_led_non_trump, trump_card, led_card, trump), true);
    ASSERT_EQUAL(Card_less(trump_card, non_led_non_trump, led_card, trump), false);
    
    // Test  neither card follows led suit and neither is trump
    Card other_non_led(KING, CLUBS);
    ASSERT_EQUAL(Card_less(non_led_non_trump, other_non_led, led_card, trump), true);
    ASSERT_EQUAL(Card_less(other_non_led, non_led_non_trump, led_card, trump), false);
    
    // Test  bower as led card
    Card right_bower(JACK, SPADES);
    Card left_bower(JACK, CLUBS);
    ASSERT_EQUAL(Card_less(high_led_suit, right_bower, led_card, trump), true);
    ASSERT_EQUAL(Card_less(left_bower, right_bower, led_card, trump), true);
}

TEST(test_left_bower_edge_cases) {
    // Test left bower  different trump suits
    Card jack_clubs(JACK, CLUBS);
    Card jack_spades(JACK, SPADES);
    Card jack_hearts(JACK, HEARTS);
    Card jack_diamonds(JACK, DIAMONDS);
    
    // Check if jack of clubs is left bower when spades is trump
    ASSERT_EQUAL(jack_clubs.is_left_bower(SPADES), true);
    ASSERT_EQUAL(jack_clubs.is_trump(SPADES), true);
    
    // Check if jack of spades is left bower when clubs is trump
    ASSERT_EQUAL(jack_spades.is_left_bower(CLUBS), true);
    ASSERT_EQUAL(jack_spades.is_trump(CLUBS), true);
    
    // Check if jack of diamonds is left bower when hearts is trump
    ASSERT_EQUAL(jack_diamonds.is_left_bower(HEARTS), true);
    ASSERT_EQUAL(jack_diamonds.is_trump(HEARTS), true);
    
    // Check if jack of hearts is left bower when diamonds is trump
    ASSERT_EQUAL(jack_hearts.is_left_bower(DIAMONDS), true);
    ASSERT_EQUAL(jack_hearts.is_trump(DIAMONDS), true);
    
    // Verify that  jack is not a left bower for suits of different color
    ASSERT_EQUAL(jack_hearts.is_left_bower(SPADES), false);
    ASSERT_EQUAL(jack_spades.is_left_bower(HEARTS), false);
}

TEST(test_Card_less_with_bowers) {
    Suit trump = SPADES;
    
    Card right_bower(JACK, SPADES);
    Card left_bower(JACK, CLUBS);
    Card ace_trump(ACE, SPADES);
    Card king_trump(KING, SPADES);
    Card queen_trump(QUEEN, SPADES);
    Card non_trump_ace(ACE, HEARTS);
    
    // Right bower beats left bower
    ASSERT_EQUAL(Card_less(left_bower, right_bower, trump), true);
    ASSERT_EQUAL(Card_less(right_bower, left_bower, trump), false);
    
    // Left bower beats all other trump cards
    ASSERT_EQUAL(Card_less(ace_trump, left_bower, trump), true);
    ASSERT_EQUAL(Card_less(left_bower, ace_trump, trump), false);
    
    // Right and left bower beat non-trump aces
    ASSERT_EQUAL(Card_less(non_trump_ace, right_bower, trump), true);
    ASSERT_EQUAL(Card_less(non_trump_ace, left_bower, trump), true);
    
    // Test the ordering of trump cards (after bowers)
    ASSERT_EQUAL(Card_less(king_trump, ace_trump, trump), true);
    ASSERT_EQUAL(Card_less(queen_trump, king_trump, trump), true);
}
TEST(test_is_trump_left_bower_bug) {
    
    Card jack_spades(JACK, SPADES);
    ASSERT_EQUAL(jack_spades.is_trump(SPADES), true);
    
   
    Card jack_clubs(JACK, CLUBS);
    ASSERT_EQUAL(jack_clubs.is_trump(SPADES), true);
    
    Card jack_hearts(JACK, HEARTS);
    ASSERT_EQUAL(jack_hearts.is_trump(SPADES), false);
    
    Card king_spades(KING, SPADES);
    ASSERT_EQUAL(king_spades.is_trump(SPADES), true);
    
    Card queen_hearts(QUEEN, HEARTS);
    ASSERT_EQUAL(queen_hearts.is_trump(SPADES), false);
    
    
    ASSERT_EQUAL(jack_clubs.is_trump(SPADES), true);
    
    ASSERT_EQUAL(jack_spades.is_trump(CLUBS), true);
    
    Card jack_diamonds(JACK, DIAMONDS);
    ASSERT_EQUAL(jack_diamonds.is_trump(HEARTS), true);
    
    Card jack_hearts2(JACK, HEARTS);
    ASSERT_EQUAL(jack_hearts2.is_trump(DIAMONDS), true);
}
TEST_MAIN()
