/*----------------------------------------------
Progran: Spelling Bee Game & Solver
    Small console program that lets you set
    a hive and then finds valid words. 
    You can also play by typing yourself.
Course: CS211, Fall 2025, UIC
Author: Eldin Nokic
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

// constants for the game
const int MIN_HIVE_SIZE = 2;
const int MAX_HIVE_SIZE = 12;
const int MIN_WORD_LENGTH = 4;

// struct that is used to hold lists of words
typedef struct WordList_struct {
    char** words; // stores an array of pointers to words
    int numWords; // how many words are in the list
    int capacity; // how much space we currenlty have 
} WordList;

/*
purpose: make an empty dynamic list of words with some starter capacity
parameters: none
returns: pointer to a new WordList on the heap
*/
WordList* createWordList() {
    WordList* newList = malloc(sizeof(WordList)); // allocates spaces for the struct
    newList->capacity = 4; // start with 4
    newList->numWords = 0; // no words yet
    newList->words = malloc(newList->capacity * sizeof(char*)); // allocate array of word pointers

    return newList;
}

/*
purpose: append a copy of newWord to the end of the list, grow array if needed
parameters: thisWordList (list we add into), newWord (string to copy)
returns: nothing (list is updates in place)
*/
void appendWord(WordList* thisWordList, char* newWord) {
    // grow the array when full 
    if (thisWordList->numWords >= thisWordList->capacity) {

        int newCap = thisWordList->capacity * 2;
        char** newList = (char**)malloc(newCap * sizeof(char*));

        // copy existing pointers over
        for (int i = 0; i < thisWordList->numWords; i++) {
            newList[i] = thisWordList->words[i];
        }

        // swap and update capacity
        free(thisWordList->words);
        thisWordList->words = newList;
        thisWordList->capacity = newCap;
    }
    // store a fresh copy of the word 
    int length = strlen(newWord);
    char* copy = malloc((length + 1) * sizeof(char));
    strcpy(copy, newWord);

    thisWordList->words[thisWordList->numWords] = copy;
    thisWordList->numWords += 1;

}

/*
purpose: read words from a file and add only words of length >= minLength
parameters: filename (dictionary path), dictionaryList (output list), minLength
returns: length of the longest word added, or -1 on error
*/
int buildDictionary(char* filename, WordList* dictionaryList, int minLength) {

    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        return -1;
    }
    
    int added = 0;
    int longest = -1;
    char word[128];

    // scan token by token, add if long enough, track max length
    while (fscanf(f, "%127s", word) == 1) {
        int length = strlen(word);
        if (length >= minLength) {
            appendWord(dictionaryList, word);
            added++;
            if (length > longest) {
                longest = length;
            }
        }
    }

    fclose(f);

    if (added == 0) {
        return -1;
    }

    return longest;
}

/*
purpose: free all heap memory tied to a WordList (words + array + struct)
parameters: list 
returns: nothing
*/
void freeWordList(WordList* list) {
    if (list == NULL) {
        return;
    }

    for (int i = 0; i < list->numWords; i++) {
        free(list->words[i]);
    }
    free(list->words);
    free(list);
}

/*
purpose: find index of aLet in str
parameters: str, aLet (char to find)
returns: index if found, -1 otherwise
*/
int findLetter(char* str, char aLet) {
    for (int i = 0; str[i] != '\0'; ++i) {
        if (str[i] == aLet) {
            return i;
        }
    }
    return -1;
}

/*
purpose: given some letters, produce a unique-sorted hive string 
parameters: str, give (ouput buffer, must be big enough)
returns: nothing
*/
void buildHive(char* str, char* hive) {
    int already[26] = {0};
    for (int i = 0; str[i] != '\0'; i++) {
        if ('a' <= str[i] && str[i] <= 'z') {
            already[str[i] - 'a'] = 1;
        }
    }
    int count = 0;
    for (char c = 'a'; c <= 'z'; c++) {
        if (already[c - 'a'] == 1) {
            hive[count] = c;
            count++;
        }
    }
    hive[count] = '\0';
}

/*
purpose: count how many distinct lowercase letters are in str 
parameters: str 
returns: number of unique letters 0-26
*/
int countUniqueLetters(char* str) {
    int already[26] = {0};
    int count = 0;

    for (int i = 0; str[i] != '\0'; i++){
        if (str[i] >= 'a' && str[i] <= 'z') {
            int index = str[i] - 'a';
            if (already[index] == 0){
                already[index] = 1;
                count++;
            }
        }
    }
    return count;
}

