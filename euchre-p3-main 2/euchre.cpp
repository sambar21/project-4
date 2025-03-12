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

// Game configuration struct to reduce parameter count
struct GameConfig {
    string pack_filename;
    bool shuffle_on;
    int points_to_win;
    vector<Player*> players;
    vector<string> player_types;
};

// Structure to hold card information for a trick
struct TrickInfo {
    int leader_idx;
    Card played_cards[NUM_PLAYERS];
    Card led_card;
    Suit trump;
};

class Game {
public:
    Game(const GameConfig& config) 
        : pack_filename(config.pack_filename), 
          shuffle_on(config.shuffle_on),
          points_to_win(config.points_to_win), 
          players(config.players),
          player_types(config.player_types),
          dealer_idx(0), scores{0, 0} {
        
        team1_idx[0] = 0;
        team1_idx[1] = 2;
        team2_idx[0] = 1;
        team2_idx[1] = 3;
    }

    void play() {
        // Print the executable and all arguments
        cout << "./euchre.exe " << pack_filename << " ";
        cout << (shuffle_on ? "shuffle" : "noshuffle") << " ";
        cout << points_to_win << " ";
        for (size_t i = 0; i < players.size(); ++i) {
            cout << players[i]->get_name() << " ";
            cout << player_types[i] << " ";
        }
        cout << endl;

        ifstream pack_file(pack_filename);
        if (!pack_file.is_open()) {
            cerr << "Error: Failed to open pack file during game play" << endl;
            return;
        }
        
        Pack pack(pack_file);
        
        int hand_num = 0;
        while (scores[0] < points_to_win && scores[1] < points_to_win) {
            // Play a single hand
            play_hand(pack, hand_num++);
            
            // Check if the game is over
            if (scores[0] >= points_to_win) {
                print_winners(0);
                break;
            } else if (scores[1] >= points_to_win) {
                print_winners(1);
                break;
            }
        }
    }

private:
    string pack_filename;
    bool shuffle_on;
    int points_to_win;
    vector<Player*> players;
    vector<string> player_types; 
    int dealer_idx;
    int scores[2]; 
    int team1_idx[2];
    int team2_idx[2]; 

    // Helper function to print winners
    void print_winners(int team) {
        if (team == 0) {
            cout << players[team1_idx[0]]->get_name() << " and " 
                 << players[team1_idx[1]]->get_name() << " win!" << endl;
        } else {
            cout << players[team2_idx[0]]->get_name() << " and " 
                 << players[team2_idx[1]]->get_name() << " win!" << endl;
        }
    }

    
    void play_hand(Pack& pack, int hand_num) {
        // Reset the pack for a new hand
        pack.reset();
        
        // Shuffle if needed
        if (shuffle_on) {
            pack.shuffle();
        }
        
        cout << "Hand " << hand_num << endl;
        cout << players[dealer_idx]->get_name() << " deals" << endl;
        
        deal_cards(pack);
        
        Card upcard = pack.deal_one();
        cout << upcard << " turned up" << endl;
        
        Suit order_up_suit;
        int trump_maker_idx = make_trump(upcard, order_up_suit);
        
        int tricks_won[2] = {0, 0}; 
        int leader_idx = (dealer_idx + 1) % NUM_PLAYERS; 
        
        // Play all tricks in the hand
        for (int trick = 0; trick < HAND_SIZE; ++trick) {
            int winner_idx = play_trick(leader_idx, order_up_suit);
            
            int winning_team = get_team(winner_idx);
            tricks_won[winning_team]++;
            
            leader_idx = winner_idx;
        }
        
        // Process results and update scores
        process_hand_results(tricks_won, trump_maker_idx);
        
        // Update dealer for next hand
        dealer_idx = (dealer_idx + 1) % NUM_PLAYERS;
    }
    //helps process results 
    void process_hand_results(int tricks_won[], int trump_maker_idx) {
        int making_team = get_team(trump_maker_idx);
        int hand_winner;
        bool is_march = false;
        bool is_euchre = false;

        // Determine which team won and any special conditions
        if (tricks_won[making_team] >= TRICKS_TO_WIN) {
            hand_winner = making_team;
            if (tricks_won[making_team] == HAND_SIZE) {
                is_march = true;
            }
        } else {
            hand_winner = 1 - making_team; 
            is_euchre = true;
        }
        // Update scores
        update_scores(hand_winner, is_march, is_euchre);
        // Print hand results
        print_hand_results(hand_winner, is_march, is_euchre);
    }
    
