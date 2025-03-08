#include "Player.hpp"
#include <cassert>
#include <iostream>
#include <array>
#include <algorithm>

using namespace std;

class Simple: public Player {
public:
    Simple(const string& inname) {
        name = inname;
    }
    
    const string& get_name() const override {
        return name;
    }
    
    void add_card(const Card &c) override {
        assert(hand.size() < MAX_HAND_SIZE);
        hand.push_back(c);
    }
    
    bool make_trump(const Card &upcard, bool is_dealer,
                   int round, Suit &order_up_suit) const override {
        assert(round == 1 || round == 2);

        if(round == 1) {
            int numCardsTrumpFace = 0; 
            Suit upcardsuit = upcard.get_suit();
            for(int i = 0; i < hand.size(); i++) {
                if((hand[i].is_trump(upcardsuit) && hand[i].is_face_or_ace())
                   || hand[i].is_left_bower(upcardsuit) 
                   || hand[i].is_right_bower(upcardsuit)) {
                    numCardsTrumpFace++;
                }
            }
            if(numCardsTrumpFace >= 2) {
                order_up_suit = upcard.get_suit();
                return true;
            }
            return false;
        }
        
        if(round == 2) {
            Suit next_suit = Suit_next(upcard.get_suit());
            if(is_dealer) {
                order_up_suit = next_suit;
                return true;
            }
            for(int i = 0; i < hand.size(); i++) {
                if((hand[i].is_trump(next_suit) && hand[i].is_face_or_ace())
                   || hand[i].is_left_bower(next_suit) 
                   || hand[i].is_right_bower(next_suit)) {
                    order_up_suit = next_suit;
                    return true;
                }
            }
        }
        return false;
    }
    
    void add_and_discard(const Card &upcard) override {
        assert(hand.size() >= 1);
        hand.push_back(upcard);
        
        // Get the suit that's actually trump (the upcard's suit)
        Suit trump_suit = upcard.get_suit();
        
        // Find the lowest card considering Euchre rules
        Card lowest = hand[0];
        int index = 0;
        for(int i = 1; i < hand.size(); i++) {
            if(Card_less(hand[i], lowest, trump_suit)) {
                lowest = hand[i];
                index = i;
            }
        }
        
        hand.erase(hand.begin() + index);
    }
    
    Card lead_card(Suit trump) override {
        assert(hand.size() >= 1);
        
        // Check if we have any non-trump cards
        bool have_non_trump = false;
        for(const Card &c : hand) {
            if(!c.is_trump(trump)) {
                have_non_trump = true;
                break;
            }
        }
        
        // Find the highest card to play based on our strategy
        int play_index = 0;
        
        for(int i = 1; i < hand.size(); i++) {
            // If we have non-trump cards, skip trump cards
            if(have_non_trump && hand[i].is_trump(trump)) {
                continue;
            }
            
            // If current highest is a trump card but we should play non-trump, update
            if(have_non_trump && hand[play_index].is_trump(trump)) {
                play_index = i;
                continue;
            }
            
            // Compare cards - if hand[i] is higher, update our selection
            if(Card_less(hand[play_index], hand[i], trump)) {
                play_index = i;
            }
        }
        
        Card to_play = hand[play_index];
        hand.erase(hand.begin() + play_index);
        return to_play;
    }
    Card play_card(const Card &led_card, Suit trump) override {
        assert(hand.size() >= 1);
        
        // First determine if we can follow suit
        bool can_follow_suit = false;
        Suit led_suit = led_card.get_suit(trump);
        
        for(const Card &c : hand) {
            // Check if card's effective suit matches led suit
            if(c.get_suit(trump) == led_suit) {
                can_follow_suit = true;
                break;
            }
        }
        
        if(can_follow_suit) {
            // Play highest card of the led suit
            Card highest;
            int highest_index = -1;
            
            for(int i = 0; i < hand.size(); i++) {
                if(hand[i].get_suit(trump) == led_suit) {
                    if(highest_index == -1 || Card_less(highest, hand[i], led_card, trump)) {
                        highest = hand[i];
                        highest_index = i;
                    }
                }
            }
            
            hand.erase(hand.begin() + highest_index);
            return highest;
        } else {
            // Play lowest card (considering trump)
            Card lowest = hand[0];
            int lowest_index = 0;
            
            for(int i = 1; i < hand.size(); i++) {
                if(Card_less(hand[i], lowest, led_card, trump)) {
                    lowest = hand[i];
                    lowest_index = i;
                }
            }
            
            hand.erase(hand.begin() + lowest_index);
            return lowest;
        }
    }
    
private: 
    std::string name;
    std::vector<Card> hand;
};

