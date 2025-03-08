#include "Pack.hpp"
#include <cassert>
#include <iostream>
#include <array>

using namespace std;

Pack::Pack(){
   next = 0;
   int index = 0;
   for(int s = SPADES; s <= DIAMONDS;s++){
    Suit suit = static_cast<Suit>(s);
    for(int r = NINE; r <= ACE; r++){
        Rank rank = static_cast<Rank>(r);
        cards[index] = Card(rank,suit);
        index ++;
    }
   }
   
}
Pack::Pack(std::istream& pack_input){
    next = 0;
    for(int i = 0; i < PACK_SIZE; i++){
        pack_input >> cards[i];
    }

}
Card Pack::deal_one(){
    assert(next < PACK_SIZE);
    return cards[next++];
}
void Pack::reset() {
    next = 0;
}
bool Pack::empty() const{
    return next >= PACK_SIZE;
}
void Pack::shuffle() {
    for (int i = 0; i < 7; ++i) {
        array<Card, PACK_SIZE> temp;
        int mid = PACK_SIZE / 2;
        for (int k = 0; k < mid; ++k) {
            // Put second half card first then first half card
            temp[2*k] = cards[mid + k];
            temp[2*k + 1] = cards[k];
        }
        cards = temp;
    }
    
    next = 0; 
}