/*
purpose: filter dictionary to only words that use exactly hiveSize unique letters
parameters: dictionaryList, hiveSize
returns: new WordList containing those words
*/WordList* findAllFitWords(WordList* dictionaryList, int hiveSize) {
    WordList* fitWords = createWordList();
    
    for (int i = 0; i < dictionaryList->numWords; i++) {
        char* w = dictionaryList->words[i];
        int u = countUniqueLetters(w);
        if (u == hiveSize){
            appendWord(fitWords, w);
        }
    }
    return fitWords;
}

/*
purpose: check if word uses only letters from hive and includes reqLet atleast once 
parameters: word, hive, reqLet (required letter)
returns: true if valid for this hive, false otherwise 
*/
bool isValidWord(char* word, char* hive, char reqLet) {

    int length = strlen(word);
    bool isRequired = false;
    // every letter must be fro hive: track if we saw the required letter
    for (int i = 0; i < length; i++) {
        if (findLetter(hive, tolower(word[i])) == -1) {
            return false;
        }
        if (tolower(word[i]) == reqLet) {
            isRequired = true;
        }
    }

    return isRequired;
}

/*
purpose: check if str uses all letters from hive at least once 
parameters: str (candidate), hive (the whole hive letters)
returns: true if pangram for this hive, else false 
*/
bool isPangram(char* str, char* hive) {

    int hiveCount = 0;
    int foundCount = 0;
    int count[26] = {0};
    // count how many hive letters exist and how many we actually hit in str 
    for (int i = 0; hive[i] != '\0'; i++) {
        hiveCount++;
    }

    for (int i = 0; str[i] != '\0'; i++) {
        char c = tolower(str[i]);
        if (findLetter(hive,c) != -1) {
            int index = c - 'a';
            if (count[index] == 0) {
                count[index] = 1;
                foundCount++;
            }
        }
    }

    return (foundCount == hiveCount);
}

/*
purpose: display hive letters and visually point to the required letter 
parameters: hive, reqLetInd (index of required letter in hive)
returns: nothing
*/
void printHive(char* hive, int reqLetInd) {
    printf("  Hive: \"%s\"\n", hive);
    printf("         ");
    for (int i = 0; i < reqLetInd; i++) {
        printf(" ");
    }
    printf("^");
    for (int i = reqLetInd + 1; i < strlen(hive); i++) {
        printf(" ");
    }
    printf(" (all words must include \'%c\')\n\n", hive[reqLetInd]);
}

/*
purpose: show all words in a WordList with their scores
parameters: thisWordList (words to show), hive 
returns: nothing
*/
void printList(WordList* thisWordList, char* hive) {
    printf("  Word List:\n");
    int totScore = 0;
    for (int i = 0; i < thisWordList->numWords; i++) {
        char* w = thisWordList->words[i];
        int length = strlen(w);
        int points;
        if (length == 4) {
            points = 1;
        }
        else {
            points = length;
        }
        if (isPangram(w,hive)){
            points += strlen(hive);
            totScore += points;
        }
        printf("    %s\n", w);
    }
    printf("  Total Score: %d\n", totScore);
}

/*
purpose: go through every dictionary word and keep it if valid under this hive 
parameters: dictionaryList (all words), solvedList (output), hive, reqlet 
returns: nothing 
*/void bruteForceSolve(WordList* dictionaryList, WordList* solvedList, char* hive, char reqLet) {
    for (int i = 0; i < dictionaryList->numWords; i++) {
        if (isValidWord(dictionaryList->words[i], hive, reqLet)) {
            appendWord(solvedList, dictionaryList->words[i]);
        }
    }
}

/*
purpose: check if partWord is a prefix of fullWord
parameters: partWord (shorter maybe), fullWord (longer)
returns: true if partWord matches from fullWord[0...length-1], else false 
*/bool isPrefix(char* partWord, char* fullWord) {
    // compare char by char until partWord ends
    int i = 0;
    while (partWord[i] != '\0') {
        if (partWord[i] != fullWord[i]) {
            return false;
        }
        i++;
    }
    if (partWord[i] == '\0') {
        return true;
    }
    return false; 
}