    void print_hand_results(int hand_winner, bool is_march, bool is_euchre) {
        string team1_name = players[team1_idx[0]]->get_name() + " and " +
                           players[team1_idx[1]]->get_name();
        string team2_name = players[team2_idx[0]]->get_name() + " and " +
                           players[team2_idx[1]]->get_name();
        
        if (hand_winner == 0) {
            cout << team1_name << " win the hand" << endl;
        } else {
            cout << team2_name << " win the hand" << endl;
        }
        
        if (is_march) {
            cout << "march!" << endl;
        } else if (is_euchre) {
            cout << "euchred!" << endl;
        }
        
        // Print updated scores
        cout << team1_name << " have " << scores[0] << " points" << endl;
        cout << team2_name << " have " << scores[1] << " points" << endl;
        cout << endl;
    }
    
    // Update scores helper 
    void update_scores(int hand_winner, bool is_march, bool is_euchre) {
        if (is_march) {
            scores[hand_winner] += POINTS_FOR_MARCH;
        } else if (is_euchre) {
            scores[hand_winner] += POINTS_FOR_EUCHRE;
        } else {
            scores[hand_winner] += POINTS_FOR_WIN;
        }
    }

    void deal_cards(Pack& pack) {
        const int first_deal[NUM_PLAYERS] = {3, 2, 3, 2};
        const int second_deal[NUM_PLAYERS] = {2, 3, 2, 3};
        
        // First round 
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            int player_idx = (dealer_idx + 1 + i) % NUM_PLAYERS;
            for (int j = 0; j < first_deal[i]; ++j) {
                players[player_idx]->add_card(pack.deal_one());
            }
        }
        
        // Second round 
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            int player_idx = (dealer_idx + 1 + i) % NUM_PLAYERS;
            for (int j = 0; j < second_deal[i]; ++j) {
                players[player_idx]->add_card(pack.deal_one());
            }
        }
    }

    int make_trump(const Card& upcard, Suit& order_up_suit) {
        int result = make_trump_round_one(upcard, order_up_suit);
        if (result >= 0) {
            return result;
        }
        
        result = make_trump_round_two(upcard, order_up_suit);
        if (result >= 0) {
            return result;
        }
        
        assert(false);
        return -1;
    }
    
    
    int make_trump_round_one(const Card& upcard, Suit& order_up_suit) {
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            int player_idx = (dealer_idx + 1 + i) % NUM_PLAYERS;
            bool is_dealer = (player_idx == dealer_idx);
            
            bool made_trump = players[player_idx]->make_trump(
                upcard, is_dealer, 1, order_up_suit);
                
            if (made_trump) {
                auto name = players[player_idx]->get_name();
                cout << name << " orders up " << order_up_suit << endl;
                
                // Dealer picks up the upcard
                players[dealer_idx]->add_and_discard(upcard);
                
                cout << endl; 
                return player_idx; 
            } else {
                cout << players[player_idx]->get_name() << " passes" << endl;
            }
        }
        return -1;
    }
    
    int make_trump_round_two(const Card& upcard, Suit& order_up_suit) {
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            int player_idx = (dealer_idx + 1 + i) % NUM_PLAYERS;
            bool is_dealer = (player_idx == dealer_idx);
            
            // Break this line to fix the long line violation
            bool made_trump = players[player_idx]->make_trump(
                upcard, is_dealer, 2, order_up_suit);
                
            if (made_trump) {
                auto name = players[player_idx]->get_name();
                cout << name << " orders up " << order_up_suit << endl;
                
                cout << endl; 
                return player_idx; 
            } else {
                cout << players[player_idx]->get_name() << " passes" << endl;
            }
        }
        return -1;
    }

    
    int play_trick(int leader_idx, Suit trump) {
        TrickInfo trick;
        trick.leader_idx = leader_idx;
        trick.trump = trump;
        
        // Leader plays first
        trick.led_card = players[leader_idx]->lead_card(trump);
        trick.played_cards[leader_idx] = trick.led_card;
        cout << trick.led_card << " led by " << players[leader_idx]->get_name() << endl;
        
        // Other players play in clockwise order
        for (int i = 1; i < NUM_PLAYERS; ++i) {
            int player_idx = (leader_idx + i) % NUM_PLAYERS;
            
            Card played_card = players[player_idx]->play_card(trick.led_card, trump);
            trick.played_cards[player_idx] = played_card;
            
            cout << played_card << " played by " 
                 << players[player_idx]->get_name() << endl;
        }
        
        // Determine the winner 
        int winner_idx = find_trick_winner(trick);
        
        cout << players[winner_idx]->get_name() << " takes the trick" << endl;
        cout << endl; 
        
        return winner_idx;
    }
    
    // find the winner of a trick fix for 
    int find_trick_winner(const TrickInfo& trick) {
        int winner_idx = trick.leader_idx;
        Card winner_card = trick.played_cards[trick.leader_idx];
        
        for (int i = 1; i < NUM_PLAYERS; ++i) {
            int player_idx = (trick.leader_idx + i) % NUM_PLAYERS;
            
            if (Card_less(winner_card, trick.played_cards[player_idx], 
                         trick.led_card, trick.trump)) {
                winner_idx = player_idx;
                winner_card = trick.played_cards[player_idx];
            }
        }
        
        return winner_idx;
    }

    // Get team of a player
    int get_team(int player_idx) {
        if (player_idx == team1_idx[0] || player_idx == team1_idx[1]) {
            return 0;
        }
        return 1;
    }
};

