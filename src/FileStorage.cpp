#include <windows.h>
#include "FileStorage.h"
#include <fstream>
#include <cstdlib>

using namespace std;

static string escapeField(const string& value) {
    string out;
    for (int i = 0; i < (int)value.size(); i++) {
        char ch = value[i];
        if (ch == '\\') out += "\\\\";
        else if (ch == ',') out += "\\,";
        else if (ch == '\n') out += "\\n";
        else if (ch == '\r') out += "\\r";
        else out += ch;
    }
    return out;
}

static bool readEscapedField(const string& line, int& pos, string& field) {
    if (pos > (int)line.size()) return false;
    field = "";

    while (pos < (int)line.size()) {
        char ch = line[pos++];

        if (ch == '\\' && pos < (int)line.size()) {
            char next = line[pos];
            if (next == ',' || next == '\\') {
                pos++;
                field += next;
            } else {
                // Backward compatibility for old unescaped Windows paths like C:\Users\Name.
                field += ch;
            }
        } else if (ch == ',') {
            return true;
        } else {
            field += ch;
        }
    }

    return true;
}

static string hashPasswordForStorage(const string& password) {
    unsigned long hash = 5381;
    for (int i = 0; i < (int)password.size(); i++) {
        hash = ((hash << 5) + hash) + (unsigned char)password[i];
    }
    return "h$" + to_string_compat(hash);
}

static bool isHashedPasswordForStorage(const string& password) {
    return password.size() > 2 && password[0] == 'h' && password[1] == '$';
}

FileStorage::FileStorage(string file) : filename(file) {}

bool FileStorage::saveListings(LinkedList<Listing*>& list) {
    ofstream outFile(filename.c_str());
    if (!outFile.is_open()) return false;

    for (int i = 0; i < list.getSize(); i++) {
        Listing** itemPtr = list.get(i);
        if (itemPtr && *itemPtr) {
            Listing* item = *itemPtr;
            outFile << item->type << "," << item->id << "," << item->price << ","
                    << escapeField(item->location) << "," << item->status << ","
                    << escapeField(item->ownerUsername) << ",";

            if (item->type == TYPE_HOUSE) {
                House* h = (House*)item;
                outFile << h->bedrooms << "," << h->size << "\n";
            } else if (item->type == TYPE_CAR) {
                Car* c = (Car*)item;
                outFile << escapeField(c->model) << "," << escapeField(c->condition) << ","
                        << escapeField(c->imgPath1) << "," << escapeField(c->imgPath2) << ","
                        << escapeField(c->ownershipDocPath) << "," << escapeField(c->bodyDocPath) << "\n";
            }
        }
    }
    outFile.close();
    return true;
}

bool FileStorage::loadListings(LinkedList<Listing*>& list) {
    ifstream inFile(filename.c_str());
    if (!inFile.is_open()) return false;

    string line;
    while (getline(inFile, line)) {
        if (line.empty() || line == "\r") continue;
        
        int pos = 0;
        string typeStr, idStr, priceStr, loc, statusStr, owner;
        if (!readEscapedField(line, pos, typeStr)) continue;
        if (!readEscapedField(line, pos, idStr)) continue;
        if (!readEscapedField(line, pos, priceStr)) continue;
        if (!readEscapedField(line, pos, loc)) continue;
        if (!readEscapedField(line, pos, statusStr)) continue;
        if (!readEscapedField(line, pos, owner)) continue;

        int type = atoi(typeStr.c_str());
        if (type == TYPE_HOUSE) {
            House* h = new House();
            h->id = atoi(idStr.c_str());
            h->price = atof(priceStr.c_str());
            h->location = loc;
            h->status = (ListingStatus)atoi(statusStr.c_str());
            h->ownerUsername = owner;
            string bedStr, sizeStr;
            readEscapedField(line, pos, bedStr);
            readEscapedField(line, pos, sizeStr);
            h->bedrooms = atoi(bedStr.c_str());
            h->size = atof(sizeStr.c_str());
            list.add(h);
        } else if (type == TYPE_CAR) {
            Car* c = new Car();
            c->id = atoi(idStr.c_str());
            c->price = atof(priceStr.c_str());
            c->location = loc;
            c->status = (ListingStatus)atoi(statusStr.c_str());
            c->ownerUsername = owner;
            string model, cond, i1, i2, od, bd;
            readEscapedField(line, pos, model);
            readEscapedField(line, pos, cond);
            readEscapedField(line, pos, i1);
            readEscapedField(line, pos, i2);
            readEscapedField(line, pos, od);
            readEscapedField(line, pos, bd);
            c->model = model;
            c->condition = cond;
            c->imgPath1 = i1;
            c->imgPath2 = i2;
            c->ownershipDocPath = od;
            c->bodyDocPath = bd;
            list.add(c);
        }
    }
    inFile.close();
    return true;
}

