// Wordle simulator from
// https://github.com/TylerGlaiel/wordlebot

// Genetic algorithm code from
// https://github.com/repos-algorithms/genetic

// Conflicts algorithm thanks to Virgile Andreani at
// https://github.com/alexandres/magicwordschallenge/issues/2

#include <iostream>  // for cout etc.
#include <vector>    // for vector class
#include <string>    // for string class
#include <algorithm> // for sort algorithm
#include <time.h>    // for random seed
#include <math.h>    // for abs()
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
#include <random>
#include <set>

#define GA_POPSIZE 100       // ga population size
#define GA_MAXITER 1638400   // maximum iterations
#define GA_ELITSIZE 10       // elitism rate
#define GA_BIRTHSIZE 1       // elitism rate
#define GA_MUTATIONRATE 0.1f // mutation rate
#define GA_TARGET_SIZE 8

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

struct ga_struct
{
    std::vector<FiveLetterWord> str; // the string
    unsigned int conflicts;          // max hidden words solved
};

typedef std::vector<ga_struct> ga_vector; // for brevity

void init_population(ga_vector &population,
                     ga_vector &buffer)
{
    int tsize = GA_TARGET_SIZE;

    for (int i = 0; i < GA_POPSIZE; i++)
    {
        ga_struct citizen;

        citizen.conflicts = 0;
        citizen.str.clear();

        for (int j = 0; j < tsize; j++)
            citizen.str.push_back(guess_words[rand() % guess_words.size()]);

        population.push_back(citizen);
    }

    buffer.resize(GA_POPSIZE);
}

void calc_fitness(ga_vector &population)
{
    for (int i = 0; i < GA_POPSIZE; i++)
    {
        auto &citizen = population[i];
        citizen.conflicts = conflicts(citizen.str);
    }
}

bool fitness_sort(ga_struct x, ga_struct y)
{
    return x.conflicts < y.conflicts;
}

inline void sort_by_fitness(ga_vector &population)
{
    sort(population.begin(), population.end(), fitness_sort);
}

void elitism(ga_vector &population,
             ga_vector &buffer, int esize)
{
    for (int i = 0; i < esize; i++)
    {
        buffer[i].str = population[i].str;
        buffer[i].conflicts = population[i].conflicts;
    }
}

void mutate(ga_struct &member)
{
    member.str[rand() % member.str.size()] = guess_words[rand() % guess_words.size()];
}

void mate(ga_vector &population, ga_vector &buffer)
{
    int esize = GA_ELITSIZE;
    int tsize = population[0].str.size(), spos, i1, i2;

    elitism(population, buffer, esize);

    // Mate the rest
    for (int i = GA_POPSIZE - 1; i >= GA_ELITSIZE; i--)
    {
        i1 = rand() % GA_POPSIZE;
        i2 = rand() % GA_POPSIZE;
        spos = rand() % (population[i1].str.size() - 1);

        std::vector<FiveLetterWord> newStr;
        for (int j = 0; j < tsize; j++)
        {
            if (j <= spos)
                newStr.push_back(population[i1].str[j]);
            else
                newStr.push_back(population[i2].str[j]);
        }
        buffer[i].str = newStr;
    }
    for (int i = 1; i < GA_POPSIZE; i++)
    {
        if (rand() < (GA_MUTATIONRATE * RAND_MAX))
            mutate(buffer[i]);
    }
    for (int i = GA_POPSIZE - 1; i >= GA_POPSIZE - GA_BIRTHSIZE; i--)
    {
        for (int j = 0; j < tsize; j++)
            buffer[i].str[j] = guess_words[rand() % guess_words.size()];
    }
}

inline void print_best(ga_vector &gav)
{
    std::cout << "\tBest:\t";
    for (auto &guess : gav[0].str)
    {
        std::cout << guess.to_s() << " ";
    }
    std::cout << "\tConflicts:\t" << gav[0].conflicts;
}

inline void swap(ga_vector *&population,
                 ga_vector *&buffer)
{
    ga_vector *temp = population;
    population = buffer;
    buffer = temp;
}

int main()
{
    hidden_words = LoadWordList("words.txt");
    guess_words = LoadWordList("words_full.txt");
    std::cerr << "Words in Hidden Dictionary: " << hidden_words.size() << std::endl;
    std::cerr << "Words in Guess Dictionary: " << guess_words.size() << std::endl;

    srand(unsigned(time(NULL)));

    ga_vector pop_alpha, pop_beta;
    ga_vector *population, *buffer;

    init_population(pop_alpha, pop_beta);
    population = &pop_alpha;
    buffer = &pop_beta;

    for (int i = 0; i < GA_MAXITER; i++)
    {
        // std::cerr << "calcfitness" << std::endl;
        calc_fitness(*population); // calculate fitness
        // std::cerr << "sortfitness" << std::endl;
        sort_by_fitness(*population); // sort them
        std::cout << "Iter:\t" << i << "\t";
        print_best(*population); // print the best one
        std::cout << std::endl;

        if ((*population)[0].conflicts == 0)
            break;

        mate(*population, *buffer); // mate the population together
        swap(population, buffer);   // swap buffers
    }
    std::cout << std::endl;

    return 0;
}
