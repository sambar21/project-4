#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <map>
#include <cmath>
#include <vector>
#include <algorithm>
#include "csvstream.hpp"

using namespace std;

// Unique words extraction function
set<string> unique_words(const string &str) {
    istringstream source(str);
    set<string> words;
    string word;
    while (source >> word) {
        words.insert(word);
    }
    return words;
}

class Classifier {
private:
    int total_posts;
    set<string> vocabulary;
    map<string, int> label_counts;
    map<string, int> word_counts;
    map<pair<string, string>, int> label_word_counts;

    // Store training data for printing
    vector<pair<string, string>> training_data;

    map<string, double> log_prior_probs;
    map<pair<string, string>, double> log_likelihood_probs;

public:
void train(const string &filename) {
    // Reset all training data
    total_posts = 0;
    vocabulary.clear();
    label_counts.clear();
    word_counts.clear();
    label_word_counts.clear();
    log_prior_probs.clear();
    log_likelihood_probs.clear();
    training_data.clear();

    // Open CSV file
    csvstream csvin(filename);
    
    map<string, string> row;
    while (csvin >> row) {
        string label = row["tag"];
        string content = row["content"];

        // Store training data
        training_data.push_back({label, content});

        total_posts++;
        label_counts[label]++;

        set<string> words = unique_words(content);

        // Update vocabulary and word counts
        for (const string &word : words) {
            vocabulary.insert(word);
            word_counts[word]++;
            label_word_counts[{label, word}]++;
        }
    }

    // Calculate log prior probabilities
    for (const auto &label_pair : label_counts) {
        log_prior_probs[label_pair.first] = 
            log(static_cast<double>(label_pair.second) / total_posts);
    }

    // Compute log likelihood probabilities
    vector<string> labels;
    for (const auto &label_pair : label_counts) {
        labels.push_back(label_pair.first);
    }
    sort(labels.begin(), labels.end());

    for (const string &label : labels) {
        for (const string &word : vocabulary) {
            int label_word_count = label_word_counts[{label, word}];
            int label_total_count = label_counts[label];
            
            // More consistent log likelihood calculation
            if (label_word_count > 0) {
                // Words that appear in the label
                double prob = static_cast<double>(label_word_count) / label_total_count;
                log_likelihood_probs[{label, word}] = log(prob);
            } else if (word_counts[word] > 0) {
                // Words not in this label but seen in training
                double prob = 1.0 / (label_total_count + 2);
                log_likelihood_probs[{label, word}] = log(prob);
            } else {
                // Rare/unseen words
                log_likelihood_probs[{label, word}] = log(1.0 / (label_total_count + 2));
            }
        }
    }
}
void print_training_summary() {
    // Print training data
    cout << "training data:" << endl;
    for (const auto &entry : training_data) {
        cout << "  label = " << entry.first 
             << ", content = " << entry.second << endl;
    }
    
    
    cout << "trained on " << total_posts << " examples" << endl;
    cout << "vocabulary size = " << vocabulary.size() << endl;
    cout << endl;
    // Print classes
    cout << "classes:" << endl;
    vector<string> sorted_labels;
    for (const auto &label_pair : label_counts) {
        sorted_labels.push_back(label_pair.first);
    }
    sort(sorted_labels.begin(), sorted_labels.end());

    for (const string &label : sorted_labels) {
        cout << "  " << label << ", " << label_counts[label] 
             << " examples, log-prior = " 
             << log_prior_probs[label] << endl;
    }
    
    // Print classifier parameters
    cout << "classifier parameters:" << endl;
    for (const string &label : sorted_labels) {
        // Collect words for this label
        vector<pair<string, int>> label_word_log_counts;
        for (const string &word : vocabulary) {
            int count = label_word_counts[{label, word}];
            if (count > 0) {
                label_word_log_counts.push_back({word, count});
            }
        }
        
        // Sort alphabetically
        sort(label_word_log_counts.begin(), label_word_log_counts.end());

        // Print words for this label
        for (const auto &word_count : label_word_log_counts) {
            cout << "  " << label << ":" << word_count.first 
                 << ", count = " << word_count.second 
                 << ", log-likelihood = " 
                 << log_likelihood_probs[{label, word_count.first}] << endl;
        }
    }
   cout<<endl;
}
    // Prediction method stays the same as in previous implementation
    string predict(const string &post_content) {
        set<string> words = unique_words(post_content);
        
        map<string, double> log_probs;
        
        vector<string> labels;
        for (const auto &label_pair : label_counts) {
            labels.push_back(label_pair.first);
        }
        sort(labels.begin(), labels.end());

        for (const string &label : labels) {
            log_probs[label] = compute_log_probability(label, words);
        }

        // Find label with highest log prob
        string best_label;
        double best_score = -numeric_limits<double>::infinity();
        for (const auto &log_prob : log_probs) {
            if (log_prob.second > best_score || 
                (log_prob.second == best_score && log_prob.first < best_label)) {
                best_label = log_prob.first;
                best_score = log_prob.second;
            }
        }

        return best_label;
    }

    double compute_log_probability(const string &label, const set<string> &words) {
        double log_prob = log_prior_probs[label];

        vector<string> sorted_words(words.begin(), words.end());
        sort(sorted_words.begin(), sorted_words.end());

        for (const string &word : sorted_words) {
            log_prob += log_likelihood_probs[{label, word}];
        }

        return log_prob;
    }
};
int main(int argc, char *argv[]) {
    cout.precision(3);  

    // Validate command line arguments
    if (argc != 2 && argc != 3) {
        cout << "Usage: " << argv[0] << " TRAIN_FILE [TEST_FILE]" << endl;
        return 1;
    }

    try {
        Classifier classifier;
        
        // Train classifier
        string train_filename = argv[1];
        classifier.train(train_filename);
        classifier.print_training_summary();

        // Optional testing
        if (argc == 3) {
            string test_filename = argv[2];
            csvstream test_csvin(test_filename);
            map<string, string> test_row;
            
            int correct_predictions = 0;
            int total_test_posts = 0;

            cout << "test data:" << endl;
            
while (test_csvin >> test_row) {
    string correct_label = test_row["tag"];
    string content = test_row["content"];
    
    string predicted_label = classifier.predict(content);
    double log_prob_score = classifier.compute_log_probability(
        predicted_label, unique_words(content)
    );

    cout << "  correct = " << correct_label  // Note the TWO spaces
         << ", predicted = " << predicted_label 
         << ", log-probability score = " << log_prob_score << endl;
    cout << "  content = " << content << endl << endl;  // TWO spaces here too

    if (predicted_label == correct_label) {
        correct_predictions++;
    }
    total_test_posts++;
}

            cout << "performance: " << correct_predictions 
                 << " / " << total_test_posts << " posts predicted correctly" << endl;
        }
    }
    catch (const csvstream_exception &e) {
        cerr << "CSV Error: " << e.what() << endl;
        return 1;
    }
    catch (const exception &e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}