// Wordle simulator from
// https://github.com/TylerGlaiel/wordlebot

// Conflicts algorithm thanks to Virgile Andreani at
// https://github.com/alexandres/magicwordschallenge/issues/2

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
#include <set>

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

std::vector<FiveLetterWord> hidden_words, guess_words;

int conflicts(std::vector<FiveLetterWord> solution)
{
    std::set<std::vector<std::string>> all_feedbacks;
    for (auto &hidden : hidden_words)
    {
        std::vector<std::string> feedbacks;
        for (auto &guess : solution)
        {
            auto hint = evaluate_guess(guess, hidden);
            feedbacks.push_back(hint.to_squares());
        }
        all_feedbacks.insert(feedbacks);
    }
    return hidden_words.size() - all_feedbacks.size();
}

int main()
{
    hidden_words = LoadWordList("words.txt");
    guess_words = LoadWordList("words_full.txt");
    std::cerr << "Words in Hidden Dictionary: " << hidden_words.size() << std::endl;
    std::cerr << "Words in Guess Dictionary: " << guess_words.size() << std::endl;
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
            auto guess = FiveLetterWord(word);
            auto found = false;
            for (auto &valid_guess : guess_words)
            {
                if (valid_guess == guess)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                std::cerr << "Error: guess " << guess.to_s() << "  not in dictionary" << std::endl;
                exit(1);
            }
            solution.push_back(guess);
        }
        auto is_solution = conflicts(solution) == 0 ? 1 : 0;
        std::cout << is_solution << std::endl;
    }
    return 0;
}
