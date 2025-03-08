#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>
#include "Player.hpp"
#include "Pack.hpp"
#include "Card.hpp"

using namespace std;

// Constants for game setup
const int NUM_ARGS = 12;
const int MIN_POINTS = 1;
const int MAX_POINTS = 100;
const int HAND_SIZE = 5;
const int NUM_PLAYERS = 4;
const int TRICKS_TO_WIN = 3;
const int POINTS_FOR_MARCH = 2;
const int POINTS_FOR_WIN = 1;
const int POINTS_FOR_EUCHRE = 2;

class Game {
public:
    // Constructor that takes all game parameters
    Game(const string& pack_filename, bool shuffle_on, int points_to_win,
         const vector<Player*>& game_players, const vector<string>& player_types) 
        : pack_filename(pack_filename), shuffle_on(shuffle_on), 
          points_to_win(points_to_win), players(game_players), player_types(player_types),
          dealer_idx(0), scores{0, 0} {
        
        // Set up team indices
        // Players 0 and 2 are on first team, 1 and 3 on second team
        team1_idx[0] = 0;
        team1_idx[1] = 2;
        team2_idx[0] = 1;
        team2_idx[1] = 3;
    }

    // Main function to play the full game
    void play() {
        // Print the executable and all arguments
        cout << "./euchre.exe " << pack_filename << " ";
        cout << (shuffle_on ? "shuffle" : "noshuffle") << " ";
        cout << points_to_win << " ";
        for (size_t i = 0; i < players.size(); ++i) {
            cout << players[i]->get_name() << " ";
            // Use stored player types instead of dynamic_cast
            cout << player_types[i] << " ";
        }
        cout << endl;

        // Load the pack from file
        ifstream pack_file(pack_filename);
        if (!pack_file.is_open()) {
            cerr << "Error: Failed to open pack file during game play" << endl;
            return;
        }
        
        Pack pack(pack_file);
        
        // Continue playing hands until a team reaches the points to win
        int hand_num = 0;
        while (scores[0] < points_to_win && scores[1] < points_to_win) {
            // Play a single hand
            play_hand(pack, hand_num++);
            
            // Check if the game is over
            if (scores[0] >= points_to_win) {
                cout << players[team1_idx[0]]->get_name() << " and " 
                     << players[team1_idx[1]]->get_name() << " win!" << endl;
                break;
            } else if (scores[1] >= points_to_win) {
                cout << players[team2_idx[0]]->get_name() << " and " 
                     << players[team2_idx[1]]->get_name() << " win!" << endl;
                break;
            }
        }
    }

private:
    string pack_filename;
    bool shuffle_on;
    int points_to_win;
    vector<Player*> players;
    vector<string> player_types; // Store player types to avoid dynamic_cast
    int dealer_idx;
    int scores[2]; // Index 0 for team 1, index 1 for team 2
    int team1_idx[2]; // Player indices for team 1
    int team2_idx[2]; // Player indices for team 2