/*
purpose: recursive binary search for exact aWord: also detect prefix matches 
parameters: thisWordList, aWord (target/prefix), loInd, hiInd
returns: index if exact match. -1 if aWord is a prefix, -99 if no match and not a prefix
*/int findWord(WordList* thisWordList, char* aWord, int loInd, int hiInd) {
    if (hiInd < loInd) { // Base case 2: aWord not found in words[]

        if (loInd < thisWordList->numWords && isPrefix(aWord, thisWordList->words[loInd])) { 
            return -1; //words match this root (partial match)
        }
        else {
            return -99; //no more words matching this root (no match)
        }
    }

    int mdInd = (hiInd + loInd) / 2;

    if (strcmp(aWord, thisWordList->words[mdInd]) == 0) { // Base case 1: found tryWord at midInd
        return mdInd;
    }
    else if (strcmp(aWord, thisWordList->words[mdInd]) > 0) { // Recursive case: search upper half
        return findWord(thisWordList, aWord, mdInd + 1, hiInd);
    }
    // Recursive case: search lower half
    else {
        return findWord(thisWordList, aWord, loInd, mdInd - 1);
    }
}

/*
purpose: optimized search that walks the hive-space like a tree, using 
binary search to prune 
parameters: dictionaryList, solvedList, tryWord, hive, reqlet
returns: nothing
*/void findAllMatches(WordList* dictionaryList, WordList* solvedList, char* tryWord, char* hive, char reqLet) {
    // check if tryWord is exact word in dictionary
    int curLen = strlen(tryWord);
    int index = findWord(dictionaryList, tryWord, 0, dictionaryList->numWords - 1);
    if (index >= 0) {
        // if length is ok and passes hive rules, add if not already present
        if (curLen >= MIN_WORD_LENGTH && isValidWord(tryWord, hive, reqLet)) {
            bool alreadySeen = false;
            for (int i = 0; i < solvedList->numWords; i++) {
                if (strcmp(solvedList->words[i], tryWord) == 0) {
                    alreadySeen = true;
                    break;
                }
            }
            if (!alreadySeen) {
                appendWord(solvedList, tryWord);
            }
        }
        // after a hit, extend by first hive letter 
        tryWord[curLen] = hive[0];
        tryWord[curLen + 1] = '\0';
        if (tryWord[0] != '\0') {
            findAllMatches(dictionaryList, solvedList, tryWord, hive, reqLet);
        }
        return;
    }
    if (index == -1) {
        // extend by first hive letter and keep going
        tryWord[curLen] = hive[0];
        tryWord[curLen + 1] = '\0';
        if (tryWord[0] != '\0') {
            findAllMatches(dictionaryList, solvedList, tryWord, hive, reqLet);
        }
        return;
    }
    else if (index == -99) {
        // no match on this branch
        if (curLen == 0) {
            return;
        }
        int position = findLetter(hive, tryWord[curLen - 1]);

        if (position != -1 && position < strlen(hive) - 1) {
            tryWord[curLen - 1] = hive[position + 1];
        }
        else {
            tryWord[curLen - 1] = '\0';
            // go back until we can advance the last letter 
            while (tryWord[0] != '\0') {
                int p = findLetter(hive, tryWord[strlen(tryWord) - 1]);
                if (p != -1 && p < strlen(hive) - 1) {
                    tryWord[strlen(tryWord) - 1] = hive[p + 1];
                    break;
                }
                tryWord[strlen(tryWord) - 1] = '\0';
            }
        }
    }

    //call recursive function here ONLY if we still have a partial word left 
    if (tryWord[0] != '\0') {
        findAllMatches(dictionaryList, solvedList, tryWord, hive, reqLet);
    }
}

