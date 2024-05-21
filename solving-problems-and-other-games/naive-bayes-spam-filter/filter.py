from math import log
from corpus import TrainCorpus, TestCorpus
from utils import argmax, cosine_similarity


class MyFilter:
    def __init__(self):
        self.was_trained = False
        self.train_mails_num = 0
        self.spam_prob, self.ham_prob = 0.5, 0.5
        self.blacklist = set()
        self.unique_tokens = list()
        self.token_base = dict()

        # tf_idf
        self.alpha = 0  # smoothing coefficient
        self.tokens_idf = dict()
        self.spam_token_tf_idf = dict()
        self.ham_token_tf_idf = dict()

        # naive Bayes
        self.beta = 0.25  # smoothing coefficient
        self.spam_token_base = dict()
        self.ham_token_base = dict()
        self.tokens_num_in_spam_mails = 0
        self.tokens_num_in_ham_mails = 0
        self.unique_tokens_num = 0

        # cosine similarity
        self.spam_vector = list()
        self.ham_vector = list()

        # parsing based on general features, ALL VALUES are recalculated during training
        self.gamma = 1.2  # coefficient for the difference between spam value and max(ham value)
        self.spam_share_of_html_tags = 0.6
        self.spam_share_of_caps_letters = 0.4
        self.spam_avg_token_len = 30
        self.spam_share_of_strange_tokens = 0.2
        # number of addresses in CC field
        self.spam_cc_field_len = 8  # if there are a lot of them, most likely this is spam mailing

    def train(self, train_path):
        self.was_trained = True
        cp = TrainCorpus(train_path)
        self.train_mails_num = cp.mails_num

        self.spam_prob = cp.compute_spam_prob()
        self.ham_prob = 1 - self.spam_prob

        for mail in cp.mails():
            self.unique_tokens.extend(mail.tokens)
        # it's important to have ordered list for cosine similarity
        self.unique_tokens = list(set(self.unique_tokens))
        self.unique_tokens_num = len(self.unique_tokens)

        self.token_base = {t: [0] * cp.mails_num for t in self.unique_tokens}
        self.spam_token_tf_idf = {t: [0] * cp.mails_num for t in self.unique_tokens}
        self.ham_token_tf_idf = {t: [0] * cp.mails_num for t in self.unique_tokens}
        self.spam_token_base = {t: [0] * cp.mails_num for t in self.unique_tokens}
        self.ham_token_base = {t: [0] * cp.mails_num for t in self.unique_tokens}

        ham_shares_of_html_tags = list()
        ham_shares_of_caps_letters = list()
        ham_avg_token_lens = list()
        ham_shares_of_strange_tokens = list()
        ham_cc_field_lengths = list()

        for idx, mail in enumerate(cp.mails()):
            if mail.is_spam():
                self.blacklist.add(mail.sender_address)
                self.tokens_num_in_spam_mails += mail.tokens_num

                curr_tokens_tf_idf = self.spam_token_tf_idf
                curr_token_base = self.spam_token_base
            else:
                self.tokens_num_in_ham_mails += mail.tokens_num

                curr_tokens_tf_idf = self.ham_token_tf_idf
                curr_token_base = self.ham_token_base

                ham_shares_of_html_tags.append(mail.share_of_html_tags())
                ham_shares_of_caps_letters.append(mail.share_of_caps_letters())
                ham_avg_token_lens.append(mail.avg_token_len())
                ham_shares_of_strange_tokens.append(mail.share_of_strange_tokens())
                ham_cc_field_lengths.append(mail.get_cc_field_len())

            for token in mail.tokens:  # counting the number of tokens in the whole corpus
                # for tf-idf
                self.token_base[token][idx] += 1
                curr_tokens_tf_idf[token][idx] += 1
                # for naive Bayes
                curr_token_base[token][idx] += 1

        self.spam_share_of_html_tags = argmax(ham_shares_of_html_tags) * self.gamma
        self.spam_share_of_caps_letters = argmax(ham_shares_of_caps_letters) * self.gamma
        self.spam_avg_token_len = argmax(ham_avg_token_lens) * self.gamma
        self.spam_share_of_strange_tokens = argmax(ham_shares_of_strange_tokens) * self.gamma
        self.spam_cc_field_len = argmax(ham_cc_field_lengths) + 2

        if None in self.blacklist:  # removes failure(s) to find sender address
            self.blacklist.remove(None)

        for token in self.unique_tokens:  # computes idf value for every token
            # how many mails have this token
            denominator = sum([bool(t) for t in self.token_base[token]])
            self.tokens_idf[token] = log(cp.mails_num / denominator) + self.alpha

        for idx, mail in enumerate(cp.mails()):
            curr_tokens_tf_idf = self.spam_token_tf_idf if mail.is_spam() else self.ham_token_tf_idf

            for token in set(mail.tokens):  # computes tf-idf value for every token
                curr_tokens_tf_idf[token][idx] /= mail.tokens_num
                curr_tokens_tf_idf[token][idx] *= self.tokens_idf[token]

        # for cosine similarity
        self.spam_vector = [sum(self.spam_token_base[t]) for t in self.unique_tokens]
        self.ham_vector = [sum(self.ham_token_base[t]) for t in self.unique_tokens]

    def test(self, test_path):
        cp = TestCorpus(test_path)

        for mail in cp.mails():  # training
            if mail.x_spam_field == 'no':
                # updating ham_token_base
                self.update_bases_for_naive_bayes_with(mail)

        for mail in cp.mails():  # testing
            if mail.x_spam_field != 'no':  # otherwise classifies as ham
                if mail.sender_address in self.blacklist:
                    mail.mark_as_spam()

                if not mail.is_spam():
                    self.parse(mail)

                if self.was_trained:
                    if not mail.is_spam():
                        self.classify_by_naive_bayes(mail)

                    # if not mail.is_spam():
                    #     self.classify_by_tf_idf(mail)

                    # # this is a time consuming method, so it requires a limitation
                    # if not mail.is_spam() and mail.tokens_num <= 1000:
                    #     self.classify_by_cosine_similarity(mail)

            cp.add_to_prediction(mail)

        cp.create_prediction_file()

    def parse(self, mail):
        """Classifies based on general features"""
        if (    mail.share_of_html_tags() >= self.spam_share_of_html_tags or
                mail.share_of_caps_letters() >= self.spam_share_of_caps_letters or
                mail.avg_token_len() >= self.spam_avg_token_len or
                mail.share_of_strange_tokens() >= self.spam_share_of_strange_tokens or
                mail.get_cc_field_len() >= self.spam_cc_field_len or
                mail.sender_address is None):  # None means that the sender address is not specified
            mail.mark_as_spam()

    def classify_by_naive_bayes(self, mail):
        # logarithm allows calculations in much smaller numbers
        spam_prob, ham_prob = log(self.spam_prob), log(self.ham_prob)

        for token in [t for t in mail.tokens if t in self.spam_token_base]:
            spam_prob += log(self.spam_token_prob(token))
            ham_prob += log(self.ham_token_prob(token))

        if spam_prob > ham_prob:
            mail.mark_as_spam()

    def spam_token_prob(self, token):
        numerator = sum(self.spam_token_base[token]) + self.beta
        denominator = self.tokens_num_in_spam_mails + self.unique_tokens_num * self.beta
        return numerator / denominator

    def ham_token_prob(self, token):
        numerator = sum(self.ham_token_base[token]) + self.beta
        denominator = self.tokens_num_in_ham_mails + self.unique_tokens_num * self.beta
        return numerator / denominator

    def update_bases_for_naive_bayes_with(self, mail):
        """Allows updating token bases during testing"""
        self.train_mails_num += 1
        new_idx = self.train_mails_num - 1

        for token in self.spam_token_base:
            self.spam_token_base[token].append(0)
            self.ham_token_base[token].append(0)

        for token in [t for t in set(mail.tokens) if t not in self.spam_token_base]:
            self.spam_token_base[token] = [0] * self.train_mails_num
            self.ham_token_base[token] = [0] * self.train_mails_num

        curr_token_base = self.spam_token_base if mail.is_spam() else self.ham_token_base
        for token in mail.tokens:
            curr_token_base[token][new_idx] += 1

    def classify_by_tf_idf(self, mail):
        spam_prob, ham_prob = self.spam_prob, self.ham_prob

        for token in [t for t in mail.tokens if t in self.unique_tokens]:
            spam_prob *= sum(self.spam_token_tf_idf[token])
            ham_prob *= sum(self.ham_token_tf_idf[token])

        if spam_prob > ham_prob:
            mail.mark_as_spam()

    def classify_by_cosine_similarity(self, mail):
        mail_vector = {t: 0 for t in self.unique_tokens}
        for token in [t for t in mail.tokens if t in self.unique_tokens]:
            mail_vector[token] += 1

        # converting dictionary to vector (mail_vector must be ordered same as spam or ham vector)
        mail_vector = [mail_vector[t] for t in self.unique_tokens]

        if sum(mail_vector):
            spam_cos = cosine_similarity(mail_vector, self.spam_vector)
            ham_cos = cosine_similarity(mail_vector, self.ham_vector)

            if spam_cos > ham_cos:
                mail.mark_as_spam()
