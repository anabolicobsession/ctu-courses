import re
from settings import SPAM_TAG, HAM_TAG, MIN_WORD_LEN, MAX_WORD_LEN
from utils import STOPWORDS, HTML_TAGS, has_normal_letter_frequency


def filter_up(tokens, by_length=True, by_letter_frequency=True, stopwords=True):
    if by_length:
        tokens = list(filter(lambda t: MIN_WORD_LEN <= len(t) <= MAX_WORD_LEN, tokens))
    if by_letter_frequency:
        tokens = list(filter(has_normal_letter_frequency, tokens))
    if stopwords:
        tokens = list(filter(lambda t: t not in STOPWORDS, tokens))
    return tokens


def extract_mail_addresses_from_string(string):
    mail_addresses = list()
    for token in string.split():
        if '@' in token:
            mail_addresses.append(token)
    return mail_addresses


class Mail:
    def __init__(self, name, mail, status=HAM_TAG):
        self.name = name
        self.status = status  # by default all mails are 'HAM'
        # it is assumed that the delimiter is the first empty line (i.e '\n' + '\n')
        self.metainfo, self.body = [s.strip() for s in mail.split('\n\n', 1)]
        self.sender_address = self.get_sender_address()
        self.tokens = self.tokenize() + self.get_subject()
        self.tokens_num = len(self.tokens)
        self.x_spam_field = self.get_x_spam_field()

    def tokenize(self):
        pattern = re.compile('<.*?>|&([a-z0-9]+|#[0-9]{1,6}|#x[0-9a-f]{1,6})')
        tokens = re.sub(pattern, '', self.body)  # removes html-tags
        tokens = re.sub('[0-9_]', '', re.sub('[^a-zA-Z0-9_]', ' ', tokens))
        return filter_up(tokens.lower().split())

    def get_sender_address(self):
        metainfo = re.sub('["<>_]', ' ', self.metainfo).lower().split('\n')
        for line in metainfo:
            if line.strip().startswith('from'):
                for token in line.split():
                    if '@' in token:
                        return token
        return None  # if sender's email was not found

    def get_subject(self):
        subject = list()
        metainfo = re.sub('[^a-zA-Z\n]', ' ', self.metainfo)
        for line in metainfo.lower().split('\n'):
            if line.strip().startswith('subject'):
                subject = line.strip().split()[1:]  # removes 'subject'
                break
        return filter_up(subject)

    def get_x_spam_field(self):
        metainfo = re.sub('[^a-zA-Z-\n]', ' ', self.metainfo)
        for line in metainfo.lower().split('\n'):
            line = line.strip()
            if line.startswith('x-spam-status'):
                if 'yes' in line:
                    return 'yes'
                elif 'no' in line:
                    return 'no'
                else:
                    break
        return None

    def share_of_html_tags(self):
        tokens = re.sub('[^a-zA-Z]', ' ', self.body).lower().split()
        html_tags_found = 0
        for html_tag in HTML_TAGS:
            for token in tokens:
                if html_tag == token:
                    html_tags_found += 1
        return html_tags_found / (len(tokens) + 1)  # +1 to avoid division by zero

    def share_of_caps_letters(self):
        letters = ''.join([c for c in self.body if c.isalpha()])
        return sum(1 for c in letters if c.isupper()) / len(letters)

    def avg_token_len(self):  # self.tokens are not used because they are too filtered
        tokens = self.body.split()
        return sum(map(len, tokens)) / (len(tokens) + 1)

    def share_of_strange_tokens(self):
        tokens = re.sub('[0-9_]', '', re.sub('[^a-zA-Z0-9_]', ' ', self.body)).split()
        tokens = filter_up(tokens, by_letter_frequency=False)
        strange_tokens_num = 0
        for token in tokens:
            if not has_normal_letter_frequency(token):
                strange_tokens_num += 1
        return strange_tokens_num / (len(tokens) + 1)

    def get_cc_field_len(self):
        cc_field, cc_field_found = list(), False

        for line in self.metainfo.lower().split('\n'):
            line = line.strip()

            if cc_field_found:
                if '@' in line.split()[0]:
                    cc_field.append(line.split())
                else:
                    break
            elif line.startswith('cc'):
                cc_field_found = True
                cc_field = extract_mail_addresses_from_string(line)

        return len(cc_field)

    def mark_as_spam(self):
        self.status = SPAM_TAG

    def is_spam(self):
        return self.status == SPAM_TAG
