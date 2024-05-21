from math import sqrt
from settings import (MAX_LETTERS_IN_ROW,
                      MIN_DIFFERENCE_BETWEEN_WORD_LEN_AND_MAX_LETTERS_IN_ROW,
                      MIN_WORD_LEN_TO_CHECK_THIS_DIFFERENCE,
                      SPAM_SHARE_OF_CONSONANTS,
                      MIN_WORD_LEN_TO_CHECK_THIS_SHARE)


def load_csv(fname):
    # these file are similar to .csv file, but has no columns
    with open(fname, 'rt', encoding='utf-8') as file:
        return file.read().replace('\n', '').replace(' ', '').split(',')


# these are not real csv files
STOPWORDS = load_csv('stopwords.csv')
HTML_TAGS = load_csv('html_tags.csv')


def cosine_similarity(vector1, vector2):
    sumxx, sumxy, sumyy = 0, 0, 0
    for i in range(len(vector1)):
        x, y = vector1[i], vector2[i]
        sumxx += x*x
        sumyy += y*y
        sumxy += x*y
    return sumxy/sqrt(sumxx*sumyy)


def argmax(values):
    mx = 0
    for val in values:
        if val > mx:
            mx = val
    return mx


def has_normal_letter_frequency(token):
    """Allows to remove a bunch of 'junk words'"""
    vowels = 'aeiou'
    letters_in_row = 0
    last_letter_is_vowel = True
    max_letters_in_row = 0
    consonants_num = 0

    for ch in token:  # counts vowels or consonants in a row
        if ch in vowels and last_letter_is_vowel or \
                ch not in vowels and not last_letter_is_vowel:
            letters_in_row += 1

            if ch not in vowels:
                consonants_num += 1
        else:
            if letters_in_row > max_letters_in_row:
                max_letters_in_row = letters_in_row

            letters_in_row = 1
            last_letter_is_vowel = ch in vowels

            if ch not in vowels:
                consonants_num += 1

        if letters_in_row > MAX_LETTERS_IN_ROW:
            return False

    if (consonants_num / len(token)) >= SPAM_SHARE_OF_CONSONANTS and \
            len(token) >= MIN_WORD_LEN_TO_CHECK_THIS_SHARE:
        return False

    max_letters_in_row = max(letters_in_row, max_letters_in_row)
    if (len(token) - max_letters_in_row <=
            MIN_DIFFERENCE_BETWEEN_WORD_LEN_AND_MAX_LETTERS_IN_ROW and
            len(token) >= MIN_WORD_LEN_TO_CHECK_THIS_DIFFERENCE):
        return False

    return True