#include "Models.h" // For to_string_compat

bool FileStorage::saveAccounts(LinkedList<Account*>& list) {
    ofstream outFile("users.txt");
    if (!outFile.is_open()) return false;
    for (int i = 0; i < list.getSize(); i++) {
        Account** itemPtr = list.get(i);
        if (itemPtr && *itemPtr) {
            Account* acc = *itemPtr;
            if (!isHashedPasswordForStorage(acc->password)) acc->password = hashPasswordForStorage(acc->password);
            outFile << escapeField(acc->username) << "," << escapeField(acc->password) << ","
                    << acc->role << "," << acc->status << ","
                    << escapeField(acc->documentPath) << "\n";
        }
    }
    outFile.close();
    return true;
}

bool FileStorage::loadAccounts(LinkedList<Account*>& list) {
    ifstream inFile("users.txt");
    if (!inFile.is_open()) {
        Account* admin = new Account("admin", hashPasswordForStorage("admin123"), ROLE_ADMIN);
        Account* seller = new Account("seller", hashPasswordForStorage("seller123"), ROLE_SELLER);
        Account* buyer = new Account("buyer", hashPasswordForStorage("buyer123"), ROLE_BUYER);
        seller->status = ACC_VERIFIED;
        buyer->status = ACC_VERIFIED;
        list.add(admin);
        list.add(seller);
        list.add(buyer);
        saveAccounts(list);
        return true;
    }

    bool hasAdmin = false;
    string line;
    while (getline(inFile, line)) {
        if (line.empty() || line == "\r") continue;

        int pos = 0;
        string user, pass, roleStr, statusStr, docPath;
        if (!readEscapedField(line, pos, user)) continue;
        if (!readEscapedField(line, pos, pass)) continue;
        if (!readEscapedField(line, pos, roleStr)) continue;
        if (!readEscapedField(line, pos, statusStr)) continue;
        readEscapedField(line, pos, docPath);

        Account* acc = new Account(user, pass, (UserRole)atoi(roleStr.c_str()));
        acc->status = (AccountStatus)atoi(statusStr.c_str());
        for(int j=0;j<(int)docPath.size();j++) if(docPath[j]=='|') docPath[j]=',';
        acc->documentPath = docPath;
        if (acc->role == ROLE_ADMIN) hasAdmin = true;
        list.add(acc);
    }
    inFile.close();

    if (!hasAdmin) {
        list.add(new Account("admin", hashPasswordForStorage("admin123"), ROLE_ADMIN));
        saveAccounts(list);
    }
    return true;
}

bool FileStorage::saveOrders(Queue<Inquiry*>& queue) {
    ofstream outFile("orders.txt");
    if (!outFile.is_open()) return false;

    Queue<Inquiry*> tempQ;
    while(!queue.isEmpty()) {
        Inquiry* inq = queue.dequeue();
        outFile << inq->listingId << "," << escapeField(inq->clientName) << ","
                << escapeField(inq->message) << "\n";
        tempQ.enqueue(inq);
    }
    while(!tempQ.isEmpty()) {
        queue.enqueue(tempQ.dequeue());
    }
    outFile.close();
    return true;
}

bool FileStorage::loadOrders(Queue<Inquiry*>& queue) {
    ifstream inFile("orders.txt");
    if (!inFile.is_open()) return false;

    string line;
    while (getline(inFile, line)) {
        if (line.empty() || line == "\r") continue;

        int pos = 0;
        string lidStr, client, msg;
        if (!readEscapedField(line, pos, lidStr)) continue;
        if (!readEscapedField(line, pos, client)) continue;
        readEscapedField(line, pos, msg);

        Inquiry* inq = new Inquiry(atoi(lidStr.c_str()), client);
        inq->message = msg;
        queue.enqueue(inq);
    }
    inFile.close();
    return true;
}