/*
purpose: parse CLI flags and set program modes + file name
parameters: argc/argv, ouput booleans + ints for modes and hive size, dict file path 
returns: true if args ok, false otherwise
flags: 
    -r <num> random hive of size num 
    -d <file> dictionary file 
    -s <seed> set srand seed 
    -p play mode 
    -o optimized solver  
*/
bool setSettings(int argc, char* argv[], bool* pRandMode, int* pNumLets, char dictFile[100], bool* pPlayMode, bool* pBruteForceMode, bool* pSeedSelection) {
    *pRandMode = false;
    *pNumLets = 0;
    strcpy(dictFile, "dictionary.txt");
    *pPlayMode = false;
    *pBruteForceMode = true;
    *pSeedSelection = false;
    srand((int)time(0));
    //--------------------------------------
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-r") == 0) {
            ++i;
            if (argc == i) {
                return false;
            }
            *pRandMode = true;
            *pNumLets = atoi(argv[i]);
            if (*pNumLets < MIN_HIVE_SIZE || *pNumLets > MAX_HIVE_SIZE) {
                return false;
            }
        }
        else if (strcmp(argv[i], "-d") == 0) {
            ++i;
            if (argc == i) {
                return false;
            }
            strcpy(dictFile, argv[i]);
            FILE* filePtr = fopen(dictFile, "r");
            if (filePtr == NULL) {
                return false;
            }
            fclose(filePtr);
        }
        else if (strcmp(argv[i], "-s") == 0) {
            ++i;
            if (argc == i) {
                return false;
            }
            *pSeedSelection = true;
            int seed = atoi(argv[i]);
            srand(seed);
        }
        else if (strcmp(argv[i], "-p") == 0) {
            *pPlayMode = true;
        }
        else if (strcmp(argv[i], "-o") == 0) {
            *pBruteForceMode = false;
        }
        else {
            return false;
        }
    }
    return true;
}


/*
purpose: helpers to print ON/OFF for bool flags 
parameters: mode (bool)
returns: nothing 
*/
void printONorOFF(bool mode) {
    if (mode) {
        printf("ON\n");
    }
    else {
        printf("OFF\n");
    }
}

void printYESorNO(bool mode) {
    if (mode) {
        printf("YES\n");
    }
    else {
        printf("NO\n");
    }
}


