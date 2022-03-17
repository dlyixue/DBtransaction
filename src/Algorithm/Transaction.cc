#include "Transaction.h"

#include <cstddef>
#include <unordered_map>
#include <shared_mutex>
#include <atomic>

static std::unordered_map<RecordKey, RecordItem> storage;
static std::unordered_map<RecordKey, object> MutexTable;

static std::vector<transaction_id_t> serializationOrder;
static std::mutex serializationOrderLock;

void preloadData(const std::unordered_map<RecordKey, RecordData> &initialRecords) {
    for (auto &[key, data] : initialRecords) {
        storage[key].data = data;
        //创建数据初始表
        object obj;
        MutexTable[key] = obj;
        //std::cout<<key<<std::endl;
    }
    //std::cout<<"preloadData \n";
}

std::vector<transaction_id_t> getSerializationOrder() {
    return serializationOrder;
}

void Transaction::start() {
    timestamp = getTimestamp();
}

bool Transaction::read(const RecordKey &key, RecordData &result) {
    bool flag = MutexTable[key].lock(id);
    //std::cout << "add read mutex \n";
    if(flag == false){
        return false;
    }
    if(Initial_data.find(key) == Initial_data.end()){
        Initial_data[key] = storage[key];
    }
    result = storage[key].data;

    return true;
}

bool Transaction::write(const RecordKey &key, const RecordData &newData) {
    bool flag = MutexTable[key].lock(id);
    //std::cout << "add write mutex \n" << key <<std::endl;
    if(flag == false){
        return false;
    }
    if(Initial_data.find(key) == Initial_data.end()){
        //std::cout << "add write data \n" << key <<std::endl;
        Initial_data[key] = storage[key];
    }
    storage[key].data = newData;

    return true;
}

bool Transaction::commit() {
    // Append the commited transaction to serialization order list
    {
        // Use a lock to prevent data race in writing the serializationOrderLock vector
        std::lock_guard lock(serializationOrderLock);
        serializationOrder.push_back(this->id);
    }
    for(auto tmp : Initial_data){
        MutexTable[tmp.first].unlock();
    }
    // "return true" means successfully commited
    return true;
}

void Transaction::rollback() {
    //回滚数据
    for(auto tmp : Initial_data){
        storage[tmp.first] = tmp.second;
        MutexTable[tmp.first].unlock();
    }
    //std::cout<<count++<<std::endl;
    // It won't rollback
}
