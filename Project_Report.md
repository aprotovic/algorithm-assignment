# Project Report: Online Broker Management System

**DEBRE BERHAN UNIVERSITY**  
**COLLEGE OF COMPUTING SCIENCE**  
**DEPARTMENT OF SOFTWARE ENGINEERING**  

**Course Name:** Design and Analysis of Data Structure and Algorithm  
**Course Code:** SEng2022  

### Group Members:
| No | Name | ID |
|---|---|---|
| 1 | Ezra Behailu | DBU17001273 |
| 2 | Fiker Mamo | DBU1700671 |
| 3 | Firdos Abdu | DBU1700677 |
| 4 | Hosaena Anteneh | DBU1700766 |

**Submitted to:** Mr. Bekele

---

## 1. Introduction
In modern markets, buying and renting houses and cars are essential activities. The **Online Broker Management System** was developed to digitally manage and organize listings. It leverages core Data Structures and Algorithms to allow brokers to efficiently maintain items for sale or rent, while enabling buyers/renters to search and filter listings quickly.

## 2. Implemented Features
1. **Authentication and Roles**: The system supports Admin, Seller, and Buyer accounts with login and registration screens.
2. **Account Verification**: Buyers and sellers upload ID documents. The Admin can view, approve, or reject accounts before users are allowed to trade.
3. **Add Listing**: Verified sellers and admins can add houses or cars with details such as price, location, bedrooms, size, model, condition, images, and ownership documents.
4. **Listing Approval Workflow**: Newly added listings are marked as pending until an Admin approves them for public visibility.
5. **View and Search Listings**: Users can view marketplace listings and search listings by location, ID, owner, model, condition, or displayed listing details.
6. **Sort Listings**: Listings can be sorted by price using an implemented sorting algorithm.
7. **Buyer Order Queue**: Verified buyers can place orders, which are processed by the Admin using a FIFO queue.
8. **Commission Calculation**: The system calculates 5% broker commission when an Admin approves a sale/order.
9. **Local Payment Integration**: Verified buyers can record payments directly inside the system for available listings. The system computes the transaction summary, logs the payment request into the admin sales queue, and awaits admin confirmation.
10. **Data Persistence**: Listings, accounts, and orders are saved to file-based storage automatically.

## 3. Data Structures Used
To fulfill the requirements of SEng2022, the following Data Structures were implemented entirely from scratch:
- **Linked List** (`LinkedList.h`): Used as the primary dynamic storage for all House and Car objects, allowing dynamic addition without reorganizing the array.
- **Hash Table** (`HashTable.h`): Implemented for fast average O(1) searches of listings based on their unique ID. It is used when a selected GUI listing must be converted back into the actual listing object.
- **Binary Search Tree (BST)** (`BST.h`): Implemented to efficiently store and retrieve properties in sorted order by price.
- **Queue** (`Queue.h`): Implemented to handle buyer orders/inquiries in a First-In-First-Out (FIFO) manner.

## 4. Algorithms Used
- **Sorting**: **Bubble Sort** (`Algorithms.h`) is used to sort the LinkedList of items by price.
- **Searching**: Linear searching/filtering is used to filter visible listings by location, ID, model, condition, owner, and listing details.
- **Hash Search**: The Hash Table is used to retrieve a selected listing quickly using its unique ID.
- **Tree Traversal**: In-order traversal is used within the BST to extract items in sorted order.
- **Queue Processing**: Orders are handled using FIFO processing so the earliest buyer request is handled first.

## 5. Object-Oriented Programming (OOP)
- **Inheritance**: A base class `Listing` was created, which is inherited by `House` and `Car` to store specific attributes (e.g., bedrooms for houses, condition for cars).
- **Polymorphism**: Virtual methods were used (e.g., `getDetails()`) to override the display logic depending on whether the object is a House or Car.

## 6. Graphical User Interface (GUI)
The system was built with a C++ Native Windows Graphical User Interface (Win32 API and GDI+) to provide a visually interactive experience where brokers can easily add and manage listings without using a command-line terminal. The dashboard includes role-based controls, listing cards, search controls, admin verification panels, and a details/payment panel.

## 7. Database Configuration
The project features a dual-system database design:
1. **File Storage System**: The active implementation reads/writes to `database.txt`, `users.txt`, and `orders.txt` to ensure the project compiles out-of-the-box in VS Code using `g++`.
2. **MySQL Schema**: Included `schema.sql` for relational database modeling representation.

## 8. Recent Improvements
- Added dashboard search/reset controls for faster listing filtering.
- Enforced verification rules so pending/rejected buyers and sellers cannot trade until approved.
- Added dashboard document re-upload support for unverified users.
- Improved listing ID generation by scanning for the maximum existing ID instead of using the last item, preventing duplicate IDs after sorting.
- Added input validation for price, house details, and car details.
- Updated build configuration to link GDI+, Common Dialog, and Shell libraries required by the GUI and file uploads.
- Integrated direct local payment recording system for immediate broker tracking and admin approval workflow.
- Improved default account loading so demo seller/buyer accounts are verified when `users.txt` is first created.
