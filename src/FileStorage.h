#ifndef FILESTORAGE_H
#define FILESTORAGE_H

#include "StorageManager.h"
#include "Queue.h"
#include <string>

class FileStorage : public StorageManager {
private:
    std::string filename;
public:
    FileStorage(std::string file);
    bool saveListings(LinkedList<Listing*>& list);
    bool loadListings(LinkedList<Listing*>& list);
    bool saveAccounts(LinkedList<Account*>& list);
    bool loadAccounts(LinkedList<Account*>& list);
    bool saveOrders(Queue<Inquiry*>& queue);
    bool loadOrders(Queue<Inquiry*>& queue);
};

#endif
