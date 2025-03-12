#include "Pack.hpp"
#include "unit_test_framework.hpp"
#include <iostream>
#include <sstream>

using namespace std;

// Test default constructor
TEST(test_pack_default_ctor) {
    Pack pack;
    Card first = pack.deal_one();
    ASSERT_EQUAL(NINE, first.get_rank());
    ASSERT_EQUAL(SPADES, first.get_suit());
}

// Test all cards are initialized correctly
TEST(test_pack_default_order) {
    Pack pack;
    
    // 
    // SPADES
    ASSERT_EQUAL(Card(NINE, SPADES), pack.deal_one());
    ASSERT_EQUAL(Card(TEN, SPADES), pack.deal_one());
    ASSERT_EQUAL(Card(JACK, SPADES), pack.deal_one());
    ASSERT_EQUAL(Card(QUEEN, SPADES), pack.deal_one());
    ASSERT_EQUAL(Card(KING, SPADES), pack.deal_one());
    ASSERT_EQUAL(Card(ACE, SPADES), pack.deal_one());
    
    // HEARTS
    ASSERT_EQUAL(Card(NINE, HEARTS), pack.deal_one());
    ASSERT_EQUAL(Card(TEN, HEARTS), pack.deal_one());
    ASSERT_EQUAL(Card(JACK, HEARTS), pack.deal_one());
    ASSERT_EQUAL(Card(QUEEN, HEARTS), pack.deal_one());
    ASSERT_EQUAL(Card(KING, HEARTS), pack.deal_one());
    ASSERT_EQUAL(Card(ACE, HEARTS), pack.deal_one());
    
    // CLUBS
    ASSERT_EQUAL(Card(NINE, CLUBS), pack.deal_one());
    ASSERT_EQUAL(Card(TEN, CLUBS), pack.deal_one());
    ASSERT_EQUAL(Card(JACK, CLUBS), pack.deal_one());
    ASSERT_EQUAL(Card(QUEEN, CLUBS), pack.deal_one());
    ASSERT_EQUAL(Card(KING, CLUBS), pack.deal_one());
    ASSERT_EQUAL(Card(ACE, CLUBS), pack.deal_one());
    
    // DIAMONDS
    ASSERT_EQUAL(Card(NINE, DIAMONDS), pack.deal_one());
    ASSERT_EQUAL(Card(TEN, DIAMONDS), pack.deal_one());
    ASSERT_EQUAL(Card(JACK, DIAMONDS), pack.deal_one());
    ASSERT_EQUAL(Card(QUEEN, DIAMONDS), pack.deal_one());
    ASSERT_EQUAL(Card(KING, DIAMONDS), pack.deal_one());
    ASSERT_EQUAL(Card(ACE, DIAMONDS), pack.deal_one());
}

// Test istream constructor
TEST(test_pack_istream_ctor) {
    stringstream ss;
    ss << "Nine of Spades" << endl;
    ss << "Ten of Spades" << endl;
    ss << "Jack of Spades" << endl;
    ss << "Queen of Spades" << endl;
    ss << "King of Spades" << endl;
    ss << "Ace of Spades" << endl;
    
    ss << "Nine of Hearts" << endl;
    ss << "Ten of Hearts" << endl;
    ss << "Jack of Hearts" << endl;
    ss << "Queen of Hearts" << endl;
    ss << "King of Hearts" << endl;
    ss << "Ace of Hearts" << endl;
    
    ss << "Nine of Clubs" << endl;
    ss << "Ten of Clubs" << endl;
    ss << "Jack of Clubs" << endl;
    ss << "Queen of Clubs" << endl;
    ss << "King of Clubs" << endl;
    ss << "Ace of Clubs" << endl;
    
    ss << "Nine of Diamonds" << endl;
    ss << "Ten of Diamonds" << endl;
    ss << "Jack of Diamonds" << endl;
    ss << "Queen of Diamonds" << endl;
    ss << "King of Diamonds" << endl;
    ss << "Ace of Diamonds" << endl;
    
    Pack pack(ss);
    
    ASSERT_EQUAL(Card(NINE, SPADES), pack.deal_one());
    ASSERT_EQUAL(Card(TEN, SPADES), pack.deal_one());
    
    ASSERT_EQUAL(Card(JACK, SPADES), pack.deal_one());
    ASSERT_EQUAL(Card(QUEEN, SPADES), pack.deal_one());
}

// Test reset 
TEST(test_pack_reset) {
    Pack pack;
    
    pack.deal_one();
    pack.deal_one();
    pack.deal_one();
    
    pack.reset();
    
    Card first = pack.deal_one();
    ASSERT_EQUAL(NINE, first.get_rank());
    ASSERT_EQUAL(SPADES, first.get_suit());
}

// Test empty 
TEST(test_pack_empty) {
    Pack pack;
    
    // Pack should not be empty 
    ASSERT_FALSE(pack.empty());
    
    // Deal all cards
    for (int i = 0; i < 24; ++i) {
        pack.deal_one();
    }
    
    
    ASSERT_TRUE(pack.empty());
}


TEST(test_deal_all_cards) {
    Pack pack;
    
    // Deal all  cards
    for (int i = 0; i < 24; ++i) {
        pack.deal_one();
    }
    
    
    ASSERT_TRUE(pack.empty());
}

// Test shuffling
TEST(test_pack_shuffle) {
    Pack pack1;
    Pack pack2;
    
    pack2.shuffle();
    
    bool different = false;
    for (int i = 0; i < 24; ++i) {
        if (!(pack1.deal_one() == pack2.deal_one())) {
            different = true;
            break;
        }
    }
    
    // different order
    ASSERT_TRUE(different);
}

// Test shuffling and resetting
TEST(test_shuffle_reset) {
    Pack pack;
    
    pack.deal_one();
    pack.deal_one();
    
    pack.shuffle();
    pack.reset();
    
    for (int i = 0; i < 24; ++i) {
        pack.deal_one();
    }
    
  
    ASSERT_TRUE(pack.empty());
}

// Test multiple shuffles
TEST(test_multiple_shuffles) {
    Pack pack;
    
    Card original = pack.deal_one();
    pack.reset();
    
    // Shuffle multiple times
    for (int i = 0; i < 3; ++i) {
        pack.shuffle();
    }
    
    // First card after shuffles should likely be different
    Card shuffled = pack.deal_one();
    
    // This test might occasionally fail if you get very unlucky
    
    ASSERT_TRUE(original != shuffled); 
}

TEST_MAIN()