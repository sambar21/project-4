#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <iomanip>         // For std::fixed, std::setprecision
#include "csvstream.hpp"   // Must be in the same directory

using namespace std;

/*
 * Return a set of unique, whitespace-delimited words from a string.
 * Fulfills the "bag of words" model by ignoring duplicates.
 */
set<string> unique_words(const string &text) {
    istringstream iss(text);
    set<string> words;
    string w;
    while (iss >> w) {
        words.insert(w);
    }
    return words;
}

/*
 * Helper function that:
 *   1) If |x| < 1, uses 3 decimals
 *   2) Else if 1 <= |x| < 10, uses 2 decimals
 *   3) Otherwise (|x| >= 10), uses 1 decimal
 *   4) Then removes trailing zeroes (e.g. "-13.70" -> "-13.7", "2.00" -> "2").
 */
static void print_mixed_precision(double x) {
    // Save current format
    ios init(nullptr);
    init.copyfmt(cout);

    // Step A: Convert to string with the chosen precision
    double ax = fabs(x);
    ostringstream oss;
    if (ax < 1.0) {
        oss << fixed << setprecision(3) << x;
    }
    else if (ax < 10.0) {
        oss << fixed << setprecision(2) << x;
    }
    else {
        oss << fixed << setprecision(1) << x;
    }
    string s = oss.str();

    // Step B: Strip trailing zeros. e.g. "1.10" -> "1.1"
    while (!s.empty() && s.back() == '0') {
        s.pop_back();
    }

    // Step C: If it ends with '.', remove it too
    if (!s.empty() && s.back() == '.') {
        s.pop_back();
    }

    // Print the result
    cout << s;

    // Restore old format
    cout.copyfmt(init);
}

/*
 * A simple Bernoulli Naive Bayes Classifier for the EECS 280 project.
 * Stores counts and vocabulary derived from a training set of (label, content) pairs.
 * Then can predict labels for new posts.
 */
class Classifier {
public:
    // Train the classifier on a CSV input stream (with columns "tag" and "content").
    // If print_training_data == true, prints line-by-line info of each training post.
    void train(csvstream &csvin, bool print_training_data = false) {
        map<string, string> row;
        while (csvin >> row) {
            const string &label   = row.at("tag");
            const string &content = row.at("content");

            if (print_training_data) {
                // Print line-by-line training data (train-only mode)
                cout << "  label = " << label
                     << ", content = " << content << "\n";
            }

            ++total_posts;
            label_counts[label]++;

            // Extract unique words in this post
            set<string> words_in_post = unique_words(content);
            for (const auto &w : words_in_post) {
                vocabulary.insert(w);
                ++word_counts[w];               // total #posts containing w (all labels)
                ++label_word_counts[label][w];  // #posts w/ this label containing w
            }
        }
    }

    // Print training summary.
    // - Always prints "trained on X examples"
    // - If train_only_mode == true, also prints vocabulary size, label details, and word likelihoods
    void print_training_summary(bool train_only_mode) const {
        // 1) total number of examples
        cout << "trained on " << total_posts << " examples\n";

        // 2) If "train-only" mode, print the rest
        if (train_only_mode) {
            cout << "vocabulary size = " << vocabulary.size() << "\n\n";

            // Print label info in alphabetical order
            cout << "classes:\n";
            vector<string> sorted_labels;
            for (auto &lc : label_counts) {
                sorted_labels.push_back(lc.first);
            }
            sort(sorted_labels.begin(), sorted_labels.end());

            // For each label, print count and log-prior
            for (auto &lbl : sorted_labels) {
                double prior = double(label_counts.at(lbl)) / double(total_posts);
                double log_prior = log(prior);

                cout << "  " << lbl << ", "
                     << label_counts.at(lbl) << " examples, "
                     << "log-prior = ";
                print_mixed_precision(log_prior);
                cout << "\n";
            }

            // Print classifier parameters
            cout << "classifier parameters:\n";
            for (auto &lbl : sorted_labels) {
                // Gather words used by this label
                vector<string> words_for_label;
                for (auto &wcount : label_word_counts.at(lbl)) {
                    words_for_label.push_back(wcount.first);
                }
                sort(words_for_label.begin(), words_for_label.end());

                for (auto &w : words_for_label) {
                    int count_label_word = label_word_counts.at(lbl).at(w);
                    double numerator   = double(count_label_word);
                    double denominator = double(label_counts.at(lbl));
                    // log( (#posts label & word) / (#posts label) )
                    double ll = log(numerator / denominator);

                    cout << "  " << lbl << ":" << w
                         << ", count = " << count_label_word
                         << ", log-likelihood = ";
                    print_mixed_precision(ll);
                    cout << "\n";
                }
            }
            cout << "\n";
        }
    }

