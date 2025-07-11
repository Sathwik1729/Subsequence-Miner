#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <queue>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <memory>
#include <functional>
#include <cctype>
#include <limits> // Required for cin.ignore

using namespace std;
using namespace std::chrono;

// Forward declaration
class TrieNode;

// Structure to hold position information
struct Position {
    int sequence_id;
    int position;
    
    Position(int seq_id, int pos) : sequence_id(seq_id), position(pos) {}
};

// Structure to hold pattern information
struct PatternInfo {
    vector<string> pattern;
    int frequency;
    int length;
    double support;
    string pattern_string;
    bool is_noncontiguous;
    string pattern_type;
    
    // Default constructor
    PatternInfo()
        : frequency(0), length(0), support(0.0), is_noncontiguous(false) {}
    
    PatternInfo(const vector<string>& p, int freq, int len, double sup, 
                const string& p_str, bool non_contig, const string& type)
        : pattern(p), frequency(freq), length(len), support(sup), 
          pattern_string(p_str), is_noncontiguous(non_contig), pattern_type(type) {}
    
    // Copy constructor
    PatternInfo(const PatternInfo& other) = default;
    
    // Assignment operator
    PatternInfo& operator=(const PatternInfo& other) = default;
    
    // Comparison operators for priority queue
    bool operator<(const PatternInfo& other) const {
        return frequency < other.frequency;
    }
    
    bool operator>(const PatternInfo& other) const {
        return frequency > other.frequency;
    }
};

// Trie Node class
class TrieNode {
public:
    unordered_map<string, shared_ptr<TrieNode>> children;
    bool is_end_of_pattern;
    int frequency;
    vector<Position> positions;
    shared_ptr<TrieNode> parent;
    string character;
    bool is_noncontiguous;
    
    TrieNode() : is_end_of_pattern(false), frequency(0), parent(nullptr), 
                 character(""), is_noncontiguous(false) {}
    
    void add_position(int sequence_id, int position) {
        positions.emplace_back(sequence_id, position);
    }
};

// Subsequence Trie class
class SubsequenceTrie {
private:
    shared_ptr<TrieNode> root;
    int pattern_count;
    
public:
    SubsequenceTrie() : root(make_shared<TrieNode>()), pattern_count(0) {}
    
    void insert_pattern(const vector<string>& pattern, int sequence_id, 
                       int start_pos, bool is_noncontiguous = false) {
        auto node = root;
        
        for (size_t i = 0; i < pattern.size(); ++i) {
            const string& ch = pattern[i];
            
            if (node->children.find(ch) == node->children.end()) {
                node->children[ch] = make_shared<TrieNode>();
                node->children[ch]->parent = node;
                node->children[ch]->character = ch;
            }
            
            node = node->children[ch];
            node->add_position(sequence_id, start_pos + static_cast<int>(i));
        }
        
        if (!node->is_end_of_pattern) {
            node->is_end_of_pattern = true;
            pattern_count++;
        }
        
        node->frequency++;
        node->is_noncontiguous = is_noncontiguous;
    }
    
    shared_ptr<TrieNode> search_pattern(const vector<string>& pattern) {
        auto node = root;
        
        for (const string& ch : pattern) {
            if (node->children.find(ch) == node->children.end()) {
                return nullptr;
            }
            node = node->children[ch];
        }
        
        return node->is_end_of_pattern ? node : nullptr;
    }
    
    vector<tuple<vector<string>, int, bool>> get_frequent_patterns(int min_frequency, 
                                                                   bool noncontiguous_only = false) {
        vector<tuple<vector<string>, int, bool>> patterns;
        vector<string> current_pattern;
        
        function<void(shared_ptr<TrieNode>)> dfs = [&](shared_ptr<TrieNode> node) {
            if (node->is_end_of_pattern && node->frequency >= min_frequency) {
                if (!noncontiguous_only || node->is_noncontiguous) {
                    patterns.emplace_back(current_pattern, node->frequency, node->is_noncontiguous);
                }
            }
            
            for (const auto& child_pair : node->children) {
                const string& ch = child_pair.first;
                shared_ptr<TrieNode> child = child_pair.second;
                current_pattern.push_back(ch);
                dfs(child);
                current_pattern.pop_back();
            }
        };
        
        dfs(root);
        
        // Sort by frequency (descending)
        sort(patterns.begin(), patterns.end(), 
             [](const auto& a, const auto& b) {
                 return get<1>(a) > get<1>(b);
             });
        
        return patterns;
    }
    
