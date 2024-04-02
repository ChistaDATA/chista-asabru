from pymongo import MongoClient
import time
from statistics import mean, median

def get_database():
    # Provide the mongodb atlas url to connect python to mongodb using pymongo
    CONNECTION_STRING = "mongodb://root:example@127.0.0.1:9120"

    # Create a connection using MongoClient. You can import MongoClient or use pymongo.MongoClient
    client = MongoClient(CONNECTION_STRING)

    # Create the database for our example (we will use the same database throughout the tutorial
    return client['user_shopping_list']

def run_test():
    # Get the database
    dbname = get_database()

    collection_name = dbname["user_1_items"]
    collection_name.drop()

    item_1 = {
        "_id": "U1IT00001",
        "item_name": "Blender",
        "max_discount": "10%",
        "batch_number": "RR450020FRG",
        "price": 340,
        "category": "kitchen appliance"
    }

    item_2 = {
        "_id": "U1IT00002",
        "item_name": "Egg",
        "category": "food",
        "quantity": 12,
        "price": 36,
        "item_description": "brown country eggs"
    }
    collection_name.insert_many([item_1, item_2])

    for x in collection_name.find():
        print(x)

def benchmark_performance():
    print('Starting benchmark')
    loop_times = []

    iters = 100

    for x in range(iters):
        print('Iteration ' + str(x + 1))
        iter_start = time.time()
        run_test()
        iter_end = time.time() - iter_start
        time.sleep(1)
        loop_times.append(iter_end)

    print('Average Time taken : ', mean(loop_times))
    print('Median Time taken : ', median(loop_times))

# This is added so that many files can reuse the function get_database()
if __name__ == "__main__":
    benchmark_performance()