    // Predict a label for a new post. Returns {best_label, best_log_score}.
    pair<string, double> predict(const set<string> &post_words) const {
        // We’ll find label that maximizes:
        //    log(#posts w/ label / total_posts) + sum( log(P(w|label)) for w in post_words )
        // Summation is done in alphabetical order for consistency.

        // Convert set -> sorted vector
        vector<string> sorted_post_words(post_words.begin(), post_words.end());
        sort(sorted_post_words.begin(), sorted_post_words.end());

        // Sort labels to break ties by alphabetical order
        vector<string> sorted_labels;
        for (auto &lc : label_counts) {
            sorted_labels.push_back(lc.first);
        }
        sort(sorted_labels.begin(), sorted_labels.end());

        string best_label;
        double best_score = -numeric_limits<double>::infinity();

        for (auto &lbl : sorted_labels) {
            // Start with log-prior
            double log_prior = log(double(label_counts.at(lbl)) / double(total_posts));
            double score = log_prior;

            // Add log-likelihood contributions for each word
            for (auto &w : sorted_post_words) {
                score += word_log_likelihood(lbl, w);
            }

            // Check if this is the best so far (tie-break on alphabetical label)
            if ((score > best_score) ||
                (fabs(score - best_score) < 1e-14 && lbl < best_label)) {
                best_score = score;
                best_label = lbl;
            }
        }
        return {best_label, best_score};
    }

private:
    int total_posts = 0;
    set<string> vocabulary; // All unique words in training data

    // label -> #posts with that label
    map<string, int> label_counts;

    // word -> #posts (across all labels) containing that word
    map<string, int> word_counts;

    // label -> (word -> #posts with label that contain word)
    map<string, map<string, int>> label_word_counts;

    /*
     * Compute log P(word | label) according to the assignment spec.
     * Cases:
     *  1) If the word never appears anywhere: log(1 / (total_posts + 2))
     *  2) If the word appears in the corpus but not under this label:
     *     log(1 / (#posts with label + 2))
     *  3) Otherwise: log( (# label&word) / (# label) )
     */
    double word_log_likelihood(const std::string &label, const std::string &word) const {
        // "occurrences" = total # of posts containing this word
        double occurrences = 0.0;
        if (word_counts.find(word) != word_counts.end()) {
            occurrences = static_cast<double>(word_counts.at(word));
        }

        // "candw" = # of posts that contain "word" AND have label "label",
        // or -1 if none is recorded.
        double candw = -1.0;
        auto it_label = label_word_counts.find(label);
        if (it_label != label_word_counts.end()) {
            auto it_word = it_label->second.find(word);
            if (it_word != it_label->second.end()) {
                candw = static_cast<double>(it_word->second);
            }
        }

        // CASE 1: candw == -1 AND occurrences == 0 => word never appears anywhere
        if ((candw == -1.0) && (occurrences == 0.0)) {
            // returns ln(1 / total_posts)
            double hold = 1.0 / static_cast<double>(total_posts);
            return std::log(hold);
        }

        // CASE 2: candw == -1 AND occurrences != 0 => word appears globally, but not under this label
        if ((candw == -1.0) && (occurrences != 0.0)) {
            // returns ln(occurrences / total_posts)
            double hold2 = occurrences / static_cast<double>(total_posts);
            return std::log(hold2);
        }

        // CASE 3: We have a valid candw => returns ln(#(label&word)/ #(label))
        double cTotal = static_cast<double>(label_counts.at(label));
        double hold3  = candw / cTotal;
        return std::log(hold3);
    }
};

int main(int argc, char *argv[]) {
    // We'll keep a default "3 decimals" for everything else,
    // but specifically for logs, we use our new print_mixed_precision().
    cout.precision(3);
    cout << fixed;

    // 1) Command line check
    if (argc < 2 || argc > 3) {
        cout << "Usage: classifier.exe TRAIN_FILE [TEST_FILE]" << endl;
        return 1;
    }

    string train_filename = argv[1];
    bool has_test_file = (argc == 3);

    // 2) Attempt to open train file
    try {
        csvstream train_csv(train_filename);

        // 3) Create classifier, do training
        Classifier nb;
        bool train_only_mode = !has_test_file;  // If no test file, it's train-only
        if (train_only_mode) {
            cout << "training data:\n";
        }
        nb.train(train_csv, train_only_mode);

        // Print training summary
        nb.print_training_summary(train_only_mode);

        // 4) If there's a test file, open it and predict
        if (has_test_file) {
            string test_filename = argv[2];
            csvstream test_csv(test_filename);

            cout << "\ntest data:\n";

            int correct_count = 0;
            int total_test_posts = 0;
            map<string, string> row;

            while (test_csv >> row) {
                ++total_test_posts;
                const string &true_label = row.at("tag");
                const string &content    = row.at("content");

                // Predict
                set<string> words = unique_words(content);
                auto [predicted_label, log_prob_score] = nb.predict(words);

                // Print the required output for each test post
                cout << "  correct = " << true_label
                     << ", predicted = " << predicted_label
                     << ", log-probability score = ";
                print_mixed_precision(log_prob_score);
                cout << "\n";

                cout << "  content = " << content << "\n\n";

                if (predicted_label == true_label) {
                    ++correct_count;
                }
            }

            // Finally, print performance summary
            cout << "performance: " << correct_count << " / " << total_test_posts
                 << " posts predicted correctly\n";
        }
    }
    catch (const csvstream_exception &) {
        cerr << "Error opening file: "
             << (has_test_file ? argv[2] : argv[1]) << endl;
        return 1;
    }

    return 0;
}