    int get_pattern_count() const { return pattern_count; }
};

// Sequence data structure
struct SequenceData {
    int id;
    vector<string> sequence;
    int length;
    
    SequenceData(int i, const vector<string>& seq) 
        : id(i), sequence(seq), length(static_cast<int>(seq.size())) {}
};

// Mining statistics
struct MiningStatistics {
    int total_sequences;
    int total_patterns_found;
    double mining_time;
    int cache_hits;
    
    MiningStatistics() : total_sequences(0), total_patterns_found(0), 
                        mining_time(0.0), cache_hits(0) {}
};

// Main Advanced Subsequence Miner class
class AdvancedSubsequenceMiner {
private:
    int min_length;
    int max_length;
    SubsequenceTrie trie;
    vector<SequenceData> sequence_database;
    unordered_map<string, PatternInfo> pattern_cache;
    MiningStatistics statistics;
    
    void generate_subsequences(const vector<string>& sequence, int seq_id) {
        int sequence_len = static_cast<int>(sequence.size());
        
        // Generate contiguous subsequences
        for (int length = min_length; length <= min(max_length, sequence_len); ++length) {
            for (int start = 0; start <= sequence_len - length; ++start) {
                vector<string> subsequence(sequence.begin() + start, 
                                         sequence.begin() + start + length);
                trie.insert_pattern(subsequence, seq_id, start, false);
            }
        }
        
        // Generate non-contiguous subsequences (limited for performance)
        if (sequence_len <= 20) {  // Only for smaller sequences
            generate_noncontiguous_subsequences(sequence, seq_id);
        }
    }
    
    void generate_noncontiguous_subsequences(const vector<string>& sequence, int seq_id) {
        int n = static_cast<int>(sequence.size());
        int max_combinations = min(1000, 1 << n);
        
        for (int i = 1; i < max_combinations; ++i) {
            vector<string> subsequence;
            vector<int> positions;
            
            for (int j = 0; j < n; ++j) {
                if (i & (1 << j)) {
                    subsequence.push_back(sequence[j]);
                    positions.push_back(j);
                }
            }
            
            if (static_cast<int>(subsequence.size()) >= min_length && 
                static_cast<int>(subsequence.size()) <= max_length) {
                trie.insert_pattern(subsequence, seq_id, positions.empty() ? 0 : positions[0], true);
            }
        }
    }
    
    string join_pattern(const vector<string>& pattern, const string& delimiter = " -> ") {
        if (pattern.empty()) return "";
        
        ostringstream oss;
        for (size_t i = 0; i < pattern.size(); ++i) {
            if (i > 0) oss << delimiter;
            oss << pattern[i];
        }
        return oss.str();
    }
    
public:
    AdvancedSubsequenceMiner(int min_len = 2, int max_len = 10) 
        : min_length(min_len), max_length(max_len) {}
    
    int add_sequence(const vector<string>& sequence, int sequence_id = -1) {
        if (sequence_id == -1) {
            sequence_id = static_cast<int>(sequence_database.size());
        }
        
        sequence_database.emplace_back(sequence_id, sequence);
        statistics.total_sequences++;
        
        return sequence_id;
    }
    