class Human: public Player {
public:
    Human(const string& inname) {
        name = inname;
    }
    
    const string& get_name() const override {
        return name;
    }
    
    void add_card(const Card &c) override {
        assert(hand.size() < MAX_HAND_SIZE);
        hand.push_back(c);
    }
    
    bool make_trump(const Card &upcard, bool is_dealer,
                   int round, Suit &order_up_suit) const override {
        assert(round == 1 || round == 2);
        print_hand();
        cout << "Human player " << name << ", please enter a suit, or \"pass\":\n";
        string decision;
        cin >> decision;
        if (decision != "pass") {
            Suit ordered_up = string_to_suit(decision);
            if(ordered_up == SPADES) {
                cout << name << " orders up Spades" << endl;
                order_up_suit = SPADES;
                return true;
            }
            else if(ordered_up == CLUBS) {
                cout << name << " orders up Clubs" << endl;
                order_up_suit = CLUBS;
                return true;
            }
            else if(ordered_up == HEARTS) {
                cout << name << " orders up Hearts" << endl;
                order_up_suit = HEARTS;
                return true;
            }
            else if(ordered_up == DIAMONDS) {
                cout << name << " orders up Diamonds" << endl;
                order_up_suit = DIAMONDS;
                return true;
            }
        }
        cout << name << " passes" << endl;
        return false;
    }
    
    void add_and_discard(const Card &upcard) override {
        assert(hand.size() >= 1);
        hand.push_back(upcard);
        
        // Get the suit that's actually trump (the upcard's suit)
        Suit trump_suit = upcard.get_suit();
        
        // Find the lowest card considering Euchre rules
        int lowest_index = 0;
        for(int i = 1; i < hand.size(); i++) {
            // Use Card_less correctly - the first card is "lower" if Card_less returns true
            if(Card_less(hand[i], hand[lowest_index], trump_suit)) {
                lowest_index = i;
            }
        }
        
        hand.erase(hand.begin() + lowest_index);
    }
    
    Card lead_card(Suit trump) override {
        int cardNum;
        Card lead_card;
        std::sort(hand.begin(), hand.end());
        print_hand();
        cout << "Human player " << name << ", please select a card:" << endl;
        cin >> cardNum;
        cout << hand[cardNum] << " led by " << name << endl;
        
        lead_card = hand[cardNum];
        hand.erase(hand.begin() + cardNum);
        
        return lead_card;
    }
    
    Card play_card(const Card &led_card, Suit trump) override {
        return lead_card(trump);
    }

private:
    std::string name;
    std::vector<Card> hand;
    
    void print_hand() const {
        for (size_t i = 0; i < hand.size(); ++i)
            cout << "Human player " << name << "'s hand: "
                 << "[" << i << "] " << hand[i] << "\n";
    }
};

Player * Player_factory(const std::string &name, 
                       const std::string &strategy) {
    if (strategy == "Simple") {
        return new Simple(name);
    }
    if (strategy == "Human") {
        return new Human(name);
    }
    assert(false);
    return nullptr;
}

std::ostream & operator<<(std::ostream &os, const Player &p) {
    os << p.get_name();    
     return os;
}