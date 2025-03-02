FIRST = ["Ajay", "Ayush", "Aryan", ""]
LAST = ["Kumar", "Gupta", "Modi", "Mishra", "Raj", "Diwedi", "Trivedi", "Sharma", "Kohli", ""]

KEYS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789~!@#$%^&*()-_+={}[]|\':;?/>.<,"
ALPHA = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
HEX = "0123456789ABCDEF"
DIGIT = "0123456789"
DEPARTMENT = [""]
GENDER = ["Male", "Female"]
ROLE = ["User", "Admin"]
COLOR = ["Red", "Blue", "Yellow", "Green", "Magenta", "Voilet", ]


def rand(upper):
	return upper


def email(firstName, lastName):
	return tolower(firstName)+tolower(lastName)+"@email.com"

def department():
	return DEPARTMENT[rand(len(DEPARTMENT))]

def color():
	return COLOR[rand(len(COLOR))]

def password(sz):
	pword = ""
	for a in range(siz):
		pword += KEYS[rand(len(KEYS))]

