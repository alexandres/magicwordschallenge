# Magic words, a Wordle challenge!

TLDR: the 8 initial guesses `MODEL LEVIN TAPPA GRABS DURGY FLYTE CHAWK SPOOR` provide enough information to *always* recover the hidden word in Wordle, no matter the hidden word. Can you find fewer than 8 (magic) words that also solve Wordle?

## Updates

- May 18th, 2022: [Laurent Poirrier](https://www.poirrier.ca/) has provided a definitive (negative) answer to the Magic Words Challenge: *at least 6 + 1 guesses are required to solve Wordle offline*, and so the 6 guess formulation of the original game cannot be solved offline. He has written [a wonderful post](https://www.poirrier.ca/notes/wordle-fixed/) about the result. **The Magic Words Challenge is now officially ended!**

- March 5th, 2022: [Virgile Andreani](https://github.com/Armavica) has [found a set of 6 words](https://github.com/alexandres/magicwordschallenge/issues/2) that solve Wordle! They are `COMBO FATTY GRRRL SPUDS VENGE WHILK`. This is very exciting: just one less word and Wordle will be solved offline!

- March 2nd, 2022: [Virgile Andreani](https://github.com/Armavica) has [found a set of 7 words](https://github.com/alexandres/magicwordschallenge/issues/1) that solve Wordle! They are `CLANG FATTY ODDER RUMBA SKILL VERGE WHOOP`.

## Solving Wordle offline

[Other solvers](https://www.poirrier.ca/notes/wordle-optimal/) can solve Wordle in fewer guesses, but they do so *online*, using feedback from each guess to decide upon the next guess.

My goal is to solve Wordle offline: remember these 8 words and you can solve Wordle on a desert island were your life to depend upon it (important: remember to also take the `2309` word list with you!).

The fundamental question is: what is the smallest set of guesses which always solve Wordle? I've shown it can be solved with 8 words. Can it be solved with 7? 6? 5? 5 is the number required if the islanders described above insist on 6 round Wordle (the official version!).

## How I found the current 8 word solution

I used a simple [genetic algorithm](https://github.com/repos-algorithms/genetic) and a [Wordle simulator](https://github.com/TylerGlaiel/wordlebot). The [https://github.com/alexandres/magicwordschallenge/issues/2](algorithm that checks whether a solution is valid) is thanks to [Virgile Andreani](https://github.com/Armavica).

If you'd like to solve for 7 words, change `GA_TARGET_SIZE` to 7 in `magicwords.cpp`. I ran the system for a couple of days searching for 7 words and failed. Maybe you'll have better luck.

To run, 

```bash
$> make
$> ./magicwords
```

## Check your solutions

Input your candidate solutions (one per line) on `stdin` to the `./check` program. For each candidate solution, it will output `1` if it solves Wordle, `0` if it doesn't.

```bash
$> make
$> echo MODEL LEVIN TAPPA GRABS DURGY FLYTE CHAWK SPOOR | ./check
# outputs 1
$> echo MODEL LEVIN TAPPA GRABS DURGY FLYTE CHAWK PIANO | ./check
# outputs 0
```