    // Play a single hand
    void play_hand(Pack& pack, int hand_num) {
        // Reset the pack for a new hand
        pack.reset();
        
        // Shuffle if needed
        if (shuffle_on) {
            pack.shuffle();
        }
        
        // Announce the hand
        cout << "Hand " << hand_num << endl;
        cout << players[dealer_idx]->get_name() << " deals" << endl;
        
        // Deal cards to all players
        deal_cards(pack);
        
        // Turn up a card
        Card upcard = pack.deal_one();
        cout << upcard << " turned up" << endl;
        
        // Determine trump suit
        Suit order_up_suit;
        int trump_maker_idx = make_trump(upcard, order_up_suit);
        
        // Play five tricks
        int tricks_won[2] = {0, 0}; // Tricks won by each team
        int leader_idx = (dealer_idx + 1) % NUM_PLAYERS; // Left of dealer leads first trick
        
        for (int trick = 0; trick < HAND_SIZE; ++trick) {
            int winner_idx = play_trick(leader_idx, order_up_suit);
            
            // Determine which team won the trick
            int winning_team = get_team(winner_idx);
            tricks_won[winning_team]++;
            
            // Next trick is led by the winner of the current trick
            leader_idx = winner_idx;
        }
        
        // Determine which team won the hand
        int making_team = get_team(trump_maker_idx);
        int hand_winner;
        bool is_march = false;
        bool is_euchre = false;

        if (tricks_won[making_team] >= TRICKS_TO_WIN) {
            hand_winner = making_team;
            if (tricks_won[making_team] == HAND_SIZE) {
                is_march = true;
            }
        } else {
            hand_winner = 1 - making_team; // Other team
            is_euchre = true;
        }
        
        // Print the winners of the hand
        if (hand_winner == 0) {
            cout << players[team1_idx[0]]->get_name() << " and " 
                 << players[team1_idx[1]]->get_name() << " win the hand" << endl;
        } else {
            cout << players[team2_idx[0]]->get_name() << " and " 
                 << players[team2_idx[1]]->get_name() << " win the hand" << endl;
        }
        
        // Print if march or euchre occurred
        if (is_march) {
            cout << "march!" << endl;
        } else if (is_euchre) {
            cout << "euchred!" << endl;
        }
        
        // Update scores
        if (is_march) {
            scores[hand_winner] += POINTS_FOR_MARCH;
        } else if (is_euchre) {
            scores[hand_winner] += POINTS_FOR_EUCHRE;
        } else {
            scores[hand_winner] += POINTS_FOR_WIN;
        }
        
        // Print scores
        cout << players[team1_idx[0]]->get_name() << " and " 
             << players[team1_idx[1]]->get_name() << " have " 
             << scores[0] << " points" << endl;
        cout << players[team2_idx[0]]->get_name() << " and " 
             << players[team2_idx[1]]->get_name() << " have " 
             << scores[1] << " points" << endl;
        cout << endl;
        
        // Update dealer for next hand
        dealer_idx = (dealer_idx + 1) % NUM_PLAYERS;
    }

    // Deal cards to all players
    void deal_cards(Pack& pack) {
        // Use arrays to define the deal pattern for better readability
        const int first_deal[NUM_PLAYERS] = {3, 2, 3, 2};
        const int second_deal[NUM_PLAYERS] = {2, 3, 2, 3};
        
        // First round of dealing
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            int player_idx = (dealer_idx + 1 + i) % NUM_PLAYERS;
            for (int j = 0; j < first_deal[i]; ++j) {
                players[player_idx]->add_card(pack.deal_one());
            }
        }
        
        // Second round of dealing
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            int player_idx = (dealer_idx + 1 + i) % NUM_PLAYERS;
            for (int j = 0; j < second_deal[i]; ++j) {
                players[player_idx]->add_card(pack.deal_one());
            }
        }
    }

    // Make trump - returns the index of the player who called trump
    int make_trump(const Card& upcard, Suit& order_up_suit) {
        // First round - everyone gets a chance to order up the upcard suit
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            int player_idx = (dealer_idx + 1 + i) % NUM_PLAYERS;
            bool is_dealer = (player_idx == dealer_idx);
            
            if (players[player_idx]->make_trump(upcard, is_dealer, 1, order_up_suit)) {
                cout << players[player_idx]->get_name() << " orders up " 
                     << order_up_suit << endl;
                
                // Dealer picks up the upcard
                players[dealer_idx]->add_and_discard(upcard);
                
                cout << endl; // Extra newline after making trump is complete
                return player_idx; // Return index of player who ordered up
            } else {
                cout << players[player_idx]->get_name() << " passes" << endl;
            }
        }
        
        // Second round - everyone gets a chance to pick a different suit
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            int player_idx = (dealer_idx + 1 + i) % NUM_PLAYERS;
            bool is_dealer = (player_idx == dealer_idx);
            
            if (players[player_idx]->make_trump(upcard, is_dealer, 2, order_up_suit)) {
                cout << players[player_idx]->get_name() << " orders up " 
                     << order_up_suit << endl;
                
                cout << endl; // Extra newline after making trump is complete
                return player_idx; // Return index of player who ordered up
            } else {
                cout << players[player_idx]->get_name() << " passes" << endl;
            }
        }
        
        // This should never happen with the current rules, but added for safety
        assert(false);
        return -1;
    }

    // Play a single trick - returns the index of the winner
    int play_trick(int leader_idx, Suit trump) {
        Card led_card;
        Card played_cards[NUM_PLAYERS];
        
        // Leader plays first
        led_card = players[leader_idx]->lead_card(trump);
        played_cards[leader_idx] = led_card;
        cout << led_card << " led by " << players[leader_idx]->get_name() << endl;
        
        // Other players play in clockwise order
        for (int i = 1; i < NUM_PLAYERS; ++i) {
            int player_idx = (leader_idx + i) % NUM_PLAYERS;
            
            Card played_card = players[player_idx]->play_card(led_card, trump);
            played_cards[player_idx] = played_card;
            
            cout << played_card << " played by " << players[player_idx]->get_name() << endl;
        }
        
        // Determine the winner of the trick
        int winner_idx = leader_idx;
        Card winner_card = played_cards[leader_idx];
        
        for (int i = 1; i < NUM_PLAYERS; ++i) {
            int player_idx = (leader_idx + i) % NUM_PLAYERS;
            
            if (Card_less(winner_card, played_cards[player_idx], led_card, trump)) {
                winner_idx = player_idx;
                winner_card = played_cards[player_idx];
            }
        }
        
        // Announce the winner
        cout << players[winner_idx]->get_name() << " takes the trick" << endl;
        cout << endl; // Extra newline after the trick
        
        return winner_idx;
    }

    // Helper to determine which team a player is on (0 for team1, 1 for team2)
    int get_team(int player_idx) {
        if (player_idx == team1_idx[0] || player_idx == team1_idx[1]) {
            return 0;
        }
        return 1;
    }
};