    vector<PatternInfo> mine_frequent_patterns(int min_support = 2, bool noncontiguous_only = false) {
        auto start_time = high_resolution_clock::now();
        
        // Generate all subsequences
        for (const auto& seq_data : sequence_database) {
            generate_subsequences(seq_data.sequence, seq_data.id);
        }
        
        // Extract frequent patterns
        auto frequent_patterns = trie.get_frequent_patterns(min_support, noncontiguous_only);
        
        // Convert to PatternInfo objects
        vector<PatternInfo> enhanced_patterns;
        for (const auto& pattern_tuple : frequent_patterns) {
            const vector<string>& pattern = get<0>(pattern_tuple);
            int frequency = get<1>(pattern_tuple);
            bool is_noncontiguous = get<2>(pattern_tuple);
            
            double support = sequence_database.empty() ? 0.0 : 
                           static_cast<double>(frequency) / sequence_database.size();
            
            string pattern_string = join_pattern(pattern);
            string pattern_type = is_noncontiguous ? "Non-contiguous" : "Contiguous";
            
            enhanced_patterns.emplace_back(pattern, frequency, static_cast<int>(pattern.size()), 
                                         support, pattern_string, is_noncontiguous, pattern_type);
        }
        
        auto end_time = high_resolution_clock::now();
        statistics.mining_time = duration_cast<microseconds>(end_time - start_time).count() / 1000.0;
        statistics.total_patterns_found = static_cast<int>(enhanced_patterns.size());
        
        return enhanced_patterns;
    }
    
    vector<PatternInfo> find_top_k_patterns(int k = 10, int min_support = 2, 
                                           bool noncontiguous_only = false) {
        auto patterns = mine_frequent_patterns(min_support, noncontiguous_only);
        
        if (static_cast<int>(patterns.size()) <= k) {
            return patterns;
        }
        
        // Sort by frequency (descending) and take top k
        sort(patterns.begin(), patterns.end(), 
             [](const PatternInfo& a, const PatternInfo& b) {
                 return a.frequency > b.frequency;
             });
        
        patterns.resize(k);
        return patterns;
    }
    
    map<string, int> analyze_pattern_distribution() {
        auto patterns = mine_frequent_patterns(1);
        
        map<string, int> length_distribution;
        map<string, int> frequency_distribution;
        
        for (const auto& pattern : patterns) {
            length_distribution["length_" + to_string(pattern.length)]++;
            frequency_distribution["freq_" + to_string(pattern.frequency)]++;
        }
        
        map<string, int> result;
        result["total_unique_patterns"] = static_cast<int>(patterns.size());
        
        // Add distributions
        for (const auto& dist_pair : length_distribution) {
            result[dist_pair.first] = dist_pair.second;
        }
        
        return result;
    }
    
    MiningStatistics get_mining_statistics() {
        statistics.cache_hits = 0; // Placeholder
        return statistics;
    }
    
    void print_statistics() {
        cout << "\n=== Mining Statistics ===" << endl;
        cout << "Total sequences: " << statistics.total_sequences << endl;
        cout << "Total patterns found: " << statistics.total_patterns_found << endl;
        cout << "Mining time: " << fixed << setprecision(4) << statistics.mining_time << " ms" << endl;
        cout << "Trie size: " << trie.get_pattern_count() << endl;
        
        if (statistics.total_sequences > 0) {
            double avg_seq_len = 0.0;
            for (const auto& seq : sequence_database) {
                avg_seq_len += seq.length;
            }
            avg_seq_len /= statistics.total_sequences;
            cout << "Average sequence length: " << fixed << setprecision(2) << avg_seq_len << endl;
        }
    }
};

