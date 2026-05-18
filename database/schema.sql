CREATE DATABASE IF NOT EXISTS BrokerDB;
USE BrokerDB;

CREATE TABLE IF NOT EXISTS Accounts (
    username VARCHAR(100) PRIMARY KEY,
    password_hash VARCHAR(255) NOT NULL,
    role INT NOT NULL, /* 0: Admin, 1: Seller, 2: Buyer */
    status INT NOT NULL, /* 0: Pending, 1: Verified, 2: Rejected */
    document_path VARCHAR(500) DEFAULT ''
);

CREATE TABLE IF NOT EXISTS Listings (
    id INT AUTO_INCREMENT PRIMARY KEY,
    type INT NOT NULL, /* 0: House, 1: Car */
    status INT NOT NULL, /* 0: Pending, 1: Available, 2: Sold, 3: Rented */
    price DECIMAL(15, 2) NOT NULL,
    location VARCHAR(255) NOT NULL,
    dateAdded TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    owner_username VARCHAR(100),

    /* House specific */
    bedrooms INT DEFAULT 0,
    size DECIMAL(10, 2) DEFAULT 0.0,

    /* Car specific */
    model VARCHAR(255) DEFAULT '',
    car_condition VARCHAR(50) DEFAULT '',
    image_path_1 VARCHAR(500) DEFAULT '',
    image_path_2 VARCHAR(500) DEFAULT '',
    ownership_doc_path VARCHAR(500) DEFAULT '',
    body_doc_path VARCHAR(500) DEFAULT '',

    FOREIGN KEY (owner_username) REFERENCES Accounts(username)
);

CREATE TABLE IF NOT EXISTS Inquiries (
    id INT AUTO_INCREMENT PRIMARY KEY,
    listing_id INT NOT NULL,
    client_name VARCHAR(255) NOT NULL,
    message TEXT NOT NULL,
    date_sent TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (listing_id) REFERENCES Listings(id) ON DELETE CASCADE
);
