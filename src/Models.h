#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <sstream>

using namespace std;

template <typename T>
string to_string_compat(T value) {
    ostringstream os;
    os << value;
    return os.str();
}

enum ListingType {
    TYPE_HOUSE,
    TYPE_CAR
};

enum ListingStatus {
    L_STATUS_PENDING,
    L_STATUS_AVAILABLE,
    L_STATUS_SOLD,
    L_STATUS_RENTED
};

class Listing {
public:
    int id;
    ListingType type;
    ListingStatus status;
    double price;
    string location;
    string dateAdded;
    string ownerUsername;

    Listing() : id(0), type(TYPE_HOUSE), status(L_STATUS_PENDING), price(0.0), location(""), dateAdded(""), ownerUsername("") {}
    virtual ~Listing() {}

    virtual string getDetails() const {
        string stat;
        if (status == L_STATUS_PENDING) stat = "[PENDING] ";
        else if (status == L_STATUS_AVAILABLE) stat = "[AVAILABLE] ";
        else if (status == L_STATUS_SOLD) stat = "[SOLD] ";
        else if (status == L_STATUS_RENTED) stat = "[RENTED] ";
        return stat + "ID: " + to_string_compat(id) + " | Price: " + to_string_compat(price) + " | Loc: " + location;
    }
};

class House : public Listing {
public:
    int bedrooms;
    double size;

    House() : bedrooms(0), size(0.0) { type = TYPE_HOUSE; }

    string getDetails() const {
        return Listing::getDetails() + " | Beds: " + to_string_compat(bedrooms) + " | Size: " + to_string_compat(size) + " sqm";
    }
};

class Car : public Listing {
public:
    string model;
    string condition;
    string imgPath1;
    string imgPath2;
    string ownershipDocPath;
    string bodyDocPath;

    Car() : model(""), condition(""), imgPath1(""), imgPath2(""), ownershipDocPath(""), bodyDocPath("") { type = TYPE_CAR; }

    string getDetails() const {
        return Listing::getDetails() + " | Model: " + model + " | Cond: " + condition;
    }
};

class Inquiry {
public:
    int id;
    int listingId;
    string clientName;
    string message;
    string date;

    Inquiry() : id(0), listingId(0), clientName(""), message(""), date("") {}
    Inquiry(int lid, string client) : id(0), listingId(lid), clientName(client), message("Wants to buy/rent"), date("") {}
};

enum UserRole {
    ROLE_ADMIN,
    ROLE_SELLER,
    ROLE_BUYER
};

enum AccountStatus {
    ACC_PENDING,
    ACC_VERIFIED,
    ACC_REJECTED
};

class Account {
public:
    string username;
    string password;
    UserRole role;
    AccountStatus status;
    string documentPath;

    Account(string u, string p, UserRole r)
        : username(u), password(p), role(r),
          status(r == ROLE_ADMIN ? ACC_VERIFIED : ACC_PENDING),
          documentPath("") {}
};

#endif