// Utility functions
vector<string> split_string(const string& str, char delimiter = ' ') {
    vector<string> tokens;
    stringstream ss(str);
    string token;
    
    while (getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

vector<vector<string>> get_demo_data() {
    return {
        {"login", "browse", "search", "view_item", "add_to_cart", "checkout"},
        {"login", "browse", "view_item", "add_to_cart", "checkout"},
        {"browse", "search", "view_item", "browse", "view_item", "add_to_cart"},
        {"login", "browse", "search", "view_item", "logout"},
        {"browse", "search", "view_item", "add_to_cart", "checkout", "logout"},
        {"login", "view_item", "add_to_cart", "checkout"},
        {"browse", "search", "search", "view_item", "add_to_cart"}
    };
}

vector<vector<string>> get_user_input() {
    cout << "\n" << string(50, '=') << endl;
    cout << "INTERACTIVE SUBSEQUENCE MINER" << endl;
    cout << string(50, '=') << endl;
    
    vector<vector<string>> sequences;
    cout << "\nEnter your sequences (space-separated elements per line)" << endl;
    cout << "Type 'done' when finished, 'demo' for sample data:" << endl;
    
    string line;
    cin.ignore(); // Clear input buffer
    
    while (true) {
        cout << "Sequence " << (sequences.size() + 1) << ": ";
        getline(cin, line);
        
        if (line == "done") {
            break;
        } else if (line == "demo") {
            return get_demo_data();
        } else if (!line.empty()) {
            auto sequence = split_string(line);
            sequences.push_back(sequence);
            cout << "Added sequence: ";
            for (size_t i = 0; i < sequence.size(); ++i) {
                if (i > 0) cout << " -> ";
                cout << sequence[i];
            }
            cout << endl;
        }
    }
    
    if (sequences.empty()) {
        cout << "No sequences provided. Using demo data." << endl;
        return get_demo_data();
    }
    
    return sequences;
}

void interactive_mining() {
    // Get sequences from user
    auto sequences = get_user_input();
    
    // Get mining parameters
    int k = 5, min_support = 2, max_length = 4;
    char pattern_type = 'b';
    
    cout << "\nEnter k for top-k patterns (default 5): ";
    string input;
    getline(cin, input);
    if (!input.empty()) {
        try {
            k = stoi(input);
        } catch (...) {
            cout << "Invalid input. Using default k=5." << endl;
        }
    }
    
    cout << "Enter minimum support (default 2): ";
    getline(cin, input);
    if (!input.empty()) {
        try {
            min_support = stoi(input);
        } catch (...) {
            cout << "Invalid input. Using default min_support=2." << endl;
        }
    }
    
    cout << "Enter maximum pattern length (default 4): ";
    getline(cin, input);
    if (!input.empty()) {
        try {
            max_length = stoi(input);
        } catch (...) {
            cout << "Invalid input. Using default max_length=4." << endl;
        }
    }
    
    cout << "Pattern type - (c)ontiguous, (n)on-contiguous, or (b)oth? (default: both): ";
    getline(cin, input);
    if (!input.empty()) {
        pattern_type = static_cast<char>(tolower(input[0]));
        if (pattern_type != 'c' && pattern_type != 'n' && pattern_type != 'b') {
            pattern_type = 'b';
        }
    }
    
    // Initialize miner
    AdvancedSubsequenceMiner miner(2, max_length);
    
    // Add sequences
    for (size_t i = 0; i < sequences.size(); ++i) {
        miner.add_sequence(sequences[i], static_cast<int>(i));
    }
    
    cout << "\n" << string(60, '=') << endl;
    cout << "MINING RESULTS" << endl;
    cout << string(60, '=') << endl;
    cout << "Dataset: " << sequences.size() << " sequences" << endl;
    cout << "Parameters: k=" << k << ", min_support=" << min_support 
         << ", max_length=" << max_length << endl;
    
    // Mine patterns based on user preference
    vector<PatternInfo> patterns;
    
    if (pattern_type == 'c') {
        cout << "\n🔍 MINING TOP-" << k << " CONTIGUOUS PATTERNS:" << endl;
        patterns = miner.find_top_k_patterns(k, min_support, false);
        // Filter out non-contiguous patterns
        patterns.erase(remove_if(patterns.begin(), patterns.end(), 
                                [](const PatternInfo& p) { return p.is_noncontiguous; }), 
                      patterns.end());
    } else if (pattern_type == 'n') {
        cout << "\n🔍 MINING TOP-" << k << " NON-CONTIGUOUS PATTERNS:" << endl;
        patterns = miner.find_top_k_patterns(k, min_support, true);
    } else {
        cout << "\n🔍 MINING TOP-" << k << " PATTERNS (ALL TYPES):" << endl;
        patterns = miner.find_top_k_patterns(k, min_support, false);
    }
    
    // Display results
    if (!patterns.empty()) {
        cout << "\nFound " << patterns.size() << " patterns:" << endl;
        cout << string(80, '-') << endl;
        
        for (size_t i = 0; i < patterns.size(); ++i) {
            cout << setw(2) << (i + 1) << ". " << setw(35) << left << patterns[i].pattern_string
                 << " [" << setw(12) << patterns[i].pattern_type << "] "
                 << "Freq: " << setw(2) << patterns[i].frequency << " "
                 << "Support: " << fixed << setprecision(2) << patterns[i].support << endl;
        }
        
        // Print statistics
        miner.print_statistics();
        
    } else {
        cout << "No patterns found with the given parameters." << endl;
        cout << "Try reducing min_support or increasing max_length." << endl;
    }
}

void demonstrate_subsequence_miner() {
    cout << "=== Advanced Subsequence Miner Demo ===" << endl;
    
    // Initialize miner
    AdvancedSubsequenceMiner miner(2, 4);
    
    // Sample data
    auto sample_sequences = get_demo_data();
    
    // Add sequences
    for (size_t i = 0; i < sample_sequences.size(); ++i) {
        miner.add_sequence(sample_sequences[i], static_cast<int>(i));
    }
    
    cout << "\nAdded " << sample_sequences.size() << " user interaction sequences" << endl;
    cout << "Example sequence: ";
    for (size_t i = 0; i < sample_sequences[0].size(); ++i) {
        if (i > 0) cout << " -> ";
        cout << sample_sequences[0][i];
    }
    cout << endl;
    
    // Mine patterns
    cout << "\n🔍 Mining frequent patterns (min_support=2)..." << endl;
    auto frequent_patterns = miner.mine_frequent_patterns(2);
    
    cout << "\nFound " << frequent_patterns.size() << " frequent patterns:" << endl;
    
    // Display top patterns
    for (size_t i = 0; i < min(static_cast<size_t>(10), frequent_patterns.size()); ++i) {
        cout << setw(2) << (i + 1) << ". " << setw(30) << left << frequent_patterns[i].pattern_string
             << " [" << setw(12) << frequent_patterns[i].pattern_type << "] "
             << "(freq: " << frequent_patterns[i].frequency 
             << ", support: " << fixed << setprecision(2) << frequent_patterns[i].support << ")" << endl;
    }
    
    // Top-k patterns
    cout << "\n=== Top 5 Most Frequent Patterns ===" << endl;
    auto top_patterns = miner.find_top_k_patterns(5, 2);
    
    for (size_t i = 0; i < top_patterns.size(); ++i) {
        cout << (i + 1) << ". " << top_patterns[i].pattern_string 
             << " [" << top_patterns[i].pattern_type << "] - Frequency: " 
             << top_patterns[i].frequency << endl;
    }
    
    // Non-contiguous patterns
    cout << "\n=== Top 3 Non-Contiguous Patterns ===" << endl;
    auto noncontiguous_patterns = miner.find_top_k_patterns(3, 2, true);
    
    for (size_t i = 0; i < noncontiguous_patterns.size(); ++i) {
        cout << (i + 1) << ". " << noncontiguous_patterns[i].pattern_string 
             << " - Frequency: " << noncontiguous_patterns[i].frequency << endl;
    }
    
    // Statistics
    miner.print_statistics();
}

int main() {
    cout << "Choose mode:" << endl;
    cout << "1. Interactive Mining (user input)" << endl;
    cout << "2. Demo with sample data" << endl;
    
    char choice;
    cout << "Enter choice (1 or 2): ";
    cin >> choice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear input buffer after reading choice
    
    if (choice == '1') {
        interactive_mining();
    } else {
        demonstrate_subsequence_miner();
    }
    
    return 0;
}