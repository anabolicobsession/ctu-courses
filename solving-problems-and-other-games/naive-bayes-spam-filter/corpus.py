import os
from mail import Mail
from settings import TRUTH_FNAME, PREDICTION_FNAME, SPAM_TAG


class Corpus:
    def __init__(self, path):
        self.path = path
        self.mail_names = [mn for mn in os.listdir(path) if mn[0] != '!']
        self.mails_num = len(self.mail_names)

    def mails(self):
        for mail_name in self.mail_names:
            with open(os.path.join(self.path, mail_name), 'rt', encoding='utf-8') as fmail:
                yield Mail(mail_name, fmail.read())


class TrainCorpus(Corpus):
    def __init__(self, path):
        super().__init__(path)
        self.truth = self.read_truth_classification()

    def mails(self):
        for mail_name in self.mail_names:
            with open(os.path.join(self.path, mail_name), 'rt', encoding='utf-8') as fmail:
                # automatically sets the status for mails
                yield Mail(mail_name, fmail.read(), self.truth[mail_name])

    def read_truth_classification(self):
        with open(os.path.join(self.path, TRUTH_FNAME), 'rt', encoding='utf-8') as file:
            truth = dict()
            for line in file:
                truth[line.split()[0]] = line.split()[1]
        return truth

    def compute_spam_prob(self):  # spam_mails_num / mails_num
        return sum(map(SPAM_TAG.__eq__, self.truth.values())) / self.mails_num


class TestCorpus(Corpus):
    def __init__(self, path):
        super().__init__(path)
        self.prediction = dict()

    def add_to_prediction(self, mail):
        self.prediction[mail.name] = mail.status

    def create_prediction_file(self):
        with open(os.path.join(self.path, PREDICTION_FNAME), 'wt', encoding='utf-8') as fpred:
            for mail_name in self.prediction:
                fpred.write(mail_name + ' ' + self.prediction[mail_name] + '\n')
