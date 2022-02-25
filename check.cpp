// Wordle simulator from
// https://github.com/TylerGlaiel/wordlebot

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <atomic>
#include <thread>
#include <chrono>
#include <array>
#include <climits>
#include <sstream>

enum GuessResult
{
    DOES_NOT_EXIST = 0,
    EXISTS_IN_DIFFERENT_SPOT = 1,
    CORRECT = 2,
};

struct FiveLetterWord
{
    char word[5];
    FiveLetterWord(std::string in = "     ")
    {
        if (in.size() != 5)
            throw std::runtime_error("word is not 5 letters: '" + in + "'");
        for (int i = 0; i < 5; i++)
            word[i] = in[i];
    }

    char &operator[](int i) { return word[i]; }
    const char &operator[](int i) const { return word[i]; }
    bool operator==(const FiveLetterWord &rhs) const { return word[0] == rhs.word[0] && word[1] == rhs.word[1] && word[2] == rhs.word[2] && word[3] == rhs.word[3] && word[4] == rhs.word[4]; }
    bool operator!=(const FiveLetterWord &rhs) const { return word[0] != rhs.word[0] || word[1] != rhs.word[1] || word[2] != rhs.word[2] || word[3] != rhs.word[3] || word[4] != rhs.word[4]; }

    std::string to_s() const
    {
        return std::string(word, word + 5);
    }

    std::string to_squares() const
    {
        std::string res;

        for (int i = 0; i < 5; i++)
        {
            if (word[i] == DOES_NOT_EXIST)
                res += "x";
            if (word[i] == EXISTS_IN_DIFFERENT_SPOT)
                res += "Y";
            if (word[i] == CORRECT)
                res += "G";
        }

        return res;
    }

    bool is_correct() const
    {
        for (int i = 0; i < 5; i++)
        {
            if (word[i] != CORRECT)
                return false;
        }

        return true;
    }

    int to_score() const
    {
        int res = 0;
        for (int i = 0; i < 5; i++)
            res += word[i];
        return res;
    }
};

FiveLetterWord first_guess("ROATE");
FiveLetterWord (*Strategy)(const std::vector<FiveLetterWord> &, const std::vector<FiveLetterWord> &, bool);

typedef FiveLetterWord WordHint;

WordHint from_hint(std::string hint)
{
    WordHint res;
    for (int i = 0; i < 5; i++)
    {
        if (hint[i] == 'x' || hint[i] == 'X')
            res[i] = DOES_NOT_EXIST;
        if (hint[i] == 'y' || hint[i] == 'Y')
            res[i] = EXISTS_IN_DIFFERENT_SPOT;
        if (hint[i] == 'g' || hint[i] == 'G')
            res[i] = CORRECT;
    }
    return res;
}

WordHint evaluate_guess(const FiveLetterWord &guess, FiveLetterWord actual)
{
    WordHint result = {{DOES_NOT_EXIST, DOES_NOT_EXIST, DOES_NOT_EXIST, DOES_NOT_EXIST, DOES_NOT_EXIST}};

    // do green squares
    for (int i = 0; i < 5; i++)
    {
        if (guess[i] == actual[i])
        {
            result[i] = CORRECT;
            actual[i] = 0;
        }
    }

    // do yellow squares
    for (int i = 0; i < 5; i++)
    {
        if (result[i] != CORRECT)
        { // check the ones that were not marked as correct
            for (int j = 0; j < 5; j++)
            {
                if (guess[i] == actual[j])
                {
                    result[i] = EXISTS_IN_DIFFERENT_SPOT;
                    actual[j] = 0;
                    break;
                }
            }
        }
    }

    return result;
}

bool IsWordPossible(const WordHint &hint, const FiveLetterWord &guess, FiveLetterWord word)
{
    // check greens
    for (int i = 0; i < 5; i++)
    {
        if (hint[i] == CORRECT)
        {
            if (guess[i] != word[i])
                return false;
            word[i] = 0; // for yellows
        }
    }
    // check yellows
    for (int i = 0; i < 5; i++)
    {
        if (hint[i] == EXISTS_IN_DIFFERENT_SPOT)
        {
            if (guess[i] == word[i])
                return false; // this would have been green, not yellow, so it fails

            bool found = false;
            for (int j = 0; j < 5; j++)
            {
                if (guess[i] == word[j])
                {
                    found = true;
                    word[j] = 0; // for yellows
                    break;
                }
            }
            if (!found)
                return false;
        }
    }
    // check greys
    for (int i = 0; i < 5; i++)
    {
        if (hint[i] == DOES_NOT_EXIST)
        {
            for (int j = 0; j < 5; j++)
            {
                if (guess[i] == word[j])
                {
                    return false;
                }
            }
        }
    }

    return true;
}

std::vector<FiveLetterWord> FilterWordList(const WordHint &hint, const FiveLetterWord &guess, const std::vector<FiveLetterWord> &wordlist)
{
    std::vector<FiveLetterWord> res;

    for (auto &word : wordlist)
    {
        if (IsWordPossible(hint, guess, word))
            res.push_back(word);
    }

    return res;
}

std::vector<FiveLetterWord> LoadWordList(std::string filename)
{
    std::vector<FiveLetterWord> res;
    std::ifstream file(filename);
    std::string str;
    while (std::getline(file, str))
    {
        if (str[str.length() - 1] == '\r')
            str.erase(str.length() - 1);
        for (auto &c : str)
            c = toupper(c);
        res.push_back(str);
    }
    return res;
}

int main()
{
    std::vector<FiveLetterWord> hidden_words = LoadWordList("words.txt");
    std::cerr << "Words in Hidden Dictionary: " << hidden_words.size() << std::endl;
    while (true)
    {
        std::string line;
        std::getline(std::cin, line);
        if (line.size() == 0)
            break;
        std::cerr << line << std::endl;
        std::vector<FiveLetterWord> solution;
        std::stringstream ss(line);
        std::string word;
        while (std::getline(ss, word, ' '))
        {
            for (auto &c : word)
                c = toupper(c);
            solution.push_back(FiveLetterWord(word));
        }
        auto is_solution = 1;
        for (auto &hidden : hidden_words)
        {
            std::vector<FiveLetterWord> possible_words = hidden_words;
            for (auto &guess : solution)
            {
                auto hint = evaluate_guess(guess, hidden);
                possible_words = FilterWordList(hint, guess, possible_words);
            }
            if (possible_words.size() > 1)
            {
                is_solution = 0;
                break;
            }
        }
        std::cout << is_solution << std::endl;
    }
    return 0;
}