int main(int argc, char **argv) {
    // Check if the correct number of arguments is provided
    if (argc != NUM_ARGS) {
        cout << "Usage: euchre.exe PACK_FILENAME [shuffle|noshuffle] "
             << "POINTS_TO_WIN NAME1 TYPE1 NAME2 TYPE2 NAME3 TYPE3 "
             << "NAME4 TYPE4" << endl;
        return 1;
    }
    
    // Extract command line arguments
    string pack_filename = argv[1];
    string shuffle_arg = argv[2];
    int points_to_win;
    vector<string> player_names;
    vector<string> player_types;
    
    // Check if shuffle argument is valid
    bool shuffle_on;
    if (shuffle_arg == "shuffle") {
        shuffle_on = true;
    } else if (shuffle_arg == "noshuffle") {
        shuffle_on = false;
    } else {
        cout << "Usage: euchre.exe PACK_FILENAME [shuffle|noshuffle] "
             << "POINTS_TO_WIN NAME1 TYPE1 NAME2 TYPE2 NAME3 TYPE3 "
             << "NAME4 TYPE4" << endl;
        return 1;
    }
    
    // Check if points to win is valid
    try {
        points_to_win = stoi(argv[3]);
        if (points_to_win < MIN_POINTS || points_to_win > MAX_POINTS) {
            cout << "Usage: euchre.exe PACK_FILENAME [shuffle|noshuffle] "
                 << "POINTS_TO_WIN NAME1 TYPE1 NAME2 TYPE2 NAME3 TYPE3 "
                 << "NAME4 TYPE4" << endl;
            return 1;
        }
    } catch (...) {
        cout << "Usage: euchre.exe PACK_FILENAME [shuffle|noshuffle] "
             << "POINTS_TO_WIN NAME1 TYPE1 NAME2 TYPE2 NAME3 TYPE3 "
             << "NAME4 TYPE4" << endl;
        return 1;
    }
    
    // Extract player names and types
    for (int i = 0; i < 4; ++i) {
        player_names.push_back(argv[4 + i * 2]);
        player_types.push_back(argv[5 + i * 2]);
        
        // Check if player type is valid
        if (player_types[i] != "Simple" && player_types[i] != "Human") {
            cout << "Usage: euchre.exe PACK_FILENAME [shuffle|noshuffle] "
                 << "POINTS_TO_WIN NAME1 TYPE1 NAME2 TYPE2 NAME3 TYPE3 "
                 << "NAME4 TYPE4" << endl;
            return 1;
        }
    }
    
    // Check if pack file can be opened
    ifstream pack_file(pack_filename);
    if (!pack_file.is_open()) {
        cout << "Error opening " << pack_filename << endl;
        return 1;
    }
    pack_file.close(); // Close it for now, it will be reopened in the Game constructor
    
    // Create player objects
    vector<Player*> players;
    for (int i = 0; i < 4; ++i) {
        players.push_back(Player_factory(player_names[i], player_types[i]));
    }
    
    // Create and play the game
    Game game(pack_filename, shuffle_on, points_to_win, players, player_types);
    game.play();
    
    // Clean up player objects
    for (auto player : players) {
        delete player;
    }
    
    return 0;
}