bool validate_shuffle_arg(const string& shuffle_arg) {
    return (shuffle_arg == "shuffle" || shuffle_arg == "noshuffle");
}

bool validate_points(int points) {
    return points >= MIN_POINTS && points <= MAX_POINTS;
}

bool validate_player_types(const vector<string>& types) {
    for (const auto& type : types) {
        if (type != "Simple" && type != "Human") {
            return false;
        }
    }
    return true;
}


bool parse_arguments(int argc, char **argv, GameConfig& config) {
    // Check if the correct number of arguments is provided
    if (argc != NUM_ARGS) {
        return false;
    }
    
    // Extract  line arguments
    config.pack_filename = argv[1];
    string shuffle_arg = argv[2];
    
    if (!validate_shuffle_arg(shuffle_arg)) {
        return false;
    }
    config.shuffle_on = (shuffle_arg == "shuffle");
    
    try {
        config.points_to_win = stoi(argv[3]);
        if (!validate_points(config.points_to_win)) {
            return false;
        }
    } catch (...) {
        return false;
    }
    
    // Extract player names  types
    vector<string> player_names;
    for (int i = 0; i < 4; ++i) {
        player_names.push_back(argv[4 + i * 2]);
        config.player_types.push_back(argv[5 + i * 2]);
    }
    
    // Validate  types
    if (!validate_player_types(config.player_types)) {
        return false;
    }
    
    // Check if pack file can be opened
    ifstream pack_file(config.pack_filename);
    if (!pack_file.is_open()) {
        cout << "Error opening " << config.pack_filename << endl;
        return false;
    }
    pack_file.close();
    
    // Create player objects
    for (int i = 0; i < 4; ++i) {
        config.players.push_back(
            Player_factory(player_names[i], config.player_types[i]));
    }
    
    return true;
}

void print_usage() {
    cout << "Usage: euchre.exe PACK_FILENAME [shuffle|noshuffle] " << endl
         << "POINTS_TO_WIN NAME1 TYPE1 NAME2 TYPE2 NAME3 TYPE3 NAME4 TYPE4" << endl;
}

int main(int argc, char **argv) {
    GameConfig config;
    
    if (!parse_arguments(argc, argv, config)) {
        print_usage();
        return 1;
    }
    
    // Create and play the game
    Game game(config);
    game.play();
    
    // Clean up 
    for (auto player : config.players) {
        delete player;
    }
    
    return 0;
}