from pymongo import MongoClient
from datetime import datetime

client = MongoClient("mongodb://samast_user:Samast%401212@216.48.181.146:27017/samast")
db = client["samast"]
coll = db.get_collection("SCADA_DATA_NEW")
query = {"UTG_ID": "1512", "TIME_STAMP": {"$gte": datetime(2024, 11, 25, 18, 30, 0), 
"$lt": datetime(2024, 11, 26, 18, 30, 0)} }
select = {"_id": False, "_class": False}
records = coll.find(query, select)
count = 0
for record in records:
	count += 1
print(count)