#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <tuple>
#include <algorithm>
#include <filesystem>
#include <cctype>

using namespace std;
namespace fs = std::filesystem;

// Constants
const int MAX_FREQUENT_WORDS = 100;
const int TOTAL_TEXT_BOOKS = 64;

// Array of words to be excluded from word Similarity calculation
const string excludedWords[] = {"a", "and", "an", "of", "in", "the"};

// Function to check if a word is in the list of the excludedWords
bool isExcluded(const string &word) {
    for (const string &excluded : excludedWords) {
        if (word == excluded) {
            return true;
        }
    }
    return false;
}

// Function to normalize a word by converting to uppercase and removing non-alphanumeric characters
string normalizeWord(const string &word) {
    string normalized;
    for (char ch : word) {
        if (isalnum(ch)) {  // Check if character is alphanumeric
            normalized += toupper(ch);  // Convert to uppercase
        }
    }
    return normalized;
}

// Function to get the top 100 most frequent words in a file
map<string, double> getTopWords(const string &filePath) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filePath << endl;
        return {};
    }

    map<string, int> wordCounts;
    int totalWords = 0;
    string word;

    // Read each word, normalize, and count if it's not in the exclusion list
    while (file >> word) {
        string normalizedWord = normalizeWord(word);
        if (!normalizedWord.empty() && !isExcluded(normalizedWord)) {
            wordCounts[normalizedWord]++;
            totalWords++;
        }
    }
    file.close();

    // To Calculate normalized frequency for each word and store in a vector
    vector<pair<string, double>> freqList;
    for (const auto &pair : wordCounts) {
        freqList.push_back({pair.first, static_cast<double>(pair.second) / totalWords});
    }

    // Sort the list by frequency in descending order and keep the top 100 words
    sort(freqList.begin(), freqList.end(), [](const auto &a, const auto &b) {
        return a.second > b.second;
    });
    if (freqList.size() > MAX_FREQUENT_WORDS) freqList.resize(MAX_FREQUENT_WORDS);

    // Transfer top 100 words with their frequencies to a map
    map<string, double> topWords;
    for (const auto &pair : freqList) {
        topWords[pair.first] = pair.second;
    }

    return topWords;
}

// To calculate similarity between two files based on common word frequencies
double calculateSimilarity(const map<string, double> &freqA, const map<string, double> &freqB) {
    double similarityValue = 0.0;
    for (const auto &pair : freqA) {
        auto it = freqB.find(pair.first);
        if (it != freqB.end()) {
            similarityValue += pair.second + it->second;
        }
    }
    return similarityValue;
}

// Function to find and display the top 10 pairs of similar files
void findTopSimilarPairs(const vector<string> &files) {
    int numFiles = files.size();

    // Check that there are exactly 64 files
    if (numFiles != TOTAL_TEXT_BOOKS) {
        cerr << "Error: Expected " << TOTAL_TEXT_BOOKS << " files, but found " << numFiles << endl;
        return;
    }

    // Compute word frequencies for each file and store in a vector
    vector<map<string, double>> fileFrequencies(numFiles);
    for (int i = 0; i < numFiles; i++) {
        fileFrequencies[i] = getTopWords(files[i]);
    }

    // For creating a 64x64 matrix to store similarity scores
    vector<vector<double>> similarityMatrix(numFiles, vector<double>(numFiles, 0.0));
    for (int i = 0; i < numFiles; i++) {
        for (int j = i + 1; j < numFiles; j++) {
            double score = calculateSimilarity(fileFrequencies[i], fileFrequencies[j]);
            similarityMatrix[i][j] = score;
            similarityMatrix[j][i] = score;  // The Matrix is symmetric
        }
    }

    // Store similarity scores along with the file indices in a vector of tuples
    vector<tuple<double, int, int>> similarityValues;
    for (int i = 0; i < numFiles; i++) {
        for (int j = i + 1; j < numFiles; j++) {
            similarityValues.emplace_back(similarityMatrix[i][j], i, j);
        }
    }

    // Sort the similarity scores in descending order to get the top result
    sort(similarityValues.rbegin(), similarityValues.rend());

    // Display the top 10 most similar file pairs by name only
    cout << "Top 10 similar pairs of books:\n";
    for (int k = 0; k < 10 && k < similarityValues.size(); k++) {
        auto [score, fileIndexA, fileIndexB] = similarityValues[k];
        cout << "\"" << fs::path(files[fileIndexA]).filename().string()
             << "\" and \"" << fs::path(files[fileIndexB]).filename().string() << "\"" << endl;
    }
}

int main() {
    string directoryPath = "./BOOKS";  // Directory containing text files
    vector<string> files;

    // Load all .txt files from the directory
    for (const auto &entry : fs::directory_iterator(directoryPath)) {
        if (entry.path().extension() == ".txt") {
            files.push_back(entry.path().string());
        }
    }

    // Find and display the top 10 most similar pairs of files
    findTopSimilarPairs(files);
    return 0;
}