/*
purpose: drive program flow - parse flags, build dictionary, set hive optionally play 
then run solver, and print summary stats 
parameters: standard argc/argv 
returns: 0 on sucess, nonzero on early error 
*/
int main(int argc, char* argv[]) {

    printf("\n----- Welcome to the CS 211 Spelling Bee Game & Solver! -----\n\n");

    // variables for settings and hive setup
    bool randMode = false;
    int hiveSize = 0;
    char dict[100] = "dictionary.txt";
    bool playMode = false;
    bool bruteForce = true;
    bool seedSelection = false;
    char hive[MAX_HIVE_SIZE + 1] = {};
    hive[0] = '\0';
    int reqLetInd = -1;
    char reqLet = '\0';

    // read command-line arguments using setSettings
    if (!setSettings(argc, argv, &randMode, &hiveSize, dict, &playMode, &bruteForce, &seedSelection)) {
        printf("Invalid command-line argument(s).\nTerminating program...\n");
        return 1;
    }
    else {
        // print the chosen setting
        printf("Program Settings:\n");
        printf("  random mode = ");
        printONorOFF(randMode);
        printf("  play mode = ");
        printONorOFF(playMode);
        printf("  brute force solution = ");
        printONorOFF(bruteForce);
        printf("  dictionary file = %s\n", dict);
        printf("  hive set = ");
        printYESorNO(randMode);
        printf("\n\n");
    }

    // build word array (only words with desired minimum length or longer) from dictionary file
    printf("Building array of words from dictionary... \n");
    WordList* dictionaryList = createWordList();
    int maxWordLength = buildDictionary(dict, dictionaryList, MIN_WORD_LENGTH);
    if (maxWordLength == -1) {
        printf("  ERROR in building word array.\n");
        printf("  File not found or incorrect number of valid words.\n");
        printf("Terminating program...\n");
        return -1;
    }
    printf("   Word array built!\n\n");


    printf("Analyzing dictionary...\n");

    if (dictionaryList->numWords < 0) {
        printf("  Dictionary %s not found...\n", dict);
        printf("Terminating program...\n");
        return -1;
    }

    // end program if file has zero words of minimum desired length or longer
    if (dictionaryList->numWords == 0) {
        printf("  Dictionary %s contains insufficient words of length %d or more...\n", dict, MIN_WORD_LENGTH);
        printf("Terminating program...\n");
        return -1;
    }
    else {
        printf("  Dictionary %s contains \n  %d words of length %d or more;\n", dict, dictionaryList->numWords, MIN_WORD_LENGTH);
    }


    if (randMode) {
        printf("==== SET HIVE: RANDOM MODE ====\n");
        //find number of words in words array that use hiveSize unique letters
        WordList* fitWords = findAllFitWords(dictionaryList, hiveSize);
        int numFitWords = fitWords->numWords;
        //pick one at random
        int pickOne = rand() % numFitWords;
        char* chosenFitWord = fitWords->words[pickOne];

        //and alaphabetize the unique letters to make the letter hive
        buildHive(chosenFitWord, hive);
        freeWordList(fitWords);

        reqLetInd = rand() % hiveSize;
        reqLet = hive[reqLetInd];

    }
    else {
        printf("==== SET HIVE: USER MODE ====\n");
        printf("  Enter a single string of lower-case,\n  unique letters for the letter hive... ");

        char input[100];

        while (true) {
            scanf("%s", input);
            int length = strlen(input);
            bool validInput = true;

            if (length < MIN_HIVE_SIZE || length > MAX_HIVE_SIZE) {
                printf("  HIVE ERROR: \"%s\" has invalid length;\n  valid hive size is between %d and %d, inclusive\n\n",input, MIN_HIVE_SIZE, MAX_HIVE_SIZE);
                printf("  Enter a single string of lower-case,\n  unique letters for the letter hive... ");
                continue;
            }

            for (int i = 0; i < length; i++) {
                if (input[i] < 'a' || input[i] > 'z') {
                    printf("  HIVE ERROR: \"%s\" contains invalid letters;\n  valid characters are lower-case alpha only\n\n",input);
                    printf("  Enter a single string of lower-case,\n  unique letters for the letter hive... ");
                    validInput = false;
                    break;
                }
            }
            if (!validInput) {
                continue;
            }

            bool duplicate = false;
            for (int i = 0; i < length; i++) {
                for (int j = i + 1; j < length; j++) {
                    if (input[i] == input[j]) {
                        duplicate = true;
                        break;
                    }
                }
                if (duplicate) {
                    break;
                }
            }
            if (duplicate) {
                printf("  HIVE ERROR: \"%s\" contains duplicate letters\n\n",input);
                printf("  Enter a single string of lower-case,\n  unique letters for the letter hive... ");
                continue;
            }

            buildHive(input, hive);
            break;
        }

        hiveSize = strlen(hive);

        reqLetInd = -1;
        reqLet = '\0';

        printf("  Enter the letter from \"%s\"\n  that is required for all words: ", hive);
        
        char reqInp[10];
        while (true) {
            scanf("%s", reqInp);
            char c = tolower(reqInp[0]);
            int index = findLetter(hive, c);

            if (index == -1){
                printf("  HIVE ERROR: \"%s\" does not contain the character \'%c\'\n\n",hive,c);
                printf("  Enter the letter from \"%s\"\n  that is required for all words: ", hive);
                continue;
            }
            else {
                reqLet = c;
                reqLetInd = index;
                break;
            }
        }
    }

    printHive(hive, reqLetInd);

    if (playMode) {
        printf("==== PLAY MODE ====\n");
    //---------------------------------------------------------------------
    //              BEGINNING OF OPEN-ENDED GAMEPLAY SECTION
    //---------------------------------------------------------------------

        char* userWord = (char*)malloc((maxWordLength + 1) * sizeof(char));
        strcpy(userWord, "default");

        WordList* userWordList = createWordList();

        printf("............................................\n");
        printHive(hive, reqLetInd);


        printf("  Enter a word (enter DONE to quit): ");
        scanf("%s", userWord);
        printf("\n");


        while (true) {

            if(strcmp(userWord, "DONE") == 0 || strcmp(userWord, "done") == 0) {
                break;
            }

            for (int i = 0; userWord[i] != '\0'; i++) {
                userWord[i] = tolower(userWord[i]);
            }
            bool added = false;

            if (strlen(userWord) >= MIN_WORD_LENGTH) {
                int index = findWord(dictionaryList, userWord, 0, dictionaryList->numWords - 1);
                bool insideDictionary = (index >= 0);

                if (insideDictionary && isValidWord(userWord, hive, reqLet)) {
                    bool already = false;
                    for (int i = 0; i < userWordList->numWords; i++) {
                        if (strcmp(userWordList->words[i], userWord) == 0) {
                            already = true;
                            break;
                        }
                    }
                    if (!already) {
                        appendWord(userWordList, userWord);
                        added = true;
                    }
                }
            }
            if (!added) {
                printf("  (invalid or duplicate)\n");
            }
            
            //prints the list and the hive, and gets the next input
            printf("\n");
            printList(userWordList, hive);
            printf("............................................\n");
            printHive(hive, reqLetInd);

            printf("  Enter a word (enter DONE to quit): ");
            scanf("%s", userWord);
            printf("\n");

        }

        freeWordList(userWordList);
        free(userWord);

    //---------------------------------------------------------------------    
    //                 END OF OPEN-ENDED GAMEPLAY SECTION
    //---------------------------------------------------------------------
    }
    
        

    printf("==== SPELLING BEE SOLVER ====\n");

    printf("  Valid words from hive \"%s\":\n", hive);
    printf("                         ");
    for (int i = 0; i < reqLetInd; i++) {
        printf(" ");
    }
    printf("^\n");

    WordList* solvedList = createWordList();

    if (bruteForce) { //find all words that work... (1) brute force
        bruteForceSolve(dictionaryList, solvedList, hive, reqLet);
    }
    else {
        char* tryWord = (char*)malloc(sizeof(char) * (maxWordLength + 1));

        tryWord[0] = hive[0];
        tryWord[1] = '\0';
        findAllMatches(dictionaryList, solvedList, tryWord, hive, reqLet);
        free(tryWord);

    }

    int longestSolvedWordLen = 0;
    for (int i = 0; i < solvedList->numWords; i++) {
        if (strlen(solvedList->words[i]) > longestSolvedWordLen) {
            longestSolvedWordLen = strlen(solvedList->words[i]);
        }
    }

    // Helpful variables
    int numValidWords = 0;
    int numPangrams = 0;
    int numPerfectPangrams = 0;
    int totScore = 0;
    int score = 0;
    bool isBingo = true;
    
    int hiveSizeLocal = strlen(hive);
    int already[26] = {0};

    for (int i = 0; i < solvedList->numWords; i++) {
        char* w = solvedList->words[i];
        int length = strlen(w);
        int thisScore;
        if (length == 4) {
            thisScore = 1;
        }
        else {
            thisScore = length;
        }
        bool pangram = isPangram(w, hive);
        if (pangram){
            thisScore += hiveSizeLocal;
        }
        bool perfect = ((pangram) && (length == hiveSizeLocal));

        if (perfect) {
            printf("  *** (%2d) %s\n", thisScore, w);
            numPerfectPangrams++;
            numPangrams++;
        }
        else if (pangram) {
            printf("  * (%2d) %s\n", thisScore, w);
            numPangrams++;
        }
        else {
            printf("    (%2d) %s\n", thisScore, w);
        }

        numValidWords++;
        totScore+= thisScore;

        char cLow = tolower(w[0]);
        if (cLow >= 'a' && cLow <= 'z' && findLetter(hive, cLow) != -1) {
            already[cLow - 'a'] = 1;
        }
    }

    isBingo = true;

    for (int i = 0; i < hiveSizeLocal; i++) {
        if (!already[hive[i] - 'a']) {
            isBingo = false;
            break;
        }
    }
    
    // Additional results are printed here:
    printf("\n");
    printf("  Total counts for hive \"%s\":\n", hive);
    printf("                         ");
    for (int i = 0; i < reqLetInd; i++) {
        printf(" ");
    }
    printf("^\n");
    printf("    Number of Valid Words: %d\n", numValidWords);
    printf("    Number of ( * ) Pangrams: %d\n", numPangrams);
    printf("    Number of (***) Perfect Pangrams: %d\n", numPerfectPangrams);
    printf("    Bingo: ");
    printYESorNO(isBingo);
    printf("    Total Score Possible: %d\n", totScore);

    printf("\n  Frequency Table:\n");
    printf("        ");
    for (int length = MIN_WORD_LENGTH; length <= longestSolvedWordLen; length++) {
        printf("%3d", length);
    }
    printf("\n      ");
    for (int length = MIN_WORD_LENGTH; length <= longestSolvedWordLen; length++) {
        printf("---");
    }
    printf("\n");

    for (int i = 0; i < hiveSizeLocal; i++) {
        char letter = hive[i];
        printf("   %c", letter);
        for (int len = MIN_WORD_LENGTH; len <= longestSolvedWordLen; len++) {
            int count = 0;
            for (int j = 0; j < solvedList->numWords; j++) {
                char* word = solvedList->words[j];
                if (tolower(word[0]) == letter && strlen(word) == len) {
                    count++;
                }
            }
            printf("%3d", count);
        }
        printf("\n");
    }

    freeWordList(dictionaryList);
    freeWordList(solvedList);
    printf("\n\n");
    return 0;
}

