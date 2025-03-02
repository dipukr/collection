import mysql.connector as db
from faker import Faker

fake = Faker('en_IN')

conn = db.connect(host="localhost", user="root", password="Root@123", database="test")
cursor = conn.cursor()

for elem in range(0, 1000):
	cursor.execute(f'insert into user(name, email, password) values("{fake.name()}", "{fake.email()}", "{fake.password()}")')
conn.commit()
