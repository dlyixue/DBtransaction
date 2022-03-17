#ifndef _MENCI_TRANSACTION_TESTER_TRANSACTION_H
#define _MENCI_TRANSACTION_TESTER_TRANSACTION_H

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <thread>   
#include <iostream>

#include "Framework/RecordType.h"
#include "Framework/Utility.h"
#include "Framework/WrappedTransaction.h"

#define try_num 10
// This function is called only once, before ALL transactions
void preloadData(const std::unordered_map<RecordKey, RecordData> &initialRecords);

// This function is called only once, after ALL transactions
std::vector<transaction_id_t> getSerializationOrder();

struct RecordItem {
    RecordData data;
};

class object;

class Transaction {
    Transaction(transaction_id_t id) : id(id) { start(); }
    friend class WrappedTransaction;
    friend class object;
public:
    // ID is used internally by the framework to identify the transaction, for validation
    // You should return a sequence of commited transactions' IDs, in their serialization order
    const transaction_id_t id;
    std::unordered_map<RecordKey, RecordItem> Initial_data;

    Transaction(const Transaction &) = delete;
    Transaction(Transaction &&) = default;

private:
    // NOTE: look at here!

    // Fill your transaction-specfied data here
    // e.g. transaction timestamp
    timestamp_t timestamp;
    // These 5 functions below are the framework's interface
    // Return false on ANY function means the transaction should ROLLBACK
    void start();
    bool read(const RecordKey &key, RecordData &result);
    bool write(const RecordKey &key, const RecordData &newData);
    bool commit();
    void rollback();

    // You can declare more utility functions to reuse your code
    void cleanUp();
};

class object{
public:
    std::shared_ptr<std::mutex> mt;
    transaction_id_t id;
    object(): id(0),mt(std::make_shared<std::mutex>()){}
    bool lock(transaction_id_t new_id){
        if(id != new_id){
            for(auto i=0; i < try_num; i++){
                if(mt->try_lock()){
                    id = new_id;
                    return true;
                }                
            }
            return false;
        }
        return true;
    }
    void unlock(){
        mt->unlock();
    }
};

#endif // _MENCI_TRANSACTION_TESTER_TRANSACTION_H
