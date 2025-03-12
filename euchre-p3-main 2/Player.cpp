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
                bool is_face_trump = hand[i].is_trump(upcardsuit) && 
                hand[i].is_face_or_ace();
                bool is_bower = hand[i].is_left_bower(upcardsuit) || 
                               hand[i].is_right_bower(upcardsuit);
                
                if(is_face_trump || is_bower) {
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
                bool is_face_trump = hand[i].is_trump(next_suit) && 
                hand[i].is_face_or_ace();
                bool is_bower = hand[i].is_left_bower(next_suit) || 
                               hand[i].is_right_bower(next_suit);
                
                if(is_face_trump || is_bower) {
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
        
        // get suit actually trump the upcard's suit
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
        
        int play_index = 0;
        
        for(int i = 1; i < hand.size(); i++) {
            // Skip trump cards if we have non-trump cards
            if(have_non_trump && hand[i].is_trump(trump)) {
                continue;
            }
            
            // Replace highest if  a trump card we  avoid playing
            if(have_non_trump && hand[play_index].is_trump(trump)) {
                play_index = i;
                continue;
            }
            
            // Otherwise pick the highest card
            if(Card_less(hand[play_index], hand[i], trump)) {
                play_index = i;
            }
        }
        
        Card to_play = hand[play_index];
        hand.erase(hand.begin() + play_index);
        return to_play;
    }
    
    // Helper function to find and play highest card of a suit
    Card find_highest_of_suit(Suit suit, const Card &led_card, Suit trump) {
        Card highest;
        int highest_index = -1;
        
        for(int i = 0; i < hand.size(); i++) {
            if(hand[i].get_suit(trump) == suit) {
                bool is_better_card = highest_index == -1 || 
                                    Card_less(highest, hand[i], led_card, trump);
                
                if(is_better_card) {
                    highest = hand[i];
                    highest_index = i;
                }
            }
        }
        
        hand.erase(hand.begin() + highest_index);
        return highest;
    }
    
    // Helper function to find and play lowest card
    Card find_lowest_card(const Card &led_card, Suit trump) {
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
            return find_highest_of_suit(led_suit, led_card, trump);
        } else {
            // Play lowest card 
            return find_lowest_card(led_card, trump);
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

        if (decision == "pass") {
            return false;
        }

        if (decision == "Spades" || decision == "Hearts" || 
            decision == "Clubs" || decision == "Diamonds") {
            order_up_suit = string_to_suit(decision);
            return true;
        } else {
            cout << "Invalid suit name. Enter Spades, Hearts, Clubs, ";
            cout<<"Diamonds, or \"pass\".\n";
            return false;
        }
    }
    
    void handle_discard(int discard_index) {
        // Create a sorted copy 
        vector<Card> sorted_hand(hand.begin(), hand.end() - 1);
        sort(sorted_hand.begin(), sorted_hand.end());
        
        // Validate index
        if (discard_index >= 0 && discard_index < sorted_hand.size()) {
          
            Card selected = sorted_hand[discard_index];
            
            // Find and remove 
            for (size_t i = 0; i < hand.size() - 1; ++i) {
                if (hand[i] == selected) {
                    hand.erase(hand.begin() + i);
                    break;
                }
            }
        } else {
            hand.pop_back();
        }
    }
    
    void add_and_discard(const Card &upcard) override {
        assert(hand.size() >= 1);
        
        // Print the hand before adding the upcard
        print_hand();
        
        // Add the upcard to the hand 
        hand.push_back(upcard);
        
        
        cout << "Discard upcard: [-1]\n";
        cout << "Human player " << name << ", please select a card to discard:\n";
        
        int discard_index;
        cin >> discard_index;
        
        if (discard_index == -1) {
            // Discard the upcard which was just added
            hand.pop_back();
        } else {
            handle_discard(discard_index);
        }
    }
    
    // handle card selection for both lead_card and play_card
    Card select_card_from_hand() {
        print_hand();
        cout << "Human player " << name << ", please select a card:" << endl;
        
        int cardNum;
        cin >> cardNum;
        
        
        vector<Card> sorted_hand = hand;
        sort(sorted_hand.begin(), sorted_hand.end());
        
        
        if (cardNum < 0 || cardNum >= sorted_hand.size()) {
            cout << "Invalid card index. Defaulting to first card." << endl;
            cardNum = 0;
        }
        
        Card selected = sorted_hand[cardNum];
        
        // Find and remove this card from the actual hand
        for (size_t i = 0; i < hand.size(); ++i) {
            if (hand[i] == selected) {
                Card to_play = hand[i];
                hand.erase(hand.begin() + i);
                return to_play;
            }
        }
        
       
        Card to_play = hand[0];
        hand.erase(hand.begin());
        return to_play;
    }
    
    Card lead_card(Suit trump) override {
        return select_card_from_hand();
    }
    
    Card play_card(const Card &led_card, Suit trump) override {
        return select_card_from_hand();
    }
    
private:
    std::string name;
    std::vector<Card> hand;
    
    void print_hand() const {
        // Create a sorted copy of the hand for display
        vector<Card> sorted_hand = hand;
        sort(sorted_hand.begin(), sorted_hand.end());
        
        for (size_t i = 0; i < sorted_hand.size(); ++i) {
            cout << "Human player " << name << "'s hand: ["
                 << i << "] " << sorted_hand[i] << "\n";
        